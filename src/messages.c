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
// clang-format off
#include "nrm.h"

#include "internal/messages.h"
#include "internal/nrmi.h"
// clang-format on

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

void nrm_msg_destroy_received(nrm_msg_t **msg)
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
	msg->type = type;
	return 0;
}

nrm_msg_actuate_t *nrm_msg_actuate_new(nrm_string_t uuid, double value)
{
	nrm_msg_actuate_t *ret = calloc(1, sizeof(nrm_msg_actuate_t));
	if (ret == NULL)
		return ret;
	nrm_msg_actuate_init(ret);
	ret->uuid = strdup(uuid);
	ret->value = value;
	return ret;
}

void nrm_msg_actuate_destroy(nrm_msg_actuate_t *msg)
{
	if (msg == NULL)
		return;

	free(msg->uuid);
	free(msg);
}

nrm_msg_sensor_t *nrm_msg_sensor_new(const char *uuid)
{
	nrm_msg_sensor_t *ret = calloc(1, sizeof(nrm_msg_sensor_t));
	if (ret == NULL)
		return ret;
	nrm_msg_sensor_init(ret);
	ret->uuid = strdup(uuid);
	return ret;
}

void nrm_msg_sensor_destroy(nrm_msg_sensor_t *msg)
{
	if (msg == NULL)
		return;

	free(msg->uuid);
	free(msg);
}

nrm_msg_slice_t *nrm_msg_slice_new(const char *uuid)
{
	nrm_msg_slice_t *ret = calloc(1, sizeof(nrm_msg_slice_t));
	if (ret == NULL)
		return ret;
	nrm_msg_slice_init(ret);
	ret->uuid = strdup(uuid);
	return ret;
}

