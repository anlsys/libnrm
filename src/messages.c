/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#include "config.h"

#include <assert.h>

#include "nrm.h"

#include "internal/nrmi.h"
#include "internal/messages.h"


nrm_msg_t *nrm_msg_new(int type, ...)
{
	nrm_msg_t *ret;
	va_list ap;
	ret = calloc(1, sizeof(nrm_msg_t));
	if (!ret)
		return NULL;

	va_start(ap, type);
	switch(type) {
		case NRM_MSG_TYPE_SENSOR_PROGRESS:
			ret->type = type;
			ret->timestamp = va_arg(ap, nrm_time_t);
			ret->u.sspg.progress = va_arg(ap, unsigned long);
			ret->u.sspg.scope = va_arg(ap, nrm_scope_t *);
			break;
		case NRM_MSG_TYPE_SENSOR_PAUSE:
			ret->type = type;
			ret->timestamp = va_arg(ap, nrm_time_t);
			break;
		case NRM_MSG_TYPE_REQ_LIST:
			ret->type = type;
			ret->u.reql.target = va_arg(ap, int);
			break;
		default:
			free(ret);
			ret = NULL;
			break;
	}
	va_end(ap);
	return ret;
}


nrm_msg_t *nrm_msg_new_req_list(int target)
{
	nrm_msg_t *ret;
	ret = calloc(1, sizeof(nrm_msg_t));
	if (!ret)
		return NULL;
	ret->type = NRM_MSG_TYPE_REQ_LIST;
	ret->u.reql.target = target;
	return ret;
}

nrm_msg_t *nrm_msg_new_rep_list(int target, nrm_vector_t *items)
{
	nrm_msg_t *ret;
	ret = calloc(1, sizeof(nrm_msg_t));
	if (!ret)
		return NULL;
	ret->type = NRM_MSG_TYPE_REQ_LIST;
	ret->u.repl.target = target;
	/* TODO: should probably copy it to avoid weird race conditions */
	ret->u.repl.items = items;
	return ret;
}

void nrm_msg_destroy(nrm_msg_t **msg)
{
	free(*msg);
	*msg = NULL;
}

json_t *nrm_msg_encode_progress(nrm_msg_t *msg)
{
	json_t *json;
	json_t *sc;
	json_t *time;
	json_error_t error;
	struct nrm_msg_sspg_s *data = &msg->u.sspg;
	time = nrm_time_to_json(&msg->timestamp);
	assert(time != NULL);
	sc = nrm_scope_to_json(data->scope);
	assert(sc != NULL);
	json = json_pack_ex(&error, 0, "{s:o, s:{s:{s:i, s:{s:s, s:s, s:i, s:i, s:i},s:o}}}",
			"timestamp", time,
			"info", "threadProgress", "progress", data->progress,
			"downstreamThreadID", "cmdID", data->cmdid,
			"taskID", data->name, "processID", 0, "rankID", 0,
			"threadID", 0, "scopes", sc);
	if (json == NULL) {
		fprintf(stderr,"error packing json: %s\n", error.text);
	}
	return json;
}

json_t *nrm_msg_encode_pause(nrm_msg_t *msg)
{
	json_t *json;
	json_t *time;
	json_error_t error;
	struct nrm_msg_sspa_s *data = &msg->u.sspa;
	time = nrm_time_to_json(&msg->timestamp);
	assert(time != NULL);
	json = json_pack_ex(&error, 0, "{s:o, s:{s:{s:{s:s, s:s, s:i, s:i, s:i}}}}",
			"timestamp", time,
			"info", "threadPause", "downstreamThreadID", 
			"cmdID", data->cmdid,
			"taskID", data->name, "processID", 0, "rankID", 0,
			"threadID", 0);
	if (json == NULL) {
		fprintf(stderr,"error packing json: %s, %s, %d, %d, %d\n",
			error.text, error.source, error.line, error.column,
			error.position);
	}
	return json;
}

json_t *nrm_msg_encode_req_list(nrm_msg_t *msg)
{
	json_t *json;
	json_t *time;
	json_error_t error;
	struct nrm_msg_reql_s *data = &msg->u.reql;
	time = nrm_time_to_json(&msg->timestamp);
	assert(time != NULL);
	json = json_pack_ex(&error, 0, "{s:o, s:i, s:{s:i}}",
			"timestamp", time,
			"type", msg->type, "extra", 
			"target", data->target, 0);
	if (json == NULL) {
		fprintf(stderr,"error packing json: %s, %s, %d, %d, %d\n",
			error.text, error.source, error.line, error.column,
			error.position);
	}
	return json;
}

json_t *nrm_msg_encode(nrm_msg_t *msg)
{
	switch(msg->type) {
	case NRM_MSG_TYPE_SENSOR_PROGRESS:
		return nrm_msg_encode_progress(msg);
	case NRM_MSG_TYPE_SENSOR_PAUSE:
		return nrm_msg_encode_pause(msg);
	case NRM_MSG_TYPE_REQ_LIST:
		return nrm_msg_encode_req_list(msg);
	default:
		assert(0);
		return NULL;
	}
}

