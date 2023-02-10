/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include "config.h"

#include "nrm.h"

#include "internal/messages.h"
#include "internal/nrmi.h"
#include "internal/roles.h"

struct nrm_client_s {
	nrm_role_t *role;
	nrm_client_event_listener_fn *user_fn;
	nrm_client_actuate_listener_fn *actuate_fn;
	void *pyclient;
};

int nrm_client_create(nrm_client_t **client,
                      const char *uri,
                      int pub_port,
                      int rpc_port)
{
	if (client == NULL || uri == NULL)
		return -NRM_EINVAL;

	nrm_client_t *ret = calloc(1, sizeof(nrm_client_t));
	if (ret == NULL)
		return -NRM_ENOMEM;

	ret->role = nrm_role_client_create_fromparams(uri, pub_port, rpc_port);
	if (ret->role == NULL)
		return -NRM_EINVAL;
	ret->user_fn = NULL;
	ret->actuate_fn = NULL;
	ret->pyclient = NULL;

	*client = ret;
	return 0;
}

int nrm_client_actuate(const nrm_client_t *client,
                       nrm_actuator_t *actuator,
                       double value)
{
	if (client == NULL || actuator == NULL)
		return -NRM_EINVAL;

	nrm_log_debug("checking value is valid");
	size_t i, len;
	nrm_vector_length(actuator->choices, &len);
	for (i = 0; i < len; i++) {
		double d;
		void *p;
		nrm_vector_get(actuator->choices, i, &p);
		d = *(double *)p;
		if (d == value)
			break;
	}
	if (i == len) {
		nrm_log_info("value %f not in choices\n", value);
		return -NRM_EDOM;
	}

	nrm_log_debug("crafting message\n");
	/* craft the message we want to send */
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_ACTUATE);
	nrm_msg_set_actuate(msg, actuator->uuid, value);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending request\n");
	nrm_role_send(client->role, msg, NULL);

	/* wait for the answer */
	nrm_log_debug("receiving reply\n");
	msg = nrm_role_recv(client->role, NULL);
	nrm_log_debug("parsing reply:\t");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);

	assert(msg->type == NRM_MSG_TYPE_ACK);
	return 0;
}

int nrm_client_add_actuator(const nrm_client_t *client,
                            nrm_actuator_t *actuator)
{
	if (client == NULL || actuator == NULL)
		return -NRM_EINVAL;

	nrm_log_debug("crafting message\n");
	/* craft the message we want to send */
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_ADD);
	nrm_msg_set_add_actuator(msg, actuator);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending request\n");
	nrm_role_send(client->role, msg, NULL);

	/* wait for the answer */
	nrm_log_debug("receiving reply\n");
	msg = nrm_role_recv(client->role, NULL);
	nrm_log_debug("parsing reply:\t");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);

	assert(msg->type == NRM_MSG_TYPE_ADD);
	assert(msg->add->type == NRM_MSG_TARGET_TYPE_ACTUATOR);
	nrm_actuator_update_frommsg(actuator, msg->add->actuator);
	return 0;
}

int nrm_client_add_scope(const nrm_client_t *client, nrm_scope_t *scope)
{
	if (client == NULL || scope == NULL)
		return -NRM_EINVAL;

	nrm_log_debug("crafting message\n");
	/* craft the message we want to send */
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_ADD);
	nrm_msg_set_add_scope(msg, scope);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending request\n");
	nrm_role_send(client->role, msg, NULL);

	/* wait for the answer */
	nrm_log_debug("receiving reply\n");
	msg = nrm_role_recv(client->role, NULL);
	nrm_log_debug("parsing reply:\t");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);

	assert(msg->type == NRM_MSG_TYPE_ADD);
	assert(msg->add->type == NRM_MSG_TARGET_TYPE_SCOPE);
	nrm_scope_update_frommsg(scope, msg->add->scope);
	return 0;
}

