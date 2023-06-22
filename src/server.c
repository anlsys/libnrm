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

#include "nrm.h"
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signalfd.h>

#include "internal/nrmi.h"
#include "internal/messages.h"
#include "internal/roles.h"


struct nrm_server_s {
	nrm_role_t *role;
	nrm_state_t *state;
	zloop_t *loop;
	nrm_server_user_callbacks_t callbacks;
};

int nrm_server_actuate_callback(nrm_server_t *self, nrm_uuid_t *clientid,
		nrm_msg_actuate_t *msg)
{
	nrm_string_t uuid = nrm_string_fromchar(msg->uuid);
	nrm_actuator_t *a = NULL;
	nrm_hash_find(self->state->actuators, uuid, (void *)&a);
	if (a != NULL) {
		/* we use the callback to figure out if we want to allow the
		 * action or not
		 */
		int ret = self->callbacks.actuate(self, a, msg->value);
		if (ret == 0) {
			nrm_log_debug("actuating %s: %f\n", a->uuid, msg->value);
			nrm_msg_t *action = nrm_msg_create();
			nrm_msg_fill(action, NRM_MSG_TYPE_ACTUATE);
			nrm_msg_set_actuate(action, a->uuid, msg->value);
			nrm_role_send(self->role, action, a->clientid);
		}
	}
	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_ACK);
	nrm_role_send(self->role, ret, clientid);
	return 0;
}

int nrm_server_event_callback(nrm_server_t *self, nrm_uuid_t *clientid,
		nrm_msg_event_t *msg)
{
	(void)clientid;
	nrm_string_t uuid = nrm_string_fromchar(msg->uuid);
	nrm_scope_t *scope = nrm_scope_create_frommsg(msg->scope);
	nrm_time_t time = nrm_time_fromns(msg->time);
	self->callbacks.event(self, uuid, scope, time, msg->value);
	return 0;
}

nrm_msg_t *nrm_server_add_actuator(nrm_server_t *self, nrm_uuid_t *clientid, nrm_msg_actuator_t *msg)
{
	nrm_msg_t *ret = nrm_msg_create();
	nrm_actuator_t *actuator = nrm_actuator_create_frommsg(msg);
	actuator->clientid = nrm_uuid_create_fromchar(nrm_uuid_to_char(clientid));

	int err = nrm_state_add_actuator(self->state, actuator);
	if (err) {
		/* TODO: NACK */
		nrm_msg_fill(ret, NRM_MSG_TYPE_ACK);
		return ret;
	}
	nrm_msg_fill(ret, NRM_MSG_TYPE_ADD);
	nrm_msg_set_add_actuator(ret, actuator);
	return ret;
}

#define NRM_SERVER_ADD_FUNC(type) \
nrm_msg_t *nrm_server_add_ ## type (nrm_server_t *self, nrm_msg_ ## type ## _t *msg) \
{ \
	nrm_msg_t *ret = nrm_msg_create(); \
	nrm_ ## type ## _t *r = nrm_ ## type ## _create_frommsg(msg); \
\
	int err = nrm_state_add_ ## type (self->state, r); \
	if (err) { \
		/* TODO: NACK */ \
		nrm_msg_fill(ret, NRM_MSG_TYPE_ACK); \
		return ret; \
	} \
	nrm_msg_fill(ret, NRM_MSG_TYPE_ADD); \
	nrm_msg_set_add_ ## type (ret, r); \
	return ret; \
}

NRM_SERVER_ADD_FUNC(scope)
NRM_SERVER_ADD_FUNC(sensor)
NRM_SERVER_ADD_FUNC(slice)

int nrm_server_add_callback(nrm_server_t *self, nrm_uuid_t *clientid,
			     nrm_msg_add_t *msg)
{
	nrm_msg_t *ret = NULL;
	switch (msg->type) {
	case NRM_MSG_TARGET_TYPE_ACTUATOR:
		nrm_log_info("adding an actuator\n");
		ret = nrm_server_add_actuator(self, clientid, msg->actuator);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SLICE:
		nrm_log_info("adding a slice\n");
		ret = nrm_server_add_slice(self, msg->slice);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		nrm_log_info("adding a sensor\n");
		ret = nrm_server_add_sensor(self, msg->sensor);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SCOPE:
		nrm_log_info("adding a scope\n");
		ret = nrm_server_add_scope(self, msg->scope);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	default:
		nrm_log_error("wrong add request type %u\n", msg->type);
		break;
	}
	if (ret == NULL)
		return -1;
	nrm_role_send(self->role, ret, clientid);
	/* we don't return an error here unless it's a failure of the server
	 * code itself */
	return 0;
}