nrm_msg_t *nrm_msg_decode_req_list(json_t *extra)
{
	int target;
	int err;
	json_error_t error;
	err = json_unpack_ex(extra, &error, 0, "{s:i}",
			     "target", &target);
	if (err) {
		fprintf(stderr, "error unpacking json: %s, %s, %d, %d, %d\n",
			error.text, error.source, error.line, error.column,
			error.position);
	}
	return nrm_msg_new_req_list(target);		
}

nrm_msg_t *nrm_msg_decode(json_t *json, int *msg_type)
{
	int err;
	json_error_t error;
	json_t *time, *extra;
	nrm_msg_t *msg;
	err = json_unpack_ex(json, &error, 0, "{s:o, s:i, s:o}",
			     "timestamp", &time,
			     "type", msg_type,
			     "extra", &extra);
	if (err) {
		fprintf(stderr, "error unpacking json: %s, %s, %d, %d, %d\n",
			error.text, error.source, error.line, error.column,
			error.position);
	}
	switch(*msg_type) {
	case NRM_MSG_TYPE_REQ_LIST:
		msg = nrm_msg_decode_req_list(extra);
		break;
	default:
		assert(0);
		return NULL;
	}
	err = nrm_time_from_json(&msg->timestamp, time);
	assert(err == 0);
	return msg;
}

int nrm_msg_send(zsock_t *socket, nrm_msg_t *msg)
{
	int err;
	json_t *json = nrm_msg_encode(msg);
	char *s = json_dumps(json, JSON_COMPACT);
	err = zsock_send(socket, "s", s); 
	free(s);
	json_decref(json);
	return err;
}

nrm_msg_t *nrm_msg_recv(zsock_t *socket, int *msg_type)
{
	int err;
	char *s;
	err = zsock_recv(socket, "s", &s);
	assert(err == 0);
	json_error_t error;
	json_t *json = json_loads(s, 0, &error);
	nrm_msg_t *ret = nrm_msg_decode(json, msg_type);
	free(s);
	json_decref(json);
	return ret;
}

nrm_msg_t *nrm_msg_recvfrom(zsock_t *socket, int *msg_type, nrm_uuid_t **from)
{
	int err;
	char *s, *uuid;
	err = zsock_recv(socket, "ss", &uuid, &s);
	assert(err == 0);
	fprintf(stderr, "recvfrom uuid:'%s' msg:'%s'\n", uuid, s);
	json_error_t error;
	json_t *json = json_loads(s, 0, &error);
	nrm_msg_t *ret = nrm_msg_decode(json, msg_type);
	free(s);
	json_decref(json);
	*from = nrm_uuid_create_fromchar(uuid);
	return ret;
}

void nrm_msg_fprintf(FILE *out, nrm_msg_t *msg)
{
	if (out == NULL || msg == NULL)
		return;
	json_t *json = nrm_msg_encode(msg);
	json_dumpf(json, out, JSON_INDENT(4)|JSON_SORT_KEYS);
	json_decref(json);
}

/* for ease of use, we build a table of those */
struct nrm_ctrlmsg_type_table_e {
	int type;
	const char *s;
};

static const struct nrm_ctrlmsg_type_table_e nrm_ctrlmsg_table[] = {
	{ NRM_CTRLMSG_TYPE_TERM, NRM_CTRLMSG_TYPE_STRING_TERM },
	{ NRM_CTRLMSG_TYPE_SEND, NRM_CTRLMSG_TYPE_STRING_SEND },
	{ NRM_CTRLMSG_TYPE_RECV, NRM_CTRLMSG_TYPE_STRING_RECV },
	{ NRM_CTRLMSG_TYPE_PUB, NRM_CTRLMSG_TYPE_STRING_PUB },
	{ NRM_CTRLMSG_TYPE_SUB, NRM_CTRLMSG_TYPE_STRING_SUB },
};

const char *nrm_ctrlmsg_t2s(int type)
{
	if (type < 0 || type > NRM_CTRLMSG_TYPE_MAX)
		return NULL;
	return nrm_ctrlmsg_table[type].s;
}

int nrm_ctrlmsg_s2t(const char *string)
{
	for(int i = 0; i < NRM_CTRLMSG_TYPE_MAX; i++)
		if(!strcmp(string, nrm_ctrlmsg_table[i].s))
			return i;
	return -1;
}

int nrm_ctrlmsg_send(zsock_t *socket, int type, nrm_msg_t *msg, nrm_uuid_t *to)
{
	int err;
	/* control messages are basically a control string and a pointer */
	const char *typestring = nrm_ctrlmsg_t2s(type);
	assert(typestring != NULL);
	err = zsock_send(socket, "spp", typestring, (void *)msg, (void *)to);
	assert(err == 0);
	return err;
}

nrm_msg_t *nrm_ctrlmsg_recv(zsock_t *socket, int *type, nrm_uuid_t **from)
{
	int err;
	char *s; void *p,*u;
	err = zsock_recv(socket, "spp", &s, &p, &u);
	assert(err == 0);
	*type = nrm_ctrlmsg_s2t(s);
	if (from != NULL)
		*from = (nrm_uuid_t *)u;
	return (nrm_msg_t *)p;
}
