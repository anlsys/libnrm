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

#include "internal/nrmi.h"
#include "internal/messages.h"

#include <czmq.h>

/* actor thread that takes care of actually communicating with the rest of the
 * NRM infrastructure.
 */
struct nrm_broker_monitor_s {
	/* communication channel with the creator of the broker */
	zsock_t *pipe;
	/* socket used to receive sensor messages */
	zsock_t *in;
	/* monitoring loop */
	zloop_t *loop;
	/* transmit ok */
	int transmit;
};

struct nrm_broker_monitor_args {
	/* uri for in connection */
	const char *uri;
	/* transmit: actually open sockets */
	int transmit;
};

struct nrm_role_monitor_s {
	/* the broker itself */
	zactor_t *broker;
};

int nrm_monitor_in_init(struct nrm_broker_monitor_s *self, const char *uri)
{
	int err;
	if (!self->transmit)
		return 0;

	/* standard setup. Might be a bid careless to remove the high water mark
	 * on the socket, but until it causes us trouble we should keep it.
	 */
	self->in =  zsock_new(ZMQ_ROUTER);
	assert(self->in != NULL);
	zsock_set_immediate(self->in, 1);
	zsock_set_unbounded(self->in);

	/* now accept connections */
	err = zsock_bind(self->in, "%s", uri);
	assert(err == 0);
	return 0;
}

int nrm_broker_monitor_in_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_broker_monitor_s *self = (struct nrm_broker_monitor_s *)arg;
	(void)self;
	(void)socket;
	return 0;
}

int nrm_broker_monitor_pipe_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_broker_monitor_s *self = (struct nrm_broker_monitor_s *)arg;
	(void)self;

	/* the only message we should ever receive here in the term one.
	 */
	int msg_type;
	nrm_msg_t *msg = nrm_ctrlmsg_recv(socket, &msg_type);
	assert(msg_type == NRM_CTRLMSG_TYPE_TERM);
	assert(msg == NULL);
	/* returning -1 exits the loop */
	return -1;
}

void nrm_broker_monitor_fn(zsock_t *pipe, void *args)
{
	int err;
	struct nrm_broker_monitor_s *self;
	struct nrm_broker_monitor_args *params;

	assert(pipe);
	assert(args);

	/* avoid losing messages */
	zsock_set_unbounded(pipe);
	self = calloc(1, sizeof(struct nrm_broker_monitor_s));
	if (self == NULL)
	{
		zsock_signal(pipe,1);
		return;
	}

	self->pipe = pipe;
	params = (struct nrm_broker_monitor_args *)args;

	/* init network */
	self->transmit = params->transmit;
	err = nrm_monitor_in_init(self, params->uri);
	assert(!err);

	/* set ourselves up to handle messages */
	self->loop = zloop_new();
	assert(self->loop != NULL);

	zloop_reader(self->loop, self->pipe,
		     (zloop_reader_fn*)nrm_broker_monitor_pipe_handler,
		     (void*)self);
	zloop_reader(self->loop, self->in,
		     (zloop_reader_fn*)nrm_broker_monitor_in_handler,
		     (void*)self);

	/* notify we are ready */
	zsock_signal(self->pipe, 0);

	/* start: will only return when broker is destroyed */
	zloop_start(self->loop);

	zloop_destroy(&self->loop);
	zsock_destroy(&self->in);
	free(self);
}

struct nrm_role_monitor_s *nrm_role_monitor_create_fromenv()
{
	struct nrm_role_monitor_s *role;
	role = calloc(1, sizeof(struct nrm_role_monitor_s));
	if (role == NULL)
		return NULL;

	/* env init */
	struct nrm_broker_monitor_args bargs;
	bargs.uri = getenv(NRM_ENV_DOWNSTREAM_URI);
	if (bargs.uri == NULL)
		bargs.uri = NRM_DEFAULT_DOWNSTREAM_URI;
	const char *transmit = getenv(NRM_ENV_TRANSMIT);
	if (transmit != NULL)
		bargs.transmit = atoi(transmit);
	else
		bargs.transmit = 1;

	/* create broker */
	role->broker = zactor_new(nrm_broker_monitor_fn, &bargs);
	if (role->broker == NULL)
		return NULL;
	zsock_set_unbounded(role->broker);
	return role;
}

int nrm_role_monitor_destroy(struct nrm_role_monitor_s *role)
{
	/* simply destroy the actor, in principle this should just send a
	 * message on the pipe and wait for the actor to exit by itself */
	zactor_destroy(&role->broker);

	free(role);
	return 0;
}

nrm_msg_t *nrm_role_monitor_recv(struct nrm_role_monitor_s *role)
{
	nrm_msg_t *msg;
	int msgtype;
	msg = nrm_ctrlmsg_recv((zsock_t *)role->broker, &msgtype);
	assert(msgtype == NRM_CTRLMSG_TYPE_RECV);
	return msg;
}

zsock_t *nrm_role_monitor_broker(struct nrm_role_monitor_s *role)
{
	return (zsock_t *)role->broker;
}