int nrm_client_add_slice(const nrm_client_t *client, nrm_slice_t *slice)
{
	if (client == NULL || slice == NULL)
		return -NRM_EINVAL;

	nrm_log_debug("crafting message\n");
	/* craft the message we want to send */
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_ADD);
	nrm_msg_set_add_slice(msg, slice);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending request\n");
	nrm_role_send(client->role, msg, NULL);

	/* wait for the answer */
	nrm_log_debug("receiving reply\n");
	msg = nrm_role_recv(client->role, NULL);
	nrm_log_debug("parsing reply:\t");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);

	assert(msg->type == NRM_MSG_TYPE_ADD);
	assert(msg->add->type == NRM_MSG_TARGET_TYPE_SLICE);
	nrm_slice_update_frommsg(slice, msg->add->slice);
	return 0;
}

int nrm_client_add_sensor(const nrm_client_t *client, nrm_sensor_t *sensor)
{
	if (client == NULL || sensor == NULL)
		return -NRM_EINVAL;

	nrm_log_debug("crafting message\n");
	/* craft the message we want to send */
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_ADD);
	nrm_msg_set_add_sensor(msg, sensor);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending request\n");
	nrm_role_send(client->role, msg, NULL);

	/* wait for the answer */
	nrm_log_debug("receiving reply\n");
	msg = nrm_role_recv(client->role, NULL);
	nrm_log_debug("parsing reply:\t");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);

	assert(msg->type == NRM_MSG_TYPE_ADD);
	assert(msg->add->type == NRM_MSG_TARGET_TYPE_SENSOR);
	nrm_sensor_update_frommsg(sensor, msg->add->sensor);
	return 0;
}

int nrm_client_find(const nrm_client_t *client,
                    int type,
                    const char *uuid,
                    nrm_vector_t **results)
{
	if (client == NULL || type < 0 || type >= NRM_MSG_TARGET_TYPE_MAX)
		return -NRM_EINVAL;

	/* we need one of those */
	if (uuid == NULL)
		return -NRM_EINVAL;

	int err;
	/* craft the message we want to send */
	nrm_log_debug("crafting message\n");
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_LIST);
	switch (type) {
	case NRM_MSG_TARGET_TYPE_ACTUATOR:
		nrm_msg_set_list_actuators(msg, NULL);
		break;
	case NRM_MSG_TARGET_TYPE_SCOPE:
		nrm_msg_set_list_scopes(msg, NULL);
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		nrm_msg_set_list_sensors(msg, NULL);
		break;
	case NRM_MSG_TARGET_TYPE_SLICE:
		nrm_msg_set_list_slices(msg, NULL);
		break;
	default:
		nrm_log_error("missing case for type %d\n", type);
		assert(0);
	}
	assert(msg->type == NRM_MSG_TYPE_LIST);
	assert(msg->list->type == type);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending request\n");
	nrm_role_send(client->role, msg, NULL);

	/* wait for the answer */
	nrm_log_debug("receiving reply\n");
	msg = nrm_role_recv(client->role, NULL);
	nrm_log_debug("parsing reply\n");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	assert(msg->type == NRM_MSG_TYPE_LIST);
	assert(msg->list->type == type);

	nrm_vector_t *ret;
	if (type == NRM_MSG_TARGET_TYPE_ACTUATOR) {
		err = nrm_vector_create(&ret, sizeof(nrm_actuator_t));
		if (err)
			return err;

		for (size_t i = 0; i < msg->list->actuators->n_actuators; i++) {
			if (uuid != NULL &&
			    strcmp(uuid,
			           msg->list->actuators->actuators[i]->uuid))
				continue;
			nrm_actuator_t *s = nrm_actuator_create_frommsg(
			        msg->list->actuators->actuators[i]);
			nrm_vector_push_back(ret, s);
		}
	} else if (type == NRM_MSG_TARGET_TYPE_SCOPE) {
		err = nrm_vector_create(&ret, sizeof(nrm_scope_t));
		if (err)
			return err;

		for (size_t i = 0; i < msg->list->scopes->n_scopes; i++) {
			if (strcmp(uuid, msg->list->scopes->scopes[i]->uuid))
				continue;
			nrm_scope_t *s = nrm_scope_create_frommsg(
			        msg->list->scopes->scopes[i]);
			nrm_vector_push_back(ret, s);
		}
	} else if (type == NRM_MSG_TARGET_TYPE_SENSOR) {
		err = nrm_vector_create(&ret, sizeof(nrm_sensor_t));
		if (err)
			return err;

		for (size_t i = 0; i < msg->list->sensors->n_sensors; i++) {
			if (strcmp(uuid, msg->list->sensors->sensors[i]->uuid))
				continue;
			nrm_sensor_t *s = nrm_sensor_create_frommsg(
			        msg->list->sensors->sensors[i]);
			nrm_vector_push_back(ret, s);
		}
	} else if (type == NRM_MSG_TARGET_TYPE_SLICE) {
		err = nrm_vector_create(&ret, sizeof(nrm_slice_t));
		if (err)
			return err;

		for (size_t i = 0; i < msg->list->slices->n_slices; i++) {
			if (strcmp(uuid, msg->list->slices->slices[i]->uuid))
				continue;
			nrm_slice_t *s = nrm_slice_create_frommsg(
			        msg->list->slices->slices[i]);
			nrm_vector_push_back(ret, s);
		}
	}
	*results = ret;
	return 0;
}

