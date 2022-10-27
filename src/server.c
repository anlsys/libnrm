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

#include "internal/nrmi.h"
#include "internal/messages.h"

typedef int(nrm_server_event_callback_fn)(nrm_server_t *, int, int, void *, void *);

/* we split the events between msg events and the rest to ease type conversion
 * logic
 */
#define NRM_SERVER_EVENT_TYPE_ACK 0
#define NRM_SERVER_EVENT_TYPE_LIST 1
#define NRM_SERVER_EVENT_TYPE_ADD 2
#define NRM_SERVER_EVENT_TYPE_REMOVE 3
#define NRM_SERVER_EVENT_TYPE_EVENT 4
#define NRM_SERVER_EVENT_TYPE_ACTUATE 5
#define NRM_SERVER_EVENT_TYPE_MSG_MAX (6)

#define NRM_SERVER_EVENT_TYPE_OTHER_START (100)
#define NRM_SERVER_EVENT_TYPE_TIMER 100
#define NRM_SERVER_EVENT_TYPE_SIGNAL 101
#define NRM_SERVER_EVENT_TYPE_MAX (102)
#define NRM_SERVER_EVENT_TYPE_COUNT (NRM_SERVER_EVENT_TYPE_MSG_MAX + \
				     (NRM_SERVER_EVENT_TYPE_MAX - NRM_SERVER_EVENT_TYPE_OTHER_START))

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
	nrm_server_event_callback_fn *callbacks[NRM_SERVER_EVENT_TYPE_MAX];
	void *user_arg;
};


/* indexed by msgtype */
static const int nrm_server_event_msg_types[NRM_SERVER_EVENT_TYPE_MSG_MAX] = {
	[NRM_MSG_TYPE_ACK]     = -1,
	[NRM_MSG_TYPE_LIST]    =  NRM_SERVER_EVENT_TYPE_LIST,
	[NRM_MSG_TYPE_ADD]     = NRM_SERVER_EVENT_TYPE_ADD,
	[NRM_MSG_TYPE_REMOVE]  =  NRM_SERVER_EVENT_TYPE_REMOVE,
	[NRM_MSG_TYPE_EVENT]   = NRM_SERVER_EVENT_TYPE_EVENT,
	[NRM_MSG_TYPE_ACTUATE] = NRM_SERVER_EVENT_TYPE_ACTUATE,
};

static const int nrm_server_object_msg_types[NRM_SERVER_OBJECT_TYPE_MSG_MAX] = {
	[NRM_MSG_TARGET_TYPE_SLICE] = NRM_SERVER_OBJECT_TYPE_SLICE,
	[NRM_MSG_TARGET_TYPE_SENSOR] = NRM_SERVER_OBJECT_TYPE_SENSOR,
	[NRM_MSG_TARGET_TYPE_SCOPE] = NRM_SERVER_OBJECT_TYPE_SCOPE,
	[NRM_MSG_TARGET_TYPE_ACTUATOR] = NRM_SERVER_OBJECT_TYPE_ACTUATOR,
};

int nrm_server_msg2event(int type)
{
	int ret;
	if (type < 0 || type >= NRM_SERVER_EVENT_TYPE_MSG_MAX)
		return -NRM_EINVAL;
	ret = nrm_server_event_msg_types[type];
	if (ret == -1)
		return -NRM_EINVAL;
	/* the values of the two types are designed to match */
	assert(ret == type);
	return ret;
}

typedef int(*nrm_server_msg_converter_fn)(nrm_msg_t *, void **, int *);

int nrm_server_msg_converter_list(nrm_msg_t *msg, void **arg, int *type)
{
	assert(msg->type == NRM_MSG_TYPE_LIST);
	assert(msg->list->type < NRM_MSG_TARGET_TYPE_MAX);
	*(intptr_t*)arg = nrm_server_object_msg_types[msg->list->type];
	*type = NRM_SERVER_OBJECT_TYPE_INT;
	return 0;
}

