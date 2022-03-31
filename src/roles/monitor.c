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
#include "internal/roles.h"

/* actor thread that takes care of actually communicating with the rest of the
 * NRM infrastructure.
 */
struct nrm_role_monitor_broker_s {
	/* communication channel with the creator of the broker */
	zsock_t *pipe;
	/* socket used to receive sensor messages */
	zsock_t *in;
	/* monitoring loop */
	zloop_t *loop;
};

struct nrm_role_monitor_broker_args {
	/* uri for in connection */
	const char *uri;
};

struct nrm_role_monitor_s {
	/* the broker itself */
	zactor_t *broker;
};


int nrm_monitor_broker_in_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_role_monitor_broker_s *self = 
		(struct nrm_role_monitor_broker_s *)arg;
	(void)self;
	(void)socket;
	return 0;
}

int nrm_monitor_broker_pipe_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_role_monitor_broker_s *self =
		(struct nrm_role_monitor_broker_s *)arg;
	(void)self;

	/* the only message we should ever receive here in the term one.
	 */
	int msg_type;
	nrm_msg_t *msg = nrm_ctrlmsg_recv(socket, &msg_type, NULL);
	assert(msg_type == NRM_CTRLMSG_TYPE_TERM);
	assert(msg == NULL);
	/* returning -1 exits the loop */
	return -1;
}

void nrm_monitor_broker_fn(zsock_t *pipe, void *args)
{
	int err;
	struct nrm_role_monitor_broker_s *self;
	struct nrm_role_monitor_broker_args *params;

	assert(pipe);
	assert(args);

	/* avoid losing messages */
	zsock_set_unbounded(pipe);
	self = calloc(1, sizeof(struct nrm_role_monitor_broker_s));
	if (self == NULL)
	{
		zsock_signal(pipe,1);
		return;
	}

	self->pipe = pipe;
	params = (struct nrm_role_monitor_broker_args *)args;

	/* init network */
	err = nrm_net_rpc_server_init(&self->in);
	assert(!err);
	err = nrm_net_bind(self->in, params->uri);
	assert(!err);

	/* set ourselves up to handle messages */
	self->loop = zloop_new();
	assert(self->loop != NULL);

	zloop_reader(self->loop, self->pipe,
		     (zloop_reader_fn*)nrm_monitor_broker_pipe_handler,
		     (void*)self);
	zloop_reader(self->loop, self->in,
		     (zloop_reader_fn*)nrm_monitor_broker_in_handler,
		     (void*)self);

	/* notify we are ready */
	zsock_signal(self->pipe, 0);

	/* start: will only return when broker is destroyed */
	zloop_start(self->loop);

	zloop_destroy(&self->loop);
	zsock_destroy(&self->in);
	free(self);
}

nrm_role_t *nrm_role_monitor_create_fromenv()
{
	nrm_role_t *role;
	struct nrm_role_monitor_s *data;

	role = NRM_INNER_MALLOC(nrm_role_t, struct nrm_role_monitor_s);
	if (role == NULL)
		return NULL;

	data = NRM_INNER_MALLOC_GET_FIELD(role, 2, nrm_role_t, struct
					  nrm_role_monitor_s);
	role->data = (struct nrm_role_data *)data;
	role->ops = &nrm_role_monitor_ops;

	/* env init */
	struct nrm_role_monitor_broker_args bargs;
	bargs.uri = getenv(NRM_ENV_DOWNSTREAM_URI);
	if (bargs.uri == NULL)
		bargs.uri = NRM_DEFAULT_DOWNSTREAM_URI;

	/* create broker */
	data->broker = zactor_new(nrm_monitor_broker_fn, &bargs);
	if (data->broker == NULL)
		return NULL;
	zsock_set_unbounded(data->broker);
	return role;
}

void nrm_role_monitor_destroy(nrm_role_t **role)
{
	if(role == NULL && *role == NULL)
		return;

	struct nrm_role_monitor_s *monitor = (struct nrm_role_monitor_s *)
		(*role)->data;

	/* simply destroy the actor, in principle this should just send a
	 * message on the pipe and wait for the actor to exit by itself */
	zactor_destroy(&monitor->broker);
	free(*role);
	*role = NULL;
}

nrm_msg_t *nrm_role_monitor_recv(const struct nrm_role_data *data)
{
	struct nrm_role_monitor_s *monitor = (struct nrm_role_monitor_s *)data;
	nrm_msg_t *msg;
	int msgtype;
	msg = nrm_ctrlmsg_recv((zsock_t *)monitor->broker, &msgtype, NULL);
	assert(msgtype == NRM_CTRLMSG_TYPE_RECV);
	return msg;
}

int nrm_role_monitor_register_recvcallback(nrm_role_t *role, zloop_t *loop,
					   zloop_reader_fn *fn,
					   void *arg)
{
	struct nrm_role_monitor_s *monitor = (struct nrm_role_monitor_s
					      *)role->data;
	zloop_reader(loop, (zsock_t *)monitor->broker, fn, arg);
	return 0;
}

struct nrm_role_ops nrm_role_monitor_ops = {
	NULL,
	nrm_role_monitor_recv,
	NULL,
	NULL,
	nrm_role_monitor_destroy,
};