int nrm_client__sub_callback(nrm_msg_t *msg, void *arg)
{
	nrm_client_t *self = (nrm_client_t *)arg;
	if (self->user_fn == NULL)
		return 0;
	nrm_string_t uuid = nrm_string_fromchar(msg->event->uuid);
	nrm_time_t time = nrm_time_fromns(msg->event->time);
	nrm_scope_t *scope = nrm_scope_create_frommsg(msg->event->scope);
	self->user_fn(uuid, time, scope, msg->event->value, arg);
	return 0;
}

int nrm_client__sub_pycallback(nrm_msg_t *msg, void *arg)
{
	nrm_client_t *self = (nrm_client_t *)arg;
	if (self->user_fn == NULL)
		return 0;
	nrm_string_t uuid = nrm_string_fromchar(msg->event->uuid);
	nrm_time_t time = nrm_time_fromns(msg->event->time);
	nrm_scope_t *scope = nrm_scope_create_frommsg(msg->event->scope);
	self->user_fn(uuid, time, scope, msg->event->value, self->pyclient);  // _event_listener_wrap() from client.py should get called?
	return 0;
}

int nrm_client_set_event_listener(nrm_client_t *client,
                                  nrm_client_event_listener_fn *fn)
{
	if (client == NULL || fn == NULL)
		return -NRM_EINVAL;
	client->user_fn = fn;
	return 0;
}

int nrm_client_set_event_Pylistener(nrm_client_t *client,
									void *pyclient,
                                    nrm_client_event_listener_fn *fn)
{
	if (client == NULL || pyclient == NULL || fn == NULL)
		return -NRM_EINVAL;
	client->user_fn = fn;
	client->pyclient = pyclient;
	return 0;
}

int nrm_client_start_event_Pylistener(nrm_client_t *client,
                                      nrm_string_t topic)
{
	if (client == NULL)
		return -NRM_EINVAL;
	nrm_role_register_sub_cb(client->role, nrm_client__sub_pycallback,
	                         (void *)client);
	nrm_role_sub(client->role, topic);
	return 0;
}

int nrm_client_set_actuate_Pylistener(nrm_client_t *client,
									  void *pyclient,
                                      nrm_client_actuate_listener_fn *fn)
{
	if (client == NULL || pyclient == NULL || fn == NULL)
		return -NRM_EINVAL;
	client->actuate_fn = fn;
	client->pyclient = pyclient;
	return 0;
}

int nrm_client_start_event_listener(nrm_client_t *client,
                                    nrm_string_t topic)
{
	if (client == NULL)
		return -NRM_EINVAL;
	nrm_role_register_sub_cb(client->role, nrm_client__sub_callback,
	                         (void *)client);
	nrm_role_sub(client->role, topic);
	return 0;
}

