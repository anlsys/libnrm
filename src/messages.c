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
 * Protobuf Management: Creating Messages
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

nrm_msg_sensor_t *nrm_msg_sensor_new(const char *name, nrm_uuid_t *uuid)
{
	nrm_msg_sensor_t *ret = calloc(1, sizeof(nrm_msg_sensor_t));
	if (ret == NULL)
		return ret;
	nrm_msg_sensor_init(ret);
	ret->name = strdup(name);
	if (uuid)
		ret->uuid = strdup((char *)nrm_uuid_to_char(uuid));
	return ret;
}

nrm_msg_slice_t *nrm_msg_slice_new(const char *name, nrm_uuid_t *uuid)
{
	nrm_msg_slice_t *ret = calloc(1, sizeof(nrm_msg_slice_t));
	if (ret == NULL)
		return ret;
	nrm_msg_slice_init(ret);
	ret->name = strdup(name);
	if (uuid)
		ret->uuid = strdup((char *)nrm_uuid_to_char(uuid));
	return ret;
}

nrm_msg_add_t *nrm_msg_add_new(int type)
{
	nrm_msg_add_t *ret = calloc(1, sizeof(nrm_msg_add_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_add_init(ret);
	ret->type = type;
	return ret;
}

nrm_msg_scope_t *nrm_msg_scope_new(nrm_scope_t *scope)
{
	nrm_msg_scope_t *ret = calloc(1, sizeof(nrm_msg_scope_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_scope_init(ret);
	nrm_bitmap_to_array(&scope->maps[NRM_SCOPE_TYPE_CPU], &ret->n_cpus, &ret->cpus);
	nrm_bitmap_to_array(&scope->maps[NRM_SCOPE_TYPE_NUMA], &ret->n_numas, &ret->numas);
	nrm_bitmap_to_array(&scope->maps[NRM_SCOPE_TYPE_GPU], &ret->n_gpus, &ret->gpus);
	return ret;
}

nrm_msg_event_t *nrm_msg_event_new(nrm_uuid_t *uuid)
{
	nrm_msg_event_t *ret = calloc(1, sizeof(nrm_msg_event_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_event_init(ret);
	ret->uuid = strdup((char *)nrm_uuid_to_char(uuid));
	return ret;
}

int nrm_msg_set_event(nrm_msg_t *msg, nrm_time_t time, nrm_uuid_t *uuid, nrm_scope_t *scope,
		      double value)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->event = nrm_msg_event_new(uuid);
	assert(msg->event);
	msg->data_case = NRM__MESSAGE__DATA_EVENT;
	msg->event->time = nrm_time_tons(&time);
	msg->event->value = value;
	msg->event->scope = nrm_msg_scope_new(scope);
	assert(msg->event->scope);
	return 0;
}

int nrm_msg_set_add_sensor(nrm_msg_t *msg, char *name, nrm_uuid_t *uuid)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->add = nrm_msg_add_new(NRM_MSG_TARGET_TYPE_SENSOR);
	assert(msg->add);
	msg->data_case = NRM__MESSAGE__DATA_ADD;
	msg->add->data_case = NRM__ADD__DATA_SENSOR;
	msg->add->sensor = nrm_msg_sensor_new(name, uuid);
	return 0;
}

int nrm_msg_set_add_slice(nrm_msg_t *msg, char *name, nrm_uuid_t *uuid)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->add = nrm_msg_add_new(NRM_MSG_TARGET_TYPE_SLICE);
	assert(msg->add);
	msg->data_case = NRM__MESSAGE__DATA_ADD;
	msg->add->data_case = NRM__ADD__DATA_SLICE;
	msg->add->slice = nrm_msg_slice_new(name, uuid);
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

nrm_msg_sensorlist_t *nrm_msg_sensorlist_new(nrm_vector_t *sensors)
{
	void *p;
	nrm_msg_sensorlist_t *ret = calloc(1, sizeof(nrm_msg_sensorlist_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_sensorlist_init(ret);
	if (sensors == NULL) {
		ret->n_sensors = 0;
		return ret;
	}
	nrm_vector_length(sensors, &ret->n_sensors);
	nrm_log_debug("vector contains %zu\n", ret->n_sensors);
	ret->sensors = calloc(ret->n_sensors, sizeof(nrm_msg_sensor_t *));
	assert(ret->sensors);
	for (size_t i = 0; i < ret->n_sensors; i++) {
		nrm_vector_get(sensors, i, &p);
		nrm_sensor_t *s = (nrm_sensor_t *)p;
		nrm_log_debug("packed sensor %zu %s\n", i, s->name);
		ret->sensors[i] = nrm_msg_sensor_new(s->name, s->uuid);
	}
	return ret;
}

nrm_msg_slicelist_t *nrm_msg_slicelist_new(nrm_vector_t *slices)
{
	void *p;
	nrm_msg_slicelist_t *ret = calloc(1, sizeof(nrm_msg_slicelist_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_slicelist_init(ret);
	if (slices == NULL) {
		ret->n_slices = 0;
		return ret;
	}
	nrm_vector_length(slices, &ret->n_slices);
	nrm_log_debug("vector contains %zu\n", ret->n_slices);
	ret->slices = calloc(ret->n_slices, sizeof(nrm_msg_slice_t *));
	assert(ret->slices);
	for (size_t i = 0; i < ret->n_slices; i++) {
		nrm_vector_get(slices, i, &p);
		nrm_slice_t *s = (nrm_slice_t *)p;
		nrm_log_debug("packed slice %zu %s\n", i, s->name);
		ret->slices[i] = nrm_msg_slice_new(s->name, s->uuid);
	}
	return ret;
}

int nrm_msg_set_list_sensors(nrm_msg_t *msg, nrm_vector_t *sensors)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->list = nrm_msg_list_new(NRM_MSG_TARGET_TYPE_SENSOR);
	assert(msg->list);
	msg->data_case = NRM__MESSAGE__DATA_LIST;
	msg->list->data_case = NRM__LIST__DATA_SENSORS;
	msg->list->sensors = nrm_msg_sensorlist_new(sensors);
	return 0;
}

int nrm_msg_set_list_slices(nrm_msg_t *msg, nrm_vector_t *slices)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->list = nrm_msg_list_new(NRM_MSG_TARGET_TYPE_SLICE);
	assert(msg->list);
	msg->data_case = NRM__MESSAGE__DATA_LIST;
	msg->list->data_case = NRM__LIST__DATA_SLICES;
	msg->list->slices = nrm_msg_slicelist_new(slices);
	return 0;
}

/*******************************************************************************
 * Protobuf Management: Parsing Messages
 *******************************************************************************/

nrm_scope_t *nrm_scope_create_frommsg(nrm_msg_scope_t *msg)
{
	if (msg == NULL)
		return NULL;
	nrm_scope_t *ret = nrm_scope_create();
	nrm_bitmap_from_array(&ret->maps[NRM_SCOPE_TYPE_CPU], msg->n_cpus,
			     msg->cpus);
	nrm_bitmap_from_array(&ret->maps[NRM_SCOPE_TYPE_NUMA], msg->n_numas,
			     msg->numas);
	nrm_bitmap_from_array(&ret->maps[NRM_SCOPE_TYPE_GPU], msg->n_gpus,
			     msg->gpus);
	return ret;
}

/*******************************************************************************
 * Protobuf Management: ZMQ Management
 *******************************************************************************/

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
	zframe_t *frame = zframe_from(nrm_uuid_to_char(uuid));
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

struct nrm_msg_type_table_s {
	int type;
	const char *s;
};

typedef struct nrm_msg_type_table_s nrm_msg_type_table_t;

static const nrm_msg_type_table_t nrm_msg_type_table[] = {
	{ NRM_MSG_TYPE_ACK, "ACK" },
	{ NRM_MSG_TYPE_LIST, "LIST" },
	{ NRM_MSG_TYPE_ADD, "ADD" },
	{ NRM_MSG_TYPE_EVENT, "EVENT" },
	{ 0, NULL },
};

static const nrm_msg_type_table_t nrm_msg_target_table[] = {
	{ NRM_MSG_TARGET_TYPE_SLICE, "SLICE" },
	{ NRM_MSG_TARGET_TYPE_SENSOR, "SENSOR" },
	{ 0, NULL },
};

const char *nrm_msg_type_t2s(int type, const nrm_msg_type_table_t *table)
{
	if (type < 0)
		return "UNKNOWN";
	for (int i = 0; table[i].s != NULL; i++)
		if (table[i].type == type)
			return table[i].s;
	return "UNKNOWN";
}

json_t *nrm_msg_sensor_to_json(nrm_msg_sensor_t *msg)
{
	json_t *ret;
	ret = json_pack("{s:s, s:s?}", "name", msg->name, "uuid", msg->uuid);
	return ret;
}

json_t *nrm_msg_sensorlist_to_json(nrm_msg_sensorlist_t *msg)
{
	json_t *ret;
	ret = json_array();
	for(size_t i = 0; i < msg->n_sensors; i++) {
		json_array_append_new(ret,
				      nrm_msg_sensor_to_json(msg->sensors[i]));
	}
	return ret;
}

json_t *nrm_msg_slice_to_json(nrm_msg_slice_t *msg)
{
	json_t *ret;
	ret = json_pack("{s:s, s:s?}", "name", msg->name, "uuid", msg->uuid);
	return ret;
}

json_t *nrm_msg_slicelist_to_json(nrm_msg_slicelist_t *msg)
{
	json_t *ret;
	ret = json_array();
	for(size_t i = 0; i < msg->n_slices; i++) {
		json_array_append_new(ret,
				      nrm_msg_slice_to_json(msg->slices[i]));
	}
	return ret;
}

json_t *nrm_msg_add_to_json(nrm_msg_add_t *msg)
{
	json_t *ret;
	json_t *sub;
	switch (msg->type) {
	case NRM_MSG_TARGET_TYPE_SLICE:
		sub = nrm_msg_slice_to_json(msg->slice);
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		sub = nrm_msg_sensor_to_json(msg->sensor);
		break;
	default:
		sub = NULL;
		break;
	}
	ret = json_pack("{s:s, s:o?}", "type", nrm_msg_type_t2s(msg->type,
							  nrm_msg_target_table),
			"data", sub);
	return ret;
}

json_t *nrm_msg_list_to_json(nrm_msg_list_t *msg)
{
	json_t *ret;
	json_t *sub;
	switch (msg->type) {
	case NRM_MSG_TARGET_TYPE_SLICE:
		sub = nrm_msg_slicelist_to_json(msg->slices);
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		sub = nrm_msg_sensorlist_to_json(msg->sensors);
		break;
	default:
		sub = NULL;
		break;
	}
	ret = json_pack("{s:s, s:o?}", "type", nrm_msg_type_t2s(msg->type,
							  nrm_msg_target_table),
			"data", sub);
	return ret;
}

json_t *nrm_msg_bitmap_to_json(size_t nitems, int32_t *items)
{
	json_t *ret;
	ret = json_array();
	for (size_t i = 0; i < nitems; i++)
		json_array_append_new(ret, json_integer(items[i]));
	return ret;
}

json_t *nrm_msg_scope_to_json(nrm_msg_scope_t *msg)
{
	json_t *ret;
	json_t *cpus, *gpus, *numas;
	cpus = nrm_msg_bitmap_to_json(msg->n_cpus, msg->cpus);
	numas = nrm_msg_bitmap_to_json(msg->n_numas, msg->numas);
	gpus = nrm_msg_bitmap_to_json(msg->n_gpus, msg->gpus);
	ret = json_pack("{s:o, s:o, s:o}", "cpu", cpus, "numa", numas, "gpu",
			gpus);
	return ret;
}

json_t *nrm_msg_event_to_json(nrm_msg_event_t *msg)
{
	json_t *ret;
	json_t *sub;
	sub = nrm_msg_scope_to_json(msg->scope);
	ret = json_pack("{s:s, s:I, s:o?, s:f}", "uuid", msg->uuid, 
			"time", msg->time , "scope", sub,
			"value", msg->value);
	return ret;
}

json_t *nrm_msg_to_json(nrm_msg_t *msg)
{
	json_t *ret;
	json_t *sub;
	switch (msg->type) {
	case NRM_MSG_TYPE_ADD:
		sub = nrm_msg_add_to_json(msg->add);
		break;
	case NRM_MSG_TYPE_LIST:
		sub = nrm_msg_list_to_json(msg->list);
		break;
	case NRM_MSG_TYPE_EVENT:
		sub = nrm_msg_event_to_json(msg->event);
		break;
	default:
		sub = NULL;
		break;
	}
	ret = json_pack("{s:s, s:o?}", "type", nrm_msg_type_t2s(msg->type,
							  nrm_msg_type_table),
			"data", sub);
	return ret;
}

int nrm_msg_fprintf(FILE *f, nrm_msg_t *msg)
{
	json_t *json;
	json = nrm_msg_to_json(msg);
	char *s = json_dumps(json, 0);
	fprintf(f, "%s\n", s);
	json_decref(json);
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
	nrm_log_debug("received %s:%u\n",s,*type);
	nrm_log_printmsg(NRM_LOG_DEBUG, (nrm_msg_t *)p);
	if (from != NULL) {
		*from = (nrm_uuid_t *)u;
		nrm_log_debug("from %s\n",nrm_uuid_to_char(*from));
	}
	return (nrm_msg_t *)p;
}