void nrm_msg_slice_destroy(nrm_msg_slice_t *msg)
{
	if (msg == NULL)
		return;

	free(msg->uuid);
	free(msg);
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

nrm_msg_actuator_t *nrm_msg_actuator_new(nrm_actuator_t *actuator)
{
	nrm_msg_actuator_t *ret = calloc(1, sizeof(nrm_msg_actuator_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_actuator_init(ret);
	ret->uuid = strdup(actuator->uuid);
	if (actuator->clientid)
		ret->clientid = strdup(nrm_uuid_to_char(actuator->clientid));
	else
		ret->clientid = NULL;
	ret->value = actuator->value;
	nrm_vector_length(actuator->choices, &ret->n_choices);
	ret->choices = calloc(ret->n_choices, sizeof(double));
	assert(ret->choices);
	for (size_t i = 0; i < ret->n_choices; i++) {
		double *d;
		nrm_vector_get_withtype(double, actuator->choices, i, d);
		ret->choices[i] = *d;
	}
	return ret;
}

void nrm_msg_actuator_destroy(nrm_msg_actuator_t *msg)
{
	if (msg == NULL)
		return;

	free(msg->uuid);
	free(msg->clientid);
	free(msg->choices);
	free(msg);
}

nrm_msg_scope_t *nrm_msg_scope_new(nrm_scope_t *scope)
{
	nrm_msg_scope_t *ret = calloc(1, sizeof(nrm_msg_scope_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_scope_init(ret);
	ret->uuid = strdup(scope->uuid);
	nrm_bitmap_to_array(&scope->maps[NRM_SCOPE_TYPE_CPU], &ret->n_cpus,
	                    &ret->cpus);
	nrm_bitmap_to_array(&scope->maps[NRM_SCOPE_TYPE_NUMA], &ret->n_numas,
	                    &ret->numas);
	nrm_bitmap_to_array(&scope->maps[NRM_SCOPE_TYPE_GPU], &ret->n_gpus,
	                    &ret->gpus);
	return ret;
}

void nrm_msg_scope_destroy(nrm_msg_scope_t *msg)
{
	if (msg == NULL)
		return;

	free(msg->uuid);
	free(msg->cpus);
	free(msg->numas);
	free(msg->gpus);
	free(msg);
}

nrm_msg_event_t *nrm_msg_event_new(nrm_event_t *event)
{
	nrm_msg_event_t *ret = calloc(1, sizeof(nrm_msg_event_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_event_init(ret);
	ret->time = nrm_time_tons(&event->time);
	ret->value = event->value;
	return ret;
}

void nrm_msg_event_destroy(nrm_msg_event_t *msg)
{
	if (msg == NULL)
		return;

	free(msg);
}

nrm_msg_timeserie_t *nrm_msg_timeserie_new(nrm_timeserie_t *timeserie)
{
	nrm_msg_timeserie_t *ret = calloc(1, sizeof(nrm_msg_timeserie_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_timeserie_init(ret);
	ret->sensor_uuid = strdup(timeserie->sensor_uuid);
	ret->scope = nrm_msg_scope_new(timeserie->scope);
	ret->start = nrm_time_tons(&timeserie->start);
	nrm_vector_length(timeserie->events, &ret->n_events);
	nrm_log_debug("vector contains %zu\n", ret->n_events);
	ret->events = calloc(ret->n_events, sizeof(nrm_msg_event_t));
	assert(ret->events);
	for (size_t i = 0; i < ret->n_events; i++) {
		nrm_event_t *e;
		nrm_vector_get_withtype(nrm_event_t, timeserie->events, i, e);
		ret->events[i] = nrm_msg_event_new(e);
	}
	return ret;	
}

void nrm_msg_timeserie_destroy(nrm_msg_timeserie_t *msg)
{
	if (msg == NULL)
		return;

	free(msg->sensor_uuid);
	nrm_msg_scope_destroy(msg->scope);
	for (size_t i = 0; i < msg->n_events; i++)
		nrm_msg_event_destroy(msg->events[i]);
	free(msg->events);
	free(msg);
}

nrm_msg_timeserielist_t *nrm_msg_timeserielist_new(nrm_vector_t *timeseries)
{
	nrm_msg_timeserielist_t *ret = calloc(1, sizeof(nrm_msg_timeserielist_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_timeserielist_init(ret);
	if (timeseries == NULL) {
		ret->n_series = 0;
		return ret;
	}
	nrm_vector_length(timeseries, &ret->n_series);
	nrm_log_debug("vector contains %zu\n", ret->n_series);
	ret->series = calloc(ret->n_series, sizeof(nrm_msg_timeserie_t *));
	assert(ret->series);
	for (size_t i = 0; i < ret->n_series; i++) {
		nrm_timeserie_t **s;
		nrm_vector_get_withtype(nrm_timeserie_t *, timeseries, i, s);
		ret->series[i] = nrm_msg_timeserie_new(*s);
	}
	return ret;

}

void nrm_msg_timeserielist_destroy(nrm_msg_timeserielist_t *msg)
{
	if (msg == NULL)
		return;

	free(msg->series);
	free(msg);
}

int nrm_msg_set_events(nrm_msg_t *msg,
		       nrm_vector_t *timeseries)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->events = nrm_msg_timeserielist_new(timeseries);
	assert(msg->events);
	msg->data_case = NRM__MESSAGE__DATA_EVENTS;
	return 0;
}

int nrm_msg_set_actuate(nrm_msg_t *msg, nrm_string_t uuid, double value)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->data_case = NRM__MESSAGE__DATA_ACTUATE;
	msg->actuate = nrm_msg_actuate_new(uuid, value);
	return 0;
}

int nrm_msg_set_add_actuator(nrm_msg_t *msg, nrm_actuator_t *actuator)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->add = nrm_msg_add_new(NRM_MSG_TARGET_TYPE_ACTUATOR);
	assert(msg->add);
	msg->data_case = NRM__MESSAGE__DATA_ADD;
	msg->add->data_case = NRM__ADD__DATA_ACTUATOR;
	msg->add->actuator = nrm_msg_actuator_new(actuator);
	return 0;
}

int nrm_msg_set_add_scope(nrm_msg_t *msg, nrm_scope_t *scope)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->add = nrm_msg_add_new(NRM_MSG_TARGET_TYPE_SCOPE);
	assert(msg->add);
	msg->data_case = NRM__MESSAGE__DATA_ADD;
	msg->add->data_case = NRM__ADD__DATA_SCOPE;
	msg->add->scope = nrm_msg_scope_new(scope);
	return 0;
}

int nrm_msg_set_add_sensor(nrm_msg_t *msg, nrm_sensor_t *sensor)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->add = nrm_msg_add_new(NRM_MSG_TARGET_TYPE_SENSOR);
	assert(msg->add);
	msg->data_case = NRM__MESSAGE__DATA_ADD;
	msg->add->data_case = NRM__ADD__DATA_SENSOR;
	msg->add->sensor = nrm_msg_sensor_new(sensor->uuid);
	return 0;
}

int nrm_msg_set_add_slice(nrm_msg_t *msg, nrm_slice_t *slice)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->add = nrm_msg_add_new(NRM_MSG_TARGET_TYPE_SLICE);
	assert(msg->add);
	msg->data_case = NRM__MESSAGE__DATA_ADD;
	msg->add->data_case = NRM__ADD__DATA_SLICE;
	msg->add->slice = nrm_msg_slice_new(slice->uuid);
	return 0;
}

void nrm_msg_add_destroy(nrm_msg_add_t *msg)
{
	if (msg == NULL)
		return;

	switch (msg->type) {
	case NRM_MSG_TARGET_TYPE_SLICE:
		nrm_msg_slice_destroy(msg->slice);
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		nrm_msg_sensor_destroy(msg->sensor);
		break;
	case NRM_MSG_TARGET_TYPE_SCOPE:
		nrm_msg_scope_destroy(msg->scope);
		break;
	case NRM_MSG_TARGET_TYPE_ACTUATOR:
		nrm_msg_actuator_destroy(msg->actuator);
		break;
	default:
		break;
	}
	free(msg);
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

nrm_msg_actuatorlist_t *nrm_msg_actuatorlist_new(nrm_vector_t *actuators)
{
	nrm_msg_actuatorlist_t *ret = calloc(1, sizeof(nrm_msg_actuatorlist_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_actuatorlist_init(ret);
	if (actuators == NULL) {
		ret->n_actuators = 0;
		return ret;
	}
	nrm_vector_length(actuators, &ret->n_actuators);
	nrm_log_debug("vector contains %zu\n", ret->n_actuators);
	ret->actuators = calloc(ret->n_actuators, sizeof(nrm_msg_actuator_t *));
	assert(ret->actuators);
	for (size_t i = 0; i < ret->n_actuators; i++) {
		nrm_actuator_t *s;
		nrm_vector_get_withtype(nrm_actuator_t, actuators, i, s);
		ret->actuators[i] = nrm_msg_actuator_new(s);
	}
	return ret;
}

void nrm_msg_actuatorlist_destroy(nrm_msg_actuatorlist_t *msg)
{
	if (msg == NULL)
		return;

	for (size_t i = 0; i < msg->n_actuators; i++)
		nrm_msg_actuator_destroy(msg->actuators[i]);
	free(msg->actuators);
	free(msg);
}

nrm_msg_scopelist_t *nrm_msg_scopelist_new(nrm_vector_t *scopes)
{
	nrm_msg_scopelist_t *ret = calloc(1, sizeof(nrm_msg_scopelist_t));
	if (ret == NULL)
		return NULL;
	nrm_msg_scopelist_init(ret);
	if (scopes == NULL) {
		ret->n_scopes = 0;
		return ret;
	}
	nrm_vector_length(scopes, &ret->n_scopes);
	nrm_log_debug("vector contains %zu\n", ret->n_scopes);
	ret->scopes = calloc(ret->n_scopes, sizeof(nrm_msg_scope_t *));
	assert(ret->scopes);
	for (size_t i = 0; i < ret->n_scopes; i++) {
		nrm_scope_t *s;
		nrm_vector_get_withtype(nrm_scope_t, scopes, i, s);
		ret->scopes[i] = nrm_msg_scope_new(s);
	}
	return ret;
}

void nrm_msg_scopelist_destroy(nrm_msg_scopelist_t *msg)
{
	if (msg == NULL)
		return;

	for (size_t i = 0; i < msg->n_scopes; i++)
		nrm_msg_scope_destroy(msg->scopes[i]);
	free(msg->scopes);
	free(msg);
}

nrm_msg_sensorlist_t *nrm_msg_sensorlist_new(nrm_vector_t *sensors)
{
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
		nrm_sensor_t *s;
		nrm_vector_get_withtype(nrm_sensor_t, sensors, i, s);
		nrm_log_debug("packed sensor %zu %s\n", i, s->uuid);
		ret->sensors[i] = nrm_msg_sensor_new(s->uuid);
	}
	return ret;
}

void nrm_msg_sensorlist_destroy(nrm_msg_sensorlist_t *msg)
{
	if (msg == NULL)
		return;

	for (size_t i = 0; i < msg->n_sensors; i++)
		nrm_msg_sensor_destroy(msg->sensors[i]);
	free(msg->sensors);
	free(msg);
}

nrm_msg_slicelist_t *nrm_msg_slicelist_new(nrm_vector_t *slices)
{
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
		nrm_slice_t *s;
		nrm_vector_get_withtype(nrm_slice_t, slices, i, s);
		nrm_log_debug("packed slice %zu %s\n", i, s->uuid);
		ret->slices[i] = nrm_msg_slice_new(s->uuid);
	}
	return ret;
}

void nrm_msg_slicelist_destroy(nrm_msg_slicelist_t *msg)
{
	if (msg == NULL)
		return;

	for (size_t i = 0; i < msg->n_slices; i++)
		nrm_msg_slice_destroy(msg->slices[i]);
	free(msg->slices);
	free(msg);
}

int nrm_msg_set_list_actuators(nrm_msg_t *msg, nrm_vector_t *actuators)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->list = nrm_msg_list_new(NRM_MSG_TARGET_TYPE_ACTUATOR);
	assert(msg->list);
	msg->data_case = NRM__MESSAGE__DATA_LIST;
	msg->list->data_case = NRM__LIST__DATA_ACTUATORS;
	msg->list->actuators = nrm_msg_actuatorlist_new(actuators);
	return 0;
}

int nrm_msg_set_list_scopes(nrm_msg_t *msg, nrm_vector_t *scopes)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->list = nrm_msg_list_new(NRM_MSG_TARGET_TYPE_SCOPE);
	assert(msg->list);
	msg->data_case = NRM__MESSAGE__DATA_LIST;
	msg->list->data_case = NRM__LIST__DATA_SCOPES;
	msg->list->scopes = nrm_msg_scopelist_new(scopes);
	return 0;
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

void nrm_msg_list_destroy(nrm_msg_list_t *msg)
{
	if (msg == NULL)
		return;

	switch (msg->type) {
	case NRM_MSG_TARGET_TYPE_SLICE:
		nrm_msg_slicelist_destroy(msg->slices);
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		nrm_msg_sensorlist_destroy(msg->sensors);
		break;
	case NRM_MSG_TARGET_TYPE_SCOPE:
		nrm_msg_scopelist_destroy(msg->scopes);
		break;
	case NRM_MSG_TARGET_TYPE_ACTUATOR:
		nrm_msg_actuatorlist_destroy(msg->actuators);
		break;
	default:
		break;
	}
	free(msg);
}

nrm_msg_remove_t *nrm_msg_remove_new(int type, nrm_string_t uuid)
{
	nrm_msg_remove_t *ret = calloc(1, sizeof(nrm_msg_remove_t));
	if (ret == NULL)
		return ret;
	nrm_msg_remove_init(ret);
	ret->type = type;
	ret->uuid = strdup(uuid);
	return ret;
}

void nrm_msg_remove_destroy(nrm_msg_remove_t *msg)
{
	if (msg == NULL)
		return;

	free(msg->uuid);
	free(msg);
}

int nrm_msg_set_remove(nrm_msg_t *msg, int type, nrm_string_t uuid)
{
	if (msg == NULL)
		return -NRM_EINVAL;
	msg->remove = nrm_msg_remove_new(type, uuid);
	assert(msg->remove);
	msg->data_case = NRM__MESSAGE__DATA_REMOVE;
	return 0;
}

void nrm_msg_destroy_created(nrm_msg_t **msg)
{
	if (msg == NULL || *msg == NULL)
		return;

	nrm_msg_t *sub = *msg;
	switch (sub->type) {
	case NRM_MSG_TYPE_ACTUATE:
		nrm_msg_actuate_destroy(sub->actuate);
		break;
	case NRM_MSG_TYPE_ADD:
		nrm_msg_add_destroy(sub->add);
		break;
	case NRM_MSG_TYPE_LIST:
		nrm_msg_list_destroy(sub->list);
		break;
	case NRM_MSG_TYPE_EVENTS:
		nrm_msg_timeserielist_destroy(sub->events);
		break;
	case NRM_MSG_TYPE_REMOVE:
		nrm_msg_remove_destroy(sub->remove);
		break;
	case NRM_MSG_TYPE_ACK:
	case NRM_MSG_TYPE_EXIT:
	case NRM_MSG_TYPE_TICK:
	default:
		break;
	}
	free(*msg);
	*msg = NULL;
}

/*******************************************************************************
 * Protobuf Management: Parsing Messages
 *******************************************************************************/

nrm_actuator_t *nrm_actuator_create_frommsg(nrm_msg_actuator_t *msg)
{
	if (msg == NULL)
		return NULL;
	nrm_actuator_t *ret = nrm_actuator_create(msg->uuid);
	if (msg->clientid)
		ret->clientid = nrm_uuid_create_fromchar(msg->clientid);
	ret->value = msg->value;
	nrm_vector_resize(ret->choices, msg->n_choices);
	nrm_vector_clear(ret->choices);
	for (size_t i = 0; i < msg->n_choices; i++)
		nrm_vector_push_back(ret->choices, &msg->choices[i]);
	nrm_vector_sort(ret->choices, nrm_vector_sort_double_cmp);
	return ret;
}

nrm_scope_t *nrm_scope_create_frommsg(nrm_msg_scope_t *msg)
{
	if (msg == NULL)
		return NULL;
	nrm_scope_t *ret = nrm_scope_create(msg->uuid);
	nrm_bitmap_from_array(&ret->maps[NRM_SCOPE_TYPE_CPU], msg->n_cpus,
	                      msg->cpus);
	nrm_bitmap_from_array(&ret->maps[NRM_SCOPE_TYPE_NUMA], msg->n_numas,
	                      msg->numas);
	nrm_bitmap_from_array(&ret->maps[NRM_SCOPE_TYPE_GPU], msg->n_gpus,
	                      msg->gpus);
	return ret;
}

nrm_sensor_t *nrm_sensor_create_frommsg(nrm_msg_sensor_t *msg)
{
	if (msg == NULL)
		return NULL;
	nrm_sensor_t *ret = nrm_sensor_create(msg->uuid);
	return ret;
}

nrm_slice_t *nrm_slice_create_frommsg(nrm_msg_slice_t *msg)
{
	if (msg == NULL)
		return NULL;
	nrm_slice_t *ret = nrm_slice_create(msg->uuid);
	return ret;
}

int nrm_actuator_update_frommsg(nrm_actuator_t *actuator,
                                nrm_msg_actuator_t *msg)
{
	if (actuator == NULL || msg == NULL)
		return -NRM_EINVAL;

	actuator->uuid = nrm_string_fromchar(msg->uuid);
	if (msg->clientid)
		actuator->clientid = nrm_uuid_create_fromchar(msg->clientid);
	actuator->value = msg->value;
	if (actuator->choices)
		nrm_vector_destroy(&actuator->choices);
	nrm_vector_create(&actuator->choices, sizeof(double));
	nrm_vector_resize(actuator->choices, msg->n_choices);
	nrm_vector_clear(actuator->choices);
	for (size_t i = 0; i < msg->n_choices; i++)
		nrm_vector_push_back(actuator->choices, &msg->choices[i]);
	return 0;
}

int nrm_scope_update_frommsg(nrm_scope_t *scope, nrm_msg_scope_t *msg)
{
	if (scope == NULL || msg == NULL)
		return -NRM_EINVAL;
	nrm_bitmap_from_array(&scope->maps[NRM_SCOPE_TYPE_CPU], msg->n_cpus,
	                      msg->cpus);
	nrm_bitmap_from_array(&scope->maps[NRM_SCOPE_TYPE_NUMA], msg->n_numas,
	                      msg->numas);
	nrm_bitmap_from_array(&scope->maps[NRM_SCOPE_TYPE_GPU], msg->n_gpus,
	                      msg->gpus);
	return 0;
}

int nrm_sensor_update_frommsg(nrm_sensor_t *sensor, nrm_msg_sensor_t *msg)
{
	if (sensor == NULL || msg == NULL)
		return -NRM_EINVAL;
	return 0;
}

int nrm_slice_update_frommsg(nrm_slice_t *slice, nrm_msg_slice_t *msg)
{
	if (slice == NULL || msg == NULL)
		return -NRM_EINVAL;
	return 0;
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
	assert(frame != NULL);
	char *identity = (char *)zframe_data(frame);
	size_t len = zframe_size(frame);
	assert(identity != NULL);
	nrm_uuid_t *id = nrm_uuid_create_frombuf(identity, len);
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

static int nrm_msg_push_topic(zmsg_t *zm, char *topic)
{
	zframe_t *frame = zframe_from(topic);
	zmsg_append(zm, &frame);
	return 0;
}

static int nrm_msg_pop_topic(zmsg_t *zm, nrm_string_t *topic)
{
	zframe_t *frame = zmsg_pop(zm);
	*topic = nrm_string_fromchar((char *)zframe_data(frame));
	zframe_destroy(&frame);
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

int nrm_msg_pub(zsock_t *socket, nrm_string_t topic, nrm_msg_t *msg)
{
	zmsg_t *zm = zmsg_new();
	if (zm == NULL)
		return -NRM_ENOMEM;
	nrm_msg_push_topic(zm, topic);
	nrm_msg_push_packed_frames(zm, msg);
	return zmsg_send(&zm, socket);
}

nrm_msg_t *nrm_msg_sub(zsock_t *socket, nrm_string_t *topic)
{
	zmsg_t *zm = zmsg_recv(socket);
	assert(zm);
	nrm_msg_t *msg = NULL;
	nrm_msg_pop_topic(zm, topic);
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

/* clang-format off */
static const nrm_msg_type_table_t nrm_msg_type_table[] = {
        {NRM_MSG_TYPE_ACK, "ACK"},
        {NRM_MSG_TYPE_LIST, "LIST"},
        {NRM_MSG_TYPE_ADD, "ADD"},
        {NRM_MSG_TYPE_REMOVE, "REMOVE"},
        {NRM_MSG_TYPE_EVENTS, "EVENTS"},
        {NRM_MSG_TYPE_ACTUATE, "ACTUATE"},
        {NRM_MSG_TYPE_EXIT, "EXIT"},
	{NRM_MSG_TYPE_TICK, "TICK"},
        {0, NULL},
};
/* clang-format on */

static const nrm_msg_type_table_t nrm_msg_target_table[] = {
        {NRM_MSG_TARGET_TYPE_ACTUATOR, "ACTUATOR"},
        {NRM_MSG_TARGET_TYPE_SLICE, "SLICE"},
        {NRM_MSG_TARGET_TYPE_SENSOR, "SENSOR"},
        {NRM_MSG_TARGET_TYPE_SCOPE, "SCOPE"},
        {0, NULL},
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

json_t *nrm_msg_bitmap_to_json(size_t nitems, int32_t *items)
{
	json_t *ret;
	ret = json_array();
	for (size_t i = 0; i < nitems; i++)
		json_array_append_new(ret, json_integer(items[i]));
	return ret;
}

json_t *nrm_msg_darray_to_json(size_t nitems, double *items)
{
	json_t *ret;
	ret = json_array();
	for (size_t i = 0; i < nitems; i++)
		json_array_append_new(ret, json_real(items[i]));
	return ret;
}

json_t *nrm_msg_actuator_to_json(nrm_msg_actuator_t *msg)
{
	json_t *ret;
	json_t *choices;
	choices = nrm_msg_darray_to_json(msg->n_choices, msg->choices);
	ret = json_pack("{s:s, s:s?, s:o, s:o}", "uuid", msg->uuid, "clientid",
	                msg->clientid, "value", json_real(msg->value),
	                "choices", choices);
	return ret;
}

json_t *nrm_msg_actuatorlist_to_json(nrm_msg_actuatorlist_t *msg)
{
	json_t *ret;
	ret = json_array();
	for (size_t i = 0; i < msg->n_actuators; i++) {
		json_array_append_new(
		        ret, nrm_msg_actuator_to_json(msg->actuators[i]));
	}
	return ret;
}

json_t *nrm_msg_scope_to_json(nrm_msg_scope_t *msg)
{
	json_t *ret;
	json_t *cpus, *gpus, *numas;
	cpus = nrm_msg_bitmap_to_json(msg->n_cpus, msg->cpus);
	numas = nrm_msg_bitmap_to_json(msg->n_numas, msg->numas);
	gpus = nrm_msg_bitmap_to_json(msg->n_gpus, msg->gpus);
	ret = json_pack("{s:s, s:o, s:o, s:o}", "uuid", msg->uuid, "cpu", cpus,
	                "numa", numas, "gpu", gpus);
	return ret;
}

json_t *nrm_msg_scopelist_to_json(nrm_msg_scopelist_t *msg)
{
	json_t *ret;
	ret = json_array();
	for (size_t i = 0; i < msg->n_scopes; i++) {
		json_array_append_new(ret,
		                      nrm_msg_scope_to_json(msg->scopes[i]));
	}
	return ret;
}

json_t *nrm_msg_sensor_to_json(nrm_msg_sensor_t *msg)
{
	json_t *ret;
	ret = json_pack("{s:s}", "uuid", msg->uuid);
	return ret;
}

json_t *nrm_msg_sensorlist_to_json(nrm_msg_sensorlist_t *msg)
{
	json_t *ret;
	ret = json_array();
	for (size_t i = 0; i < msg->n_sensors; i++) {
		json_array_append_new(ret,
		                      nrm_msg_sensor_to_json(msg->sensors[i]));
	}
	return ret;
}

json_t *nrm_msg_slice_to_json(nrm_msg_slice_t *msg)
{
	json_t *ret;
	ret = json_pack("{s:s}", "uuid", msg->uuid);
	return ret;
}

json_t *nrm_msg_slicelist_to_json(nrm_msg_slicelist_t *msg)
{
	json_t *ret;
	ret = json_array();
	for (size_t i = 0; i < msg->n_slices; i++) {
		json_array_append_new(ret,
		                      nrm_msg_slice_to_json(msg->slices[i]));
	}
	return ret;
}

json_t *nrm_msg_event_to_json(nrm_msg_event_t *msg)
{
	json_t *ret;
	ret = json_pack("{s:I, s:f}", "time", msg->time, "value", msg->value);
	return ret;
}

json_t *nrm_msg_timeserie_to_json(nrm_msg_timeserie_t *msg)
{
	json_t *ret;
	json_t *scope;
	json_t *events;
	events = json_array();
	for (size_t i = 0; i < msg->n_events; i++) {
		json_array_append_new(events,
				      nrm_msg_event_to_json(msg->events[i]));
	}
	scope = nrm_msg_scope_to_json(msg->scope);
	ret = json_pack("{s:s, s:o, s:I, s:o}", "sensor_uuid", msg->sensor_uuid,
			"scope_uuid", scope, "start", msg->start,
			"events", events);
	return ret;
}

json_t *nrm_msg_timeserielist_to_json(nrm_msg_timeserielist_t *msg)
{
	json_t *ret;
	ret = json_array();
	for (size_t i = 0; i < msg->n_series; i++) {
		json_array_append_new(ret,
				      nrm_msg_timeserie_to_json(msg->series[i]));
	}
	return ret;
}

json_t *nrm_msg_add_to_json(nrm_msg_add_t *msg)
{
	json_t *ret;
	json_t *sub;
	switch (msg->type) {
	case NRM_MSG_TARGET_TYPE_ACTUATOR:
		sub = nrm_msg_actuator_to_json(msg->actuator);
		break;
	case NRM_MSG_TARGET_TYPE_SLICE:
		sub = nrm_msg_slice_to_json(msg->slice);
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		sub = nrm_msg_sensor_to_json(msg->sensor);
		break;
	case NRM_MSG_TARGET_TYPE_SCOPE:
		sub = nrm_msg_scope_to_json(msg->scope);
		break;
	default:
		sub = NULL;
		break;
	}
	ret = json_pack("{s:s, s:o?}", "type",
	                nrm_msg_type_t2s(msg->type, nrm_msg_target_table),
	                "data", sub);
	return ret;
}

json_t *nrm_msg_list_to_json(nrm_msg_list_t *msg)
{
	json_t *ret;
	json_t *sub;
	switch (msg->type) {
	case NRM_MSG_TARGET_TYPE_ACTUATOR:
		sub = nrm_msg_actuatorlist_to_json(msg->actuators);
		break;
	case NRM_MSG_TARGET_TYPE_SLICE:
		sub = nrm_msg_slicelist_to_json(msg->slices);
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		sub = nrm_msg_sensorlist_to_json(msg->sensors);
		break;
	case NRM_MSG_TARGET_TYPE_SCOPE:
		sub = nrm_msg_scopelist_to_json(msg->scopes);
		break;
	default:
		sub = NULL;
		break;
	}
	ret = json_pack("{s:s, s:o?}", "type",
	                nrm_msg_type_t2s(msg->type, nrm_msg_target_table),
	                "data", sub);
	return ret;
}

json_t *nrm_msg_remove_to_json(nrm_msg_remove_t *msg)
{
	json_t *ret;
	ret = json_pack("{s:s, s:s?}", "type",
	                nrm_msg_type_t2s(msg->type, nrm_msg_target_table),
	                "uuid", msg->uuid);
	return ret;
}

json_t *nrm_msg_actuate_to_json(nrm_msg_actuate_t *msg)
{
	json_t *ret;
	ret = json_pack("{s:s, s:f}", "uuid", msg->uuid, "value", msg->value);
	return ret;
}

json_t *nrm_msg_to_json(nrm_msg_t *msg)
{
	json_t *ret;
	json_t *sub;
	switch (msg->type) {
	case NRM_MSG_TYPE_ACTUATE:
		sub = nrm_msg_actuate_to_json(msg->actuate);
		break;
	case NRM_MSG_TYPE_ADD:
		sub = nrm_msg_add_to_json(msg->add);
		break;
	case NRM_MSG_TYPE_LIST:
		sub = nrm_msg_list_to_json(msg->list);
		break;
	case NRM_MSG_TYPE_EVENTS:
		sub = nrm_msg_timeserielist_to_json(msg->events);
		break;
	case NRM_MSG_TYPE_REMOVE:
		sub = nrm_msg_remove_to_json(msg->remove);
		break;
	default:
		sub = NULL;
		break;
	}
	ret = json_pack("{s:s, s:o?}", "type",
	                nrm_msg_type_t2s(msg->type, nrm_msg_type_table), "data",
	                sub);
	return ret;
}

int nrm_msg_fprintf(FILE *f, nrm_msg_t *msg)
{
	json_t *json;
	json = nrm_msg_to_json(msg);
	char *s = json_dumps(json, 0);
	fprintf(f, "%s\n", s);
	json_decref(json);
	free(s);
	return 0;
}

int nrm_msg_is_reply(nrm_msg_t *msg)
{
	switch (msg->type) {
	case NRM_MSG_TYPE_ACTUATE:
	case NRM_MSG_TYPE_TICK:
		return 0;
	default:
		return 1;
	}
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
        {NRM_CTRLMSG_TYPE_TERM, NRM_CTRLMSG_TYPE_STRING_TERM},
        {NRM_CTRLMSG_TYPE_SEND, NRM_CTRLMSG_TYPE_STRING_SEND},
        {NRM_CTRLMSG_TYPE_RECV, NRM_CTRLMSG_TYPE_STRING_RECV},
        {NRM_CTRLMSG_TYPE_PUB, NRM_CTRLMSG_TYPE_STRING_PUB},
        {NRM_CTRLMSG_TYPE_SUB, NRM_CTRLMSG_TYPE_STRING_SUB},
};

const char *nrm_ctrlmsg_t2s(int type)
{
	if (type < 0 || type > NRM_CTRLMSG_TYPE_MAX)
		return NULL;
	return nrm_ctrlmsg_table[type].s;
}

int nrm_ctrlmsg_s2t(const char *string)
{
	for (int i = 0; i < NRM_CTRLMSG_TYPE_MAX; i++)
		if (!strcmp(string, nrm_ctrlmsg_table[i].s))
			return i;
	return -1;
}

int nrm_ctrlmsg__send(zsock_t *socket, int type, void *p1, void *p2)
{
	int err;
	/* control messages are basically a control string and a pointer */
	const char *typestring = nrm_ctrlmsg_t2s(type);
	assert(typestring != NULL);
	err = zsock_send(socket, "spp", typestring, p1, p2);
	assert(err == 0);
	return err;
}

int nrm_ctrlmsg__recv(zsock_t *socket, int *type, void **p1, void **p2)
{
	int err;
	char *s;
	void *p, *q;
	err = zsock_recv(socket, "spp", &s, &p, &q);
	assert(err == 0);
	*type = nrm_ctrlmsg_s2t(s);
	nrm_log_debug("received %s:%u\n", s, *type);
	if (p1 != NULL) {
		*p1 = p;
	}
	if (p2 != NULL) {
		*p2 = q;
	}
	free(s);
	return err;
}

int nrm_ctrlmsg_sendmsg(zsock_t *socket,
                        int type,
                        nrm_msg_t *msg,
                        nrm_uuid_t *to)
{
	return nrm_ctrlmsg__send(socket, type, (void *)msg, (void *)to);
}

int nrm_ctrlmsg_pub(zsock_t *socket,
                    int type,
                    nrm_string_t topic,
                    nrm_msg_t *msg)
{
	assert(type == NRM_CTRLMSG_TYPE_PUB);
	return nrm_ctrlmsg__send(socket, type, (void *)topic, (void *)msg);
}

int nrm_ctrlmsg_sub(zsock_t *socket, int type, nrm_string_t topic)
{
	assert(type == NRM_CTRLMSG_TYPE_SUB);
	return nrm_ctrlmsg__send(socket, type, (void *)topic, NULL);
}

nrm_msg_t *nrm_ctrlmsg_recvmsg(zsock_t *socket, int *type, nrm_uuid_t **from)
{
	int err;
	void *p, *q;
	err = nrm_ctrlmsg__recv(socket, type, &p, &q);
	assert(err == 0);
	nrm_log_printmsg(NRM_LOG_DEBUG, (nrm_msg_t *)p);
	if (from != NULL) {
		*from = (nrm_uuid_t *)q;
		nrm_log_debug("from %s\n", nrm_uuid_to_char(*from));
	}
	return (nrm_msg_t *)p;
}