#define NRM_SERVER_LIST_FUNC(type) \
nrm_msg_t *nrm_server_list_ ## type ## s(nrm_server_t *self) \
{ \
	nrm_msg_t *ret = nrm_msg_create(); \
	nrm_vector_t *vec; \
	nrm_vector_create(&vec, sizeof(nrm_ ## type ## _t)); \
	int err = nrm_state_list_ ## type ## s(self->state, vec); \
	if (err) { \
		/* TODO: NACK */ \
		nrm_msg_fill(ret, NRM_MSG_TYPE_ACK); \
		goto end; \
	} \
	nrm_msg_fill(ret, NRM_MSG_TYPE_LIST); \
	nrm_msg_set_list_ ## type ## s(ret, vec); \
end: \
	nrm_vector_destroy(&vec); \
	return ret; \
}

NRM_SERVER_LIST_FUNC(actuator)
NRM_SERVER_LIST_FUNC(scope)
NRM_SERVER_LIST_FUNC(slice)
NRM_SERVER_LIST_FUNC(sensor)

int nrm_server_list_callback(nrm_server_t *self, nrm_uuid_t *clientid,
			     nrm_msg_list_t *msg)
{
	nrm_msg_t *ret = NULL;
	switch (msg->type) {
	case NRM_MSG_TARGET_TYPE_ACTUATOR:
		nrm_log_info("listing actuators\n");
		ret = nrm_server_list_actuators(self);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SLICE:
		nrm_log_info("listing slices\n");
		ret = nrm_server_list_slices(self);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		nrm_log_info("listing sensors\n");
		ret = nrm_server_list_sensors(self);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SCOPE:
		nrm_log_info("listing scopes\n");
		ret = nrm_server_list_scopes(self);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	default:
		nrm_log_error("wrong list request type %u\n", msg->type);
		break;
	}
	if (ret == NULL)
		return -1;
	nrm_role_send(self->role, ret, clientid);
	/* we don't return an error here unless it's a failure of the server
	 * code itself */
	return 0;
}

#define NRM_SERVER_REMOVE_FUNC(type) \
nrm_msg_t *nrm_server_remove_ ## type (nrm_server_t *self, const char *uuid) \
{ \
	nrm_msg_t *ret = nrm_msg_create(); \
	nrm_state_remove_ ## type (self->state, uuid); \
	/* TODO: NACK */ \
	nrm_msg_fill(ret, NRM_MSG_TYPE_ACK); \
	return ret; \
}

NRM_SERVER_REMOVE_FUNC(actuator)
NRM_SERVER_REMOVE_FUNC(scope)
NRM_SERVER_REMOVE_FUNC(sensor)
NRM_SERVER_REMOVE_FUNC(slice)

int nrm_server_remove_callback(nrm_server_t *self, nrm_uuid_t *clientid,
			     nrm_msg_remove_t *msg)
{
	nrm_msg_t *ret = NULL;
	switch (msg->type) {
	case NRM_MSG_TARGET_TYPE_ACTUATOR:
		nrm_log_info("removing an actuator\n");
		ret = nrm_server_remove_actuator(self, msg->uuid);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SLICE:
		nrm_log_info("removing a slice\n");
		ret = nrm_server_remove_slice(self, msg->uuid);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		nrm_log_info("removing a sensor\n");
		ret = nrm_server_remove_sensor(self, msg->uuid);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SCOPE:
		nrm_log_info("removing a scope\n");
		ret = nrm_server_remove_scope(self, msg->uuid);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	default:
		nrm_log_error("wrong remove request type %u\n", msg->type);
		break;
	}
	if (ret == NULL)
		return -1;
	nrm_role_send(self->role, ret, clientid);
	/* we don't return an error here unless it's a failure of the server
	 * code itself */
	return 0;
}

int nrm_server_exit_callback(nrm_server_t *self, nrm_uuid_t *uuid)
{
	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_ACK);
	nrm_role_send(self->role, ret, uuid);
	/* trigger exit */
	return -1;
}

int nrm_server_role_callback(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	(void)socket;
	nrm_server_t *self = (nrm_server_t *)arg;
	nrm_log_info("event callback: message\n");
	nrm_msg_t *msg;
	nrm_uuid_t *uuid;
	nrm_log_debug("receiving message...\n");
	msg = nrm_role_recv(self->role, &uuid);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);

	int err;
	switch(msg->type) {
	case NRM_MSG_TYPE_ACTUATE:
		err = nrm_server_actuate_callback(self, uuid, msg->actuate);
		break;
	case NRM_MSG_TYPE_LIST:
		err = nrm_server_list_callback(self, uuid, msg->list);
		break;
	case NRM_MSG_TYPE_ADD:
		err = nrm_server_add_callback(self, uuid, msg->add);
		break;
	case NRM_MSG_TYPE_EVENT:
		err = nrm_server_event_callback(self, uuid, msg->event);
		break;
	case NRM_MSG_TYPE_REMOVE:
		err = nrm_server_remove_callback(self, uuid, msg->remove);
		break;
	case NRM_MSG_TYPE_EXIT:
		err = nrm_server_exit_callback(self, uuid);
		break;
	default:
		nrm_log_error("message type not handled\n");
		return -NRM_EINVAL;
	}
	nrm_msg_destroy_received(&msg);
	return err;
}

