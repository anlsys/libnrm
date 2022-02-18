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


nrm_msg_t *nrm_msg_new_progress(nrm_time_t timestamp, unsigned long progress,
				nrm_scope_t *scope)
{
	nrm_msg_t *ret;
	ret = calloc(1, sizeof(nrm_msg_t));
	assert(ret != NULL);
	ret->type = NRM_MSG_TYPE_SENSOR_PROGRESS;
	ret->timestamp = timestamp;
	ret->u.sspg.progress = progress;
	ret->u.sspg.scope = scope; 
	return ret;
}

nrm_msg_t *nrm_msg_new_pause(nrm_time_t timestamp)
{
	nrm_msg_t *ret;
	ret = calloc(1, sizeof(nrm_msg_t));
	assert(ret != NULL);
	ret->type = NRM_MSG_TYPE_SENSOR_PAUSE;
	ret->timestamp = timestamp;
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
		return NULL;
	}
}

/* for ease of use, we build a table of those */
struct nrm_ctrlmsg_type_table_e {
	int type;
	const char *s;
};

static const struct nrm_ctrlmsg_type_table_e nrm_ctrlmsg_table[] = {
	{ NRM_CTRLMSG_TYPE_SEND, NRM_CTRLMSG_TYPE_STRING_SEND },
	{ NRM_CTRLMSG_TYPE_TERM, NRM_CTRLMSG_TYPE_STRING_TERM },
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