int nrm_client__actuate_callback(nrm_msg_t *msg, void *arg)
{
	nrm_client_t *self = (nrm_client_t *)arg;
	if (self->actuate_fn == NULL)
		return 0;
	nrm_uuid_t *uuid = nrm_uuid_create_fromchar(msg->actuate->uuid);
	self->actuate_fn(uuid, msg->event->value, arg);
	return 0;
}

int nrm_client__actuate_pycallback(nrm_msg_t *msg, void *arg)
{
	nrm_client_t *self = (nrm_client_t *)arg;
	if (self->actuate_fn == NULL)
		return 0;
	nrm_uuid_t *uuid = nrm_uuid_create_fromchar(msg->actuate->uuid);
	self->actuate_fn(uuid, msg->event->value, self->pyclient); // _actuate_listener_wrap() from client.py should get called?
	return 0;
}

int nrm_client_start_actuate_Pylistener(nrm_client_t *client)
{
	if (client == NULL)
		return -NRM_EINVAL;
	nrm_role_register_cmd_cb(client->role, nrm_client__actuate_pycallback,
	                         client);
	return 0;
}

int nrm_client_set_actuate_listener(nrm_client_t *client,
                                    nrm_client_actuate_listener_fn fn)
{
	if (client == NULL || fn == NULL)
		return -NRM_EINVAL;
	client->actuate_fn = fn;
	return 0;
}

int nrm_client_start_actuate_listener(nrm_client_t *client)
{
	if (client == NULL)
		return -NRM_EINVAL;
	nrm_role_register_cmd_cb(client->role, nrm_client__actuate_callback,
	                         client);
	return 0;
}

int nrm_client_list_actuators(const nrm_client_t *client,
                              nrm_vector_t **actuators)
{
	if (client == NULL || actuators == NULL)
		return -NRM_EINVAL;

	int err;
	/* craft the message we want to send */
	nrm_log_debug("crafting message\n");
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_LIST);
	nrm_msg_set_list_actuators(msg, NULL);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending request\n");
	nrm_role_send(client->role, msg, NULL);

	/* wait for the answer */
	nrm_log_debug("receiving reply\n");
	msg = nrm_role_recv(client->role, NULL);
	nrm_log_debug("parsing reply\n");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);

	nrm_vector_t *ret;
	err = nrm_vector_create(&ret, sizeof(nrm_actuator_t));
	if (err)
		return err;

	assert(msg->type == NRM_MSG_TYPE_LIST);
	assert(msg->list->type == NRM_MSG_TARGET_TYPE_ACTUATOR);
	for (size_t i = 0; i < msg->list->actuators->n_actuators; i++) {
		nrm_actuator_t *s = nrm_actuator_create_frommsg(
		        msg->list->actuators->actuators[i]);
		nrm_vector_push_back(ret, s);
	}
	*actuators = ret;
	return 0;
}

int nrm_client_list_scopes(const nrm_client_t *client, nrm_vector_t **scopes)
{
	if (client == NULL || scopes == NULL)
		return -NRM_EINVAL;

	int err;
	/* craft the message we want to send */
	nrm_log_debug("crafting message\n");
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_LIST);
	nrm_msg_set_list_scopes(msg, NULL);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending request\n");
	nrm_role_send(client->role, msg, NULL);

	/* wait for the answer */
	nrm_log_debug("receiving reply\n");
	msg = nrm_role_recv(client->role, NULL);
	nrm_log_debug("parsing reply\n");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);

	nrm_vector_t *ret;
	err = nrm_vector_create(&ret, sizeof(nrm_scope_t));
	if (err)
		return err;

	assert(msg->type == NRM_MSG_TYPE_LIST);
	assert(msg->list->type == NRM_MSG_TARGET_TYPE_SCOPE);
	for (size_t i = 0; i < msg->list->scopes->n_scopes; i++) {
		nrm_scope_t *s =
		        nrm_scope_create_frommsg(msg->list->scopes->scopes[i]);
		nrm_vector_push_back(ret, s);
	}
	*scopes = ret;
	return 0;
}

