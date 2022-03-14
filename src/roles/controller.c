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
struct nrm_role_controller_broker_s {
	/* communication channel with the creator of the broker */
	zsock_t *pipe;
	/* socket used to receive client rpc */
	zsock_t *rpc;
	/* socket used to publish controller events */
	zsock_t *pub;
	/* controllering loop */
	zloop_t *loop;
	/* transmit ok */
	int transmit;
};

struct nrm_role_controller_broker_args {
	const char *pub_uri;
	const char *rpc_uri;
	/* transmit: actually open sockets */
	int transmit;
};

struct nrm_role_controller_s {
	/* the broker itself */
	zactor_t *broker;
};

int nrm_controller_rpc_init(struct nrm_role_controller_broker_s *self, const char *uri)
{
	int err;
	if (!self->transmit)
		return 0;

	/* standard setup. Might be a bid careless to remove the high water mark
	 * on the socket, but until it causes us trouble we should keep it.
	 */
	self->rpc =  zsock_new(ZMQ_ROUTER);
	assert(self->rpc != NULL);
	zsock_set_immediate(self->rpc, 1);
	zsock_set_unbounded(self->rpc);

	/* now accept connections */
	err = zsock_bind(self->rpc, "%s", uri);
	assert(err == 0);
	return 0;
}

int nrm_controller_pub_init(struct nrm_role_controller_broker_s *self, const char *uri)
{
	int err;
	if (!self->transmit)
		return 0;

	self->pub =  zsock_new(ZMQ_PUB);
	assert(self->pub != NULL);
	zsock_set_linger(self->pub, 0);
	zsock_set_sndhwm(self->pub, 0);

	/* now accept connections */
	err = zsock_bind(self->pub, "%s", uri);
	assert(err == 0);
	return 0;
}

int nrm_controller_broker_rpc_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_role_controller_broker_s *self = 
		(struct nrm_role_controller_broker_s *)arg;
	(void)self;
	(void)socket;
	return 0;
}

int nrm_controller_broker_pipe_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_role_controller_broker_s *self =
		(struct nrm_role_controller_broker_s *)arg;
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

void nrm_controller_broker_fn(zsock_t *pipe, void *args)
{
	int err;
	struct nrm_role_controller_broker_s *self;
	struct nrm_role_controller_broker_args *params;

	assert(pipe);
	assert(args);

	/* avoid losing messages */
	zsock_set_unbounded(pipe);
	self = calloc(1, sizeof(struct nrm_role_controller_broker_s));
	if (self == NULL)
	{
		zsock_signal(pipe,1);
		return;
	}

	self->pipe = pipe;
	params = (struct nrm_role_controller_broker_args *)args;

	/* init network */
	self->transmit = params->transmit;
	err = nrm_controller_rpc_init(self, params->rpc_uri);
	assert(!err);
	err = nrm_controller_pub_init(self, params->pub_uri);
	assert(!err);

	/* set ourselves up to handle messages */
	self->loop = zloop_new();
	assert(self->loop != NULL);

	zloop_reader(self->loop, self->pipe,
		     (zloop_reader_fn*)nrm_controller_broker_pipe_handler,
		     (void*)self);
	zloop_reader(self->loop, self->rpc,
		     (zloop_reader_fn*)nrm_controller_broker_rpc_handler,
		     (void*)self);

	/* notify we are ready */
	zsock_signal(self->pipe, 0);

	/* start: will only return when broker is destroyed */
	zloop_start(self->loop);

	zloop_destroy(&self->loop);
	zsock_destroy(&self->rpc);
	zsock_destroy(&self->pub);
	free(self);
}

nrm_role_t *nrm_role_controller_create_fromparams(const char *pub_uri, const
						  char *rpc_uri, int transmit)
{
	nrm_role_t *role;
	struct nrm_role_controller_s *data;

	role = NRM_INNER_MALLOC(nrm_role_t, struct nrm_role_controller_s);
	if (role == NULL)
		return NULL;

	data = NRM_INNER_MALLOC_GET_FIELD(role, 2, nrm_role_t, struct
					  nrm_role_controller_s);
	role->data = (struct nrm_role_data *)data;
	role->ops = &nrm_role_controller_ops;

	/* env init */
	struct nrm_role_controller_broker_args bargs;
	bargs.pub_uri = pub_uri;
	bargs.rpc_uri = rpc_uri;
	bargs.transmit = transmit;

	/* create broker */
	data->broker = zactor_new(nrm_controller_broker_fn, &bargs);
	if (data->broker == NULL)
		return NULL;
	zsock_set_unbounded(data->broker);
	return role;
}

void nrm_role_controller_destroy(nrm_role_t **role)
{
	if(role == NULL && *role == NULL)
		return;

	struct nrm_role_controller_s *controller = (struct nrm_role_controller_s *)
		(*role)->data;

	/* simply destroy the actor, in principle this should just send a
	 * message on the pipe and wait for the actor to exit by itself */
	zactor_destroy(&controller->broker);
	free(*role);
	*role = NULL;
}

int nrm_role_controller_send(const struct nrm_role_data *data,
			 nrm_msg_t *msg)
{
	struct nrm_role_controller_s *controller = (struct nrm_role_controller_s *)data;
	nrm_ctrlmsg_send((zsock_t *)controller->broker, NRM_CTRLMSG_TYPE_SEND, msg);
	return 0;
}

nrm_msg_t *nrm_role_controller_recv(const struct nrm_role_data *data)
{
	struct nrm_role_controller_s *controller = (struct nrm_role_controller_s *)data;
	nrm_msg_t *msg;
	int msgtype;
	msg = nrm_ctrlmsg_recv((zsock_t *)controller->broker, &msgtype);
	assert(msgtype == NRM_CTRLMSG_TYPE_RECV);
	return msg;
}

int nrm_role_controller_register_recvcallback(nrm_role_t *role, zloop_t *loop,
					   zloop_reader_fn *fn,
					   void *arg)
{
	struct nrm_role_controller_s *controller = (struct nrm_role_controller_s
					      *)role->data;
	zloop_reader(loop, (zsock_t *)controller->broker, fn, arg);
	return 0;
}

int nrm_role_controller_pub(const struct nrm_role_data *data, nrm_msg_t *msg)
{
	struct nrm_role_controller_s *controller = (struct nrm_role_controller_s *)data;
	nrm_ctrlmsg_send((zsock_t *)controller->broker, NRM_CTRLMSG_TYPE_PUB, msg);
	return 0;
}

struct nrm_role_ops nrm_role_controller_ops = {
	nrm_role_controller_send,
	nrm_role_controller_recv,
	nrm_role_controller_pub,
	NULL,
	nrm_role_controller_destroy,
};
