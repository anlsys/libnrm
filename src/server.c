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

struct nrm_server_user_callbacks_s {
	int(*list)(nrm_server_t *, nrm_uuid_t *, int, void *);
	int(*add)(nrm_server_t *, nrm_uuid_t *, int, void *, void *);
	int(*remove)(nrm_server_t *, nrm_uuid_t *, int, nrm_string_t, void *);
	int(*event)(nrm_server_t *, nrm_uuid_t *, nrm_string_t, nrm_scope_t *,
		    nrm_time_t, double value, void *);
	int(*actuate)(nrm_server_t *, nrm_uuid_t *, nrm_string_t, double value,
		      void *);
	int(*signal)(nrm_server_t *, int, void *);
	int(*timer)(nrm_server_t *, void *);
};

typedef struct nrm_server_user_callbacks_s nrm_server_user_callbacks_t;


#define NRM_SERVER_OBJECT_TYPE_SLICE 0
#define NRM_SERVER_OBJECT_TYPE_SENSOR 1
#define NRM_SERVER_OBJECT_TYPE_SCOPE 2
#define NRM_SERVER_OBJECT_TYPE_ACTUATOR 3
#define NRM_SERVER_OBJECT_TYPE_MSG_MAX (4)

#define NRM_SERVER_OBJECT_TYPE_OTHER_START (100)
#define NRM_SERVER_OBJECT_TYPE_INT 100
#define NRM_SERVER_OBJECT_TYPE_NONE 101
#define NRM_SERVER_OBJECT_TYPE_STRING 102
#define NRM_SERVER_OBJECT_TYPE_MAX (103)
#define NRM_SERVER_OBJECT_TYPE_COUNT (NRM_SERVER_OBJECT_TYPE_MSG_MAX + \
				     (NRM_SERVER_OBJECT_TYPE_MAX - NRM_SERVER_OBJECT_TYPE_OTHER_START))

struct nrm_server_s {
	nrm_role_t *role;
	zloop_t *loop;
	nrm_server_user_callback_t callbacks;
	void *user_arg;
};


static const int nrm_server_object_msg_types[NRM_SERVER_OBJECT_TYPE_MSG_MAX] = {
	[NRM_MSG_TARGET_TYPE_SLICE] = NRM_SERVER_OBJECT_TYPE_SLICE,
	[NRM_MSG_TARGET_TYPE_SENSOR] = NRM_SERVER_OBJECT_TYPE_SENSOR,
	[NRM_MSG_TARGET_TYPE_SCOPE] = NRM_SERVER_OBJECT_TYPE_SCOPE,
	[NRM_MSG_TARGET_TYPE_ACTUATOR] = NRM_SERVER_OBJECT_TYPE_ACTUATOR,
};

int nrm_server_msg_converter_list(nrm_uuid_t *uuid, nrm_msg_t *msg, void **arg, int *type)
{
	(void)uuid;
	assert(msg->type == NRM_MSG_TYPE_LIST);
	assert(msg->list->type < NRM_MSG_TARGET_TYPE_MAX);
	*(intptr_t*)arg = nrm_server_object_msg_types[msg->list->type];
	*type = NRM_SERVER_OBJECT_TYPE_INT;
	return 0;
}

int nrm_server_msg_converter_add(nrm_uuid_t *uuid, nrm_msg_t *msg, void **arg, int *type)
{
	assert(msg->type == NRM_MSG_TYPE_ADD);
	assert(msg->add->type < NRM_MSG_TARGET_TYPE_MAX);
	nrm_actuator_t *a;
	nrm_slice_t *sl;
	nrm_sensor_t *se;
	nrm_scope_t *sc;
	switch(msg->add->type) {
	case NRM_MSG_TARGET_TYPE_ACTUATOR:
		a = nrm_actuator_create_frommsg(msg->add->actuator);
		a->clientid = nrm_uuid_create_fromchar(nrm_uuid_to_char(uuid));
		*type = NRM_SERVER_OBJECT_TYPE_ACTUATOR;
		*arg = a;
		break;
	case NRM_MSG_TARGET_TYPE_SLICE:
		sl = nrm_slice_create_frommsg(msg->add->slice);
		*type = NRM_SERVER_OBJECT_TYPE_SLICE;
		*arg = sl;
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		se = nrm_sensor_create_frommsg(msg->add->sensor);
		*type = NRM_SERVER_OBJECT_TYPE_SENSOR;
		*arg = se;
		break;
	case NRM_MSG_TARGET_TYPE_SCOPE:
		sc = nrm_scope_create_frommsg(msg->add->scope);
		*type = NRM_SERVER_OBJECT_TYPE_SCOPE;
		*arg = sc;
		break;
	default:
		nrm_log_error("wrong message type: %d\n", msg->add->type);
		assert(0);
	}
	return 0;
}

int nrm_server_msg_converter_remove(nrm_uuid_t *uuid, nrm_msg_t *msg, void **arg, int *type)
{
	assert(msg->type == NRM_MSG_TYPE_REMOVE);
	assert(msg->remove->type < NRM_MSG_TARGET_TYPE_MAX);
	nrm_string_t uuid;

	return 0;
}

int nrm_server_msg_converter_event(nrm_uuid_t *uuid, nrm_msg_t *msg, void **arg, int *type)
{
	return 0;
}

int nrm_server_msg_converter_actuate(nrm_uuid_t *uuid, nrm_msg_t *msg, void **arg, int *type)
{
	return 0;
}


int nrm_server_list_callback(nrm_server_t *self, nrm_uuid_t *clientid,
			     nrm_msg_list_t *msg)
{
	(void)uuid;
	assert(msg->type == NRM_MSG_TYPE_LIST);
	int type = nrm_server_msg_types[msg->type];
	int ret = self->callbacks->list(self, clientid, type,
					self->user_arg);
	if (ret == 0) {
		/* TODO: callback should fill out a vector of results */
	} else {
		/* TODO: nack message */	
	}
	/* we don't return an error here unless it's a failure of the server
	 * code itself */
	return 0;
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
	default:
		nrm_log_error("message type not handled\n");
		return -NRM_EINVAL;
	}
	nrm_msg_destroy(&msg);
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
	if (self->callbacks->signal != NULL)
		ret = self->callbacks->signal(self, signalid, self->user_arg);
	return ret;
}

int nrm_server_timer_callback(zloop_t *loop, int timerid, void *arg)
{
	(void)loop;
	(void)timerid;
	nrm_server_t *self = (nrm_server_t *)arg;

	int ret = 0;
	if (self->callbacks->timer != NULL)
		ret = self->callbacks->timer(self, self->user_arg);
	return ret;
}


int nrm_server_create(nrm_server_t **server, const char *uri, int pub_port, int
		      rpc_port)
{
	if (server == NULL || uri == NULL)
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