int nrm_client_list_sensors(const nrm_client_t *client, nrm_vector_t **sensors)
{
	if (client == NULL || sensors == NULL)
		return -NRM_EINVAL;

	int err;
	/* craft the message we want to send */
	nrm_log_debug("crafting message\n");
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_LIST);
	nrm_msg_set_list_sensors(msg, NULL);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending request\n");
	nrm_role_send(client->role, msg, NULL);

	/* wait for the answer */
	nrm_log_debug("receiving reply\n");
	msg = nrm_role_recv(client->role, NULL);
	nrm_log_debug("parsing reply\n");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);

	nrm_vector_t *ret;
	err = nrm_vector_create(&ret, sizeof(nrm_sensor_t));
	if (err)
		return err;

	assert(msg->type == NRM_MSG_TYPE_LIST);
	assert(msg->list->type == NRM_MSG_TARGET_TYPE_SENSOR);
	for (size_t i = 0; i < msg->list->sensors->n_sensors; i++) {
		nrm_sensor_t *s = nrm_sensor_create_frommsg(
		        msg->list->sensors->sensors[i]);
		nrm_vector_push_back(ret, s);
	}
	*sensors = ret;
	return 0;
}

int nrm_client_list_slices(const nrm_client_t *client, nrm_vector_t **slices)
{
	if (client == NULL || slices == NULL)
		return -NRM_EINVAL;

	int err;
	/* craft the message we want to send */
	nrm_log_debug("crafting message\n");
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_LIST);
	nrm_msg_set_list_slices(msg, NULL);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending request\n");
	nrm_role_send(client->role, msg, NULL);

	/* wait for the answer */
	nrm_log_debug("receiving reply\n");
	msg = nrm_role_recv(client->role, NULL);
	nrm_log_debug("parsing reply\n");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);

	nrm_vector_t *ret;
	err = nrm_vector_create(&ret, sizeof(nrm_slice_t));
	if (err)
		return err;

	assert(msg->type == NRM_MSG_TYPE_LIST);
	assert(msg->list->type == NRM_MSG_TARGET_TYPE_SLICE);
	for (size_t i = 0; i < msg->list->slices->n_slices; i++) {
		nrm_slice_t *s =
		        nrm_slice_create_frommsg(msg->list->slices->slices[i]);
		nrm_vector_push_back(ret, s);
	}
	*slices = ret;
	return 0;
}

int nrm_client_remove(const nrm_client_t *client, int type, nrm_string_t uuid)
{
	if (client == NULL || uuid == NULL)
		return -NRM_EINVAL;
	if (type < 0 || type > NRM_MSG_TARGET_TYPE_MAX)
		return -NRM_EINVAL;

	/* craft the message we want to send */
	nrm_log_debug("crafting message\n");
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_REMOVE);
	nrm_msg_set_remove(msg, type, uuid);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending request\n");
	nrm_role_send(client->role, msg, NULL);

	/* wait for the answer */
	nrm_log_debug("receiving reply\n");
	msg = nrm_role_recv(client->role, NULL);
	nrm_log_debug("parsing reply\n");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);

	assert(msg->type == NRM_MSG_TYPE_ACK);
	return 0;
}

int nrm_client_send_event(const nrm_client_t *client,
                          nrm_time_t time,
                          nrm_sensor_t *sensor,
                          nrm_scope_t *scope,
                          double value)
{
	if (client == NULL || sensor == NULL || scope == NULL)
		return -NRM_EINVAL;

	nrm_log_debug("crafting message\n");
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_EVENT);
	nrm_msg_set_event(msg, time, sensor->uuid, scope, value);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending request\n");
	nrm_role_send(client->role, msg, NULL);

	return 0;
}

void nrm_client_destroy(nrm_client_t **client)
{
	if (client == NULL || *client == NULL)
		return;

	nrm_client_t *c = *client;
	nrm_role_destroy(&c->role);
	free(c);
	*client = NULL;
}