int nrm_server_signal_callback(zloop_t *loop, zmq_pollitem_t *poller, void
				*arg)
{
	(void)loop;
	nrm_server_t *self = (nrm_server_t *)arg;
	struct signalfd_siginfo fdsi;
	ssize_t s = read(poller->fd, &fdsi, sizeof(struct signalfd_siginfo));
	assert(s == sizeof(struct signalfd_siginfo));
	int signalid = fdsi.ssi_signo;
	nrm_log_debug("caught signal %d\n", signalid);

	/* we default to exit */
	int ret = -1;
	if (self->callbacks.signal != NULL)
		ret = self->callbacks.signal(self, signalid);
	return ret;
}

int nrm_server_timer_callback(zloop_t *loop, int timerid, void *arg)
{
	(void)loop;
	(void)timerid;
	nrm_server_t *self = (nrm_server_t *)arg;

	int ret = 0;
	if (self->callbacks.timer != NULL)
		ret = self->callbacks.timer(self);
	return ret;
}


int nrm_server_create(nrm_server_t **server, nrm_state_t *state,
		const char *uri, int pub_port, int rpc_port)
{
	if (server == NULL || uri == NULL || state == NULL)
		return -NRM_EINVAL;

	nrm_server_t *ret = calloc(1, sizeof(nrm_server_t));
	if (ret == NULL)
		return -NRM_ENOMEM;

	ret->role = nrm_role_controller_create_fromparams(uri, pub_port,
							  rpc_port);
	if (ret->role == NULL)
		return -NRM_EINVAL;
	ret->loop = zloop_new();
	assert(ret->loop != NULL);

	ret->state = state;

	/* we always setup signal handling and controller callback */
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGINT);
	int sfd = signalfd(-1, &sigmask, 0);
	assert(sfd != -1);
	zmq_pollitem_t signal_poller = {0, sfd, ZMQ_POLLIN, 0};
	zloop_poller(ret->loop, &signal_poller, nrm_server_signal_callback, NULL);

	nrm_role_controller_register_recvcallback(ret->role, ret->loop,
						  nrm_server_role_callback,
						  ret);
	*server = ret;
	return 0;
}

/* this function bypass the user validation on actuation */
int nrm_server_actuate(nrm_server_t *self, nrm_string_t uuid, double value)
{
	nrm_actuator_t *a = NULL;
	nrm_hash_find(self->state->actuators, uuid, (void *)&a);
	if (a != NULL) {
		nrm_log_debug("actuating %s: %f\n", a->uuid, value);
		nrm_msg_t *action = nrm_msg_create();
		nrm_msg_fill(action, NRM_MSG_TYPE_ACTUATE);
		nrm_msg_set_actuate(action, a->uuid, value);
		nrm_role_send(self->role, action, a->clientid);
	}
	return 0;
}

int nrm_server_setcallbacks(nrm_server_t *server, nrm_server_user_callbacks_t
			    callbacks)
{
	if (server == NULL)
		return -NRM_EINVAL;

	server->callbacks = callbacks;
	return 0;
}

int nrm_server_settimer(nrm_server_t *server, int millisecs)
{
	if (server == NULL)
		return -NRM_EINVAL;

	zloop_timer(server->loop, millisecs, 0, nrm_server_timer_callback,
		    server);
	return 0;
}

int nrm_server_publish(nrm_server_t *server, nrm_string_t topic,
		       nrm_time_t now, nrm_sensor_t *sensor,
		       nrm_scope_t *scope,
		       double value)
{
	if (server == NULL || topic == NULL || sensor == NULL || scope == NULL)
		return -NRM_EINVAL;

	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_EVENT);
	nrm_msg_set_event(msg, now, sensor->uuid, scope, value);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	return nrm_role_pub(server->role, topic, msg);
}

int nrm_server_start(nrm_server_t *server)
{
	if (server == NULL)
		return -NRM_EINVAL;

	return zloop_start(server->loop);
}

void nrm_server_destroy(nrm_server_t **server)
{
	if (server == NULL || *server == NULL)
		return;
	nrm_server_t *s = *server;
	zloop_destroy(&s->loop);
	nrm_role_destroy(&s->role);
	free(s);
	*server = NULL;
}
