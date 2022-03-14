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
		default:
			free(ret);
			ret = NULL;
			break;
	}
	va_end(ap);
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

json_t *nrm_msg_encode(nrm_msg_t *msg)
{
	switch(msg->type) {
	case NRM_MSG_TYPE_SENSOR_PROGRESS:
		return nrm_msg_encode_progress(msg);
	case NRM_MSG_TYPE_SENSOR_PAUSE:
		return nrm_msg_encode_pause(msg);
	default:
		assert(0);
		return NULL;
	}
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
	(void)socket;
	(void)msg_type;
	return NULL;
}

void nrm_msg_fprintf(FILE *out, nrm_msg_t *msg)
{
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

int nrm_ctrlmsg_send(zsock_t *socket, int type, nrm_msg_t *msg)
{
	int err;
	/* control messages are basically a control string and a pointer */
	const char *typestring = nrm_ctrlmsg_t2s(type);
	assert(typestring != NULL);
	err = zsock_send(socket, "sp", typestring, (void *)msg);
	assert(err == 0);
	return err;
}

nrm_msg_t *nrm_ctrlmsg_recv(zsock_t *socket, int *type)
{
	int err;
	char *s; void *p;
	err = zsock_recv(socket, "sp", &s, &p);
	assert(err == 0);
	*type = nrm_ctrlmsg_s2t(s);
	return (nrm_msg_t *)p;
}