int nrm_server_msg_converter_add(nrm_msg_t *msg, void **arg, int *type)
{
	assert(msg->type == NRM_MSG_TYPE_ADD);
	assert(msg->add->type < NRM_MSG_TARGET_TYPE_MAX);
	nrm_actuator_t *a;
	nrm_slice_t *sl;
	switch(msg->add->type) {
	case NRM_MSG_TARGET_TYPE_ACTUATOR:
	a = nrm_actuator_create_frommsg(msg->add->actuator);
	a->clientid = nrm_uuid_create_fromchar(nrm_uuid_to_char(clientid));
	*type = NRM_SERVER_OBJECT_TYPE_ACTUATOR;
	*arg = a;
	break;
	case NRM_MSG_TARGET_TYPE_SLICE:
	sl = nrm_slice_create(msg->add->slice->uuid);
	*type = NRM_SERVER_OBJECT_TYPE_SLICE;
	*arg = sl;
	}
	return 0;
}

static const nrm_server_msg_converter_fn nrm_server_msg_converters[] = {
	[NRM_SERVER_EVENT_TYPE_LIST] = nrm_server_msg_converter_list,
	[NRM_SERVER_EVENT_TYPE_ADD] = nrm_server_msg_converter_add,
};

int nrm_server_role_callback(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	(void)socket;
	nrm_server_t *self = (nrm_server_t *)arg;
	nrm_log_info("event callback: message\n");
	nrm_msg_t *msg, *ret;
	nrm_uuid_t *uuid;
	nrm_log_debug("receiving message...\n");
	msg = nrm_role_recv(self->role, &uuid);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);

	int event_type = nrm_server_msg2event(msg->type);
	if (self->callbacks[event_type] == NULL) {
		nrm_log_debug("missing callback for %d\n");
		return 0;
	}

	void *arg1;
	int object_type;
	int err = nrm_server_msg2params(msg, &arg1, &object_type);
	if (!err) {
		nrm_log_error("failed to convert message to callback params\n");
		return -1;
	}

	nrm_server_event_callback_fn cb = self->callbacks[event_type];
	err = cb(self, event_type, object_type, arg1, self->user_arg);
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
	nrm_server_event_callback_fn cb = self->callbacks[NRM_SERVER_EVENT_SIGNAL];
	if (cb != NULL)
		ret = cb(self, NRM_SERVER_EVENT_TYPE_SIGNAL,
		   NRM_SERVER_OBJECT_TYPE_INT, &signalid, self->user_arg);
	return ret;
}

int nrm_server_timer_callback(zloop_t *loop, int timerid, void *arg)
{
	(void)loop;
	(void)timerid;
	nrm_server_t *self = (nrm_server_t *)arg;

	int ret = 0;
	nrm_server_event_callback_fn cb =
		self->callbacks[NRM_SERVER_EVENT_TYPE_TIMER];
	if (cb != NULL)
		ret = cb(self, NRM_SERVER_EVENT_TYPE_TIMER,
			 NRM_SERVER_OBJECT_NONE, NULL, self->user_arg);
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
	sfd = signalfd(-1, &sigmask, 0);
	assert(sfd != -1);
	zmq_pollitem_t signal_poller = {0, sfd, ZMQ_POLLIN};
	zloop_poller(ret->loop, &signal_poller, nrm_server_signal_callback, NULL);

	nrm_role_controller_register_recvcallback(ret->role, ret->loop,
						  nrm_server_role_callback,
						  ret);
	return ret;
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
		       nrm_sensor_t *sensor,
		       nrm_scope_t *scope,
		       double value)
{
	if (server == NULL || topic == NULL || sensor == NULL || scope == NULL)
		return -NRM_EINVAL;

	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_EVENT);
	nrm_msg_set_event(msg, now, sensor->uuid, scope, 1.0);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	return nrm_role_pub(server->role, topic, msg);
}

int nrm_server_start(nrm_server_t *server)
{
	if (server == NULL)
		return -NRM_EINVAL;

	return zloop_start(server->start);
}

void nrm_server_destroy(nrm_server_t **server)
{
	if (server == NULL || *server == NULL)
		return;
	nrm_server_t *s = *server;
	zloop_destroy(&server->loop);
	nrm_role_destroy(&server->role);
	free(s);
	*server = NULL;
}
