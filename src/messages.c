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

/*******************************************************************************
 * Protobuf Management
 *******************************************************************************/

nrm_msg_t *nrm_msg_create(void)
{
	nrm_msg_t *ret = calloc(1, sizeof(nrm_msg_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_init(ret);
	return ret;
}

void nrm_msg_destroy(nrm_msg_t **msg)
{
	if (msg == NULL || *msg == NULL)
		return;
	nrm__message__free_unpacked(*msg, NULL);
	*msg = NULL;
}

int nrm_msg_fill(nrm_msg_t *msg, int type)
{
	if (msg == NULL || type < 0 || type >= NRM_MSG_TYPE_MAX)
		return -NRM_EINVAL;
	msg->type  = type;
	return 0;
}

static nrm_msg_list_t *nrm_msg_list_new(int type)
{
	nrm_msg_list_t *ret = calloc(1, sizeof(nrm_msg_list_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_list_init(ret);
	ret->type = type;
	return ret;
}

nrm_msg_slicelist_t *nrm_msg_new_slicelist(nrm_vector_t *slices)
{
	void *p;
	nrm_slice_t *s;
	nrm_msg_slicelist_t *ret = calloc(1, sizeof(nrm_msg_slicelist_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_slicelist_init(ret);
	if (slices == NULL) {
		ret->n_slices = 0;
		return ret;
	}
	nrm_vector_length(slices, &ret->n_slices);
	ret->slices = calloc(ret->n_slices, sizeof(nrm_msg_slice_t *));
	assert(ret->slices);
	for (size_t i = 0; i < ret->n_slices; i++) {
		nrm_vector_get(slices, i, &p);
		s = (nrm_slice_t *)p;
		ret->slices[i] = calloc(1, sizeof(nrm_msg_slice_t));
		assert(ret->slices[i]);
		nrm_msg_slice_init(ret->slices[i]);
		ret->slices[i]->name = strdup(s->name);
		ret->slices[i]->uuid = (char *)nrm_uuid_to_char(s->uuid);
	}
	return ret;
}

int nrm_msg_set_list_slices(nrm_msg_t *msg, nrm_vector_t *slices)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->list = nrm_msg_list_new(NRM_MSG_LIST_TYPE_SLICE);
	assert(msg->list);
	msg->list->slices = nrm_msg_new_slicelist(slices);
	return 0;
}

static int nrm_msg_pop_packed_frames(zmsg_t *zm, nrm_msg_t **msg)
{
	/* empty frame delimiter */
	zframe_t *frame = zmsg_pop(zm);
	assert(zframe_size(frame) == 0);
	zframe_destroy(&frame);
	/* unpack */
	frame = zmsg_pop(zm);
	*msg = nrm__message__unpack(NULL, zframe_size(frame),
				    zframe_data(frame));
	zframe_destroy(&frame);
	return 0;
}

static int nrm_msg_push_packed_frames(zmsg_t *zm, nrm_msg_t *msg)
{
	/* need to add a frame delimiter for zmq to consider this a separate
	 * message
	 */
	zframe_t *frame = zframe_new_empty();
	zmsg_append(zm, &frame);
	/* now add the packed data */
	size_t size = nrm__message__get_packed_size(msg);
	unsigned char *buf = malloc(size);
	assert(buf);
	nrm__message__pack(msg, buf);
	zmsg_addmem(zm, buf, size);
	free(buf);
	return 0;
}

/*******************************************************************************
 * General API, NOT PROTOBUF STUFF
 *******************************************************************************/

static int nrm_msg_pop_identity(zmsg_t *zm, nrm_uuid_t **uuid)
{
	zframe_t *frame = zmsg_pop(zm);
	nrm_uuid_t *id = nrm_uuid_create_fromchar((char *)zframe_data(frame));
	zframe_destroy(&frame);
	*uuid = id;
	return 0;
}

static int nrm_msg_push_identity(zmsg_t *zm, nrm_uuid_t *uuid)
{
	zframe_t *frame = zframe_new(uuid->data, NRM_UUID_SIZE);
	zmsg_append(zm, &frame);
	return 0;	
}

int nrm_msg_send(zsock_t *socket, nrm_msg_t *msg)
{
	zmsg_t *zm = zmsg_new();
	if (zm == NULL)
		return -NRM_ENOMEM;
	nrm_msg_push_packed_frames(zm, msg);
	return zmsg_send(&zm, socket);
}

int nrm_msg_sendto(zsock_t *socket, nrm_msg_t *msg, nrm_uuid_t *uuid)
{
	zmsg_t *zm = zmsg_new();
	if (zm == NULL)
		return -NRM_ENOMEM;
	nrm_msg_push_identity(zm, uuid);
	nrm_msg_push_packed_frames(zm, msg);
	return zmsg_send(&zm, socket);
}

nrm_msg_t *nrm_msg_recv(zsock_t *socket)
{
	zmsg_t *zm = zmsg_recv(socket);
	assert(zm);
	nrm_msg_t *msg = NULL;
	nrm_msg_pop_packed_frames(zm, &msg);
	zmsg_destroy(&zm);
	return msg;
}

nrm_msg_t *nrm_msg_recvfrom(zsock_t *socket, nrm_uuid_t **uuid)
{
	zmsg_t *zm = zmsg_recv(socket);
	assert(zm);
	nrm_msg_pop_identity(zm, uuid);
	nrm_msg_t *msg = NULL;
	nrm_msg_pop_packed_frames(zm, &msg);
	zmsg_destroy(&zm);
	return msg;
}

/*******************************************************************************
 * JSON Pretty-printing
 *******************************************************************************/

struct nrm_msg_type_table_e {
	int type;
	const char *s;
};

static const struct nrm_msg_type_table_e nrm_msg_type_table[] = {
	{ NRM_MSG_TYPE_ACK, "ACK" },
	{ NRM_MSG_TYPE_LIST, "LIST" },
};

const char *nrm_msg_type_t2s(int type)
{
	if (type < 0 || type > NRM_MSG_TYPE_MAX)
		return NULL;
	return nrm_msg_type_table[type].s;
}

json_t *nrm_msg_to_json(nrm_msg_t *msg)
{
	json_t *ret;
	ret = json_pack("{s:s}", "type", nrm_msg_type_t2s(msg->type));
	return ret;
}

int nrm_msg_fprintf(FILE *f, nrm_msg_t *msg)
{
	json_t *json;
	json = nrm_msg_to_json(msg);
	char *s = json_dumps(json, JSON_INDENT(4) | JSON_SORT_KEYS);
	fprintf(f, "%s\n", s);
	return 0;
}

/*******************************************************************************
 * Broker Communication
 *******************************************************************************/

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
