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
};

struct nrm_role_controller_broker_args {
	const char *uri;
	int pub_port;
	int rpc_port;
};

struct nrm_role_controller_s {
	/* the broker itself */
	zactor_t *broker;
};

int nrm_controller_broker_rpc_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_role_controller_broker_s *self = 
		(struct nrm_role_controller_broker_s *)arg;
	nrm_log_debug("controller rpc recv\n");
	nrm_uuid_t *uuid;
	nrm_msg_t *msg = nrm_msg_recvfrom(socket, &uuid);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_ctrlmsg_send(self->pipe, NRM_CTRLMSG_TYPE_RECV, msg, uuid);
	return 0;
}

int nrm_controller_broker_pipe_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_role_controller_broker_s *self =
		(struct nrm_role_controller_broker_s *)arg;

	nrm_log_debug("controller pipe recv\n");
	int msg_type;
	nrm_uuid_t *uuid;
	nrm_msg_t *msg = nrm_ctrlmsg_recv(socket, &msg_type, &uuid);
	switch(msg_type) {
		case NRM_CTRLMSG_TYPE_TERM:
			nrm_log_info("received term request\n");
			/* returning -1 exits the loop */
			return -1;
		case NRM_CTRLMSG_TYPE_SEND:
			nrm_log_info("received request to send to client\n");
			nrm_msg_sendto(self->rpc, msg, uuid);
			break;
		default:
			nrm_log_error("msg type %u not handled\n", msg_type);
			break;
	}
	return 0;
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
	err = nrm_net_rpc_server_init(&self->rpc);
	assert(!err);
	err = nrm_net_bind_2(self->rpc, params->uri, params->rpc_port);
	assert(!err);

	err = nrm_net_pub_init(&self->pub);
	assert(!err);
	err = nrm_net_bind_2(self->pub, params->uri, params->pub_port);
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

nrm_role_t *nrm_role_controller_create_fromparams(const char *uri, int pub_port,
						  int rpc_port)
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
	bargs.uri = uri;
	bargs.pub_port = pub_port;
	bargs.rpc_port = rpc_port;

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
			 nrm_msg_t *msg, nrm_uuid_t *to)
{
	struct nrm_role_controller_s *controller = (struct nrm_role_controller_s *)data;
	nrm_ctrlmsg_send((zsock_t *)controller->broker, NRM_CTRLMSG_TYPE_SEND, msg, to);
	return 0;
}

nrm_msg_t *nrm_role_controller_recv(const struct nrm_role_data *data, nrm_uuid_t
				    **from)
{
	struct nrm_role_controller_s *controller = (struct nrm_role_controller_s *)data;
	nrm_msg_t *msg;
	int msgtype;
	msg = nrm_ctrlmsg_recv((zsock_t *)controller->broker, &msgtype, from);
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
	nrm_ctrlmsg_send((zsock_t *)controller->broker, NRM_CTRLMSG_TYPE_PUB, msg, 
			 NULL);
	return 0;
}

struct nrm_role_ops nrm_role_controller_ops = {
	nrm_role_controller_send,
	nrm_role_controller_recv,
	nrm_role_controller_pub,
	NULL,
	nrm_role_controller_destroy,
};
