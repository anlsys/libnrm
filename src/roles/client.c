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
#include "internal/roles.h"

/* actor thread that takes care of actually communicating with the rest of the
 * NRM infrastructure.
 */
struct nrm_client_broker_s {
	/* communication channel with the creator of the broker */
	zsock_t *pipe;
	/* socket used to send out client requests */
	zsock_t *rpc;
	/* socket used to listen to daemon events */
	zsock_t *sub;
	/* monitoring loop */
	zloop_t *loop;
	/* transmit ok */
	int transmit;
};

struct nrm_client_broker_args {
	/* uri for out connection */
	const char *uri;
	/* port to use for client */
	int rpc_port;
	/* port to use for client */
	int sub_port;
	/* whether or not to transmit messages */
	int transmit;
};

struct nrm_role_client_s {
	zactor_t *broker;
};

int nrm_client_rpc_init(struct nrm_client_broker_s *self, const char *uri, int
			port)
{
	int err;
	int connected;

	if (!self->transmit)
		return 0;
	
	/* the process is a little more complicated that you would think, as
	 * we're trying to avoid returning from this function before the socket
	 * is fully connected.
	 *
	 * To do that, we also create a socket monitor (socket pair + actor)
	 * that will loop until we actually receive a connection message.
	 *
	 * To avoid missing that message on the monitor, we're also careful of
	 * connecting the monitor before connecting the real socket to the
	 * server.
	 */
	self->rpc = zsock_new(ZMQ_DEALER);
	assert(self->rpc != NULL);
	zsock_set_immediate(self->rpc, 1);
	zsock_set_unbounded(self->rpc);

	zactor_t *monitor = zactor_new(zmonitor, self->rpc);
	assert(monitor != NULL);

	zstr_sendx(monitor, "LISTEN", "CONNECTED", NULL);
	zstr_send(monitor, "START");
	zsock_wait(monitor);

	/* now connect the original client */
	err = zsock_connect(self->rpc, "%s:%d", uri, port);
	assert(err == 0);

	connected = 0;
	while(!connected) {
		/* read first frame for event number */
		zmsg_t *msg = zmsg_recv(monitor);
		assert(msg != NULL);

		char *event = zmsg_popstr(msg);

		if(!strcmp(event, "CONNECTED"))
			connected = 1;
		free(event);
		zmsg_destroy(&msg);
	}
	/* cleanup monitor */
	zactor_destroy(&monitor);
	return 0;
}

int nrm_client_sub_init(struct nrm_client_broker_s *self, const char *uri, int
			port)
{
	int err, connected;
	if (!self->transmit)
		return 0;
	
	self->sub = zsock_new(ZMQ_SUB);
	assert(self->sub != NULL);
	zsock_set_rcvhwm(self->sub, 0);

	zactor_t *monitor = zactor_new(zmonitor, self->sub);
	assert(monitor != NULL);

	zstr_sendx(monitor, "LISTEN", "CONNECTED", NULL);
	zstr_send(monitor, "START");
	zsock_wait(monitor);

	/* now connect the original client */
	err = zsock_connect(self->rpc, "%s:%d", uri, port);
	assert(err == 0);

	connected = 0;
	while(!connected) {
		/* read first frame for event number */
		zmsg_t *msg = zmsg_recv(monitor);
		assert(msg != NULL);

		char *event = zmsg_popstr(msg);

		if(!strcmp(event, "CONNECTED"))
			connected = 1;
		free(event);
		zmsg_destroy(&msg);
	}
	/* cleanup monitor */
	zactor_destroy(&monitor);
	return 0;
}

int nrm_client_broker_pipe_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_client_broker_s *self = (struct nrm_client_broker_s *)arg;

	/* pipe messages are in shared memory, not actually packed on
	 * the network */
	int msg_type;
	nrm_msg_t *msg = nrm_ctrlmsg_recv(socket, &msg_type);
	if (msg_type == NRM_CTRLMSG_TYPE_SEND
	    && self->transmit) {
	}
	else if(msg_type == NRM_CTRLMSG_TYPE_TERM) {
		/* returning -1 exits the loop */
		return -1;
	}
	/* we've taken ownership of the message at this point */
	nrm_msg_destroy(&msg);
	return 0;
}

int nrm_client_broker_rpc_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_client_broker_s *self = (struct nrm_client_broker_s *)arg;

	/* this should be a rpc answer, that we need to transmit to the pipe */
	int msg_type;
	nrm_msg_t *msg = nrm_msg_recv(socket, &msg_type);
	nrm_ctrlmsg_send(self->pipe, NRM_CTRLMSG_TYPE_SEND, msg);	
	return 0;
}

int nrm_client_broker_sub_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_client_broker_s *self = (struct nrm_client_broker_s *)arg;
	
	/* this should be a sub message, that we need to transmit to the pipe */
	int msg_type;
	nrm_msg_t *msg = nrm_msg_recv(socket, &msg_type);
	nrm_ctrlmsg_send(self->pipe, NRM_CTRLMSG_TYPE_SUB, msg);	
	return 0;
}

void nrm_client_broker_fn(zsock_t *pipe, void *args)
{
	int err;
	struct nrm_client_broker_s *self;
	struct nrm_client_broker_args *params;

	assert(pipe);
	assert(args);

	/* avoid losing messages */
	zsock_set_unbounded(pipe);
	self = calloc(1, sizeof(struct nrm_client_broker_s));
	if (self == NULL)
	{
		zsock_signal(pipe,1);
		return;
	}

	self->pipe = pipe;
	params = (struct nrm_client_broker_args *)args;

	fprintf(stderr, "%s:%d: %s:%d:%d\n",__FILE__,__LINE__,params->uri,
		params->rpc_port, params->sub_port);

	/* init network */
	self->transmit = params->transmit;
	err = nrm_client_rpc_init(self, params->uri, params->rpc_port);
	assert(!err);
	err = nrm_client_sub_init(self, params->uri, params->sub_port);
	assert(!err);

	/* set ourselves up to handle messages */
	self->loop = zloop_new();
	assert(self->loop != NULL);

	zloop_reader(self->loop, self->pipe,
		     (zloop_reader_fn*)nrm_client_broker_pipe_handler,
		     (void*)self);
	zloop_reader(self->loop, self->rpc,
		     (zloop_reader_fn*)nrm_client_broker_rpc_handler,
		     (void*)self);
	zloop_reader(self->loop, self->sub,
		     (zloop_reader_fn*)nrm_client_broker_sub_handler,
		     (void*)self);
	/* notify we are ready */
	zsock_signal(self->pipe, 0);

	/* start: will only return when broker is destroyed */
	zloop_start(self->loop);

	zloop_destroy(&self->loop);
	zsock_destroy(&self->rpc);
	zsock_destroy(&self->sub);
	free(self);
}

nrm_role_t *nrm_role_client_create_fromparams(const char *uri,
					      int sub_port,
					      int rpc_port,
					      int transmit)
{
	nrm_role_t *role;
	struct nrm_role_client_s *data;
	
	role = NRM_INNER_MALLOC(nrm_role_t, struct nrm_role_client_s);
	if (role == NULL)
		return NULL;

	data = NRM_INNER_MALLOC_GET_FIELD(role, 2, nrm_role_t, struct
					  nrm_role_client_s);
	role->data = (struct nrm_role_data *)data;
	role->ops = &nrm_role_client_ops;

	/* env init */
	struct nrm_client_broker_args bargs;
	bargs.uri = uri;
	bargs.sub_port = sub_port;
	bargs.rpc_port = rpc_port;
	bargs.transmit = transmit;

	/* create broker */
	data->broker = zactor_new(nrm_client_broker_fn, &bargs);
	if (data->broker == NULL)
		return NULL;
	zsock_set_unbounded(data->broker);
	return role;
}

void nrm_role_client_destroy(nrm_role_t **role)
{
	if (role == NULL || *role == NULL)
		return;

	struct nrm_role_client_s *client = (struct nrm_role_client_s
					    *)(*role)->data;
	/* now exit: in principle this should just send a message on the pipe
	 * and wait for the actor to exit by itself */
	zactor_destroy(&client->broker);

	free(*role);
	*role = NULL;	
}

int nrm_role_client_send(const struct nrm_role_data *data,
			 nrm_msg_t *msg)
{
	struct nrm_role_client_s *client = (struct nrm_role_client_s *)data;
	nrm_ctrlmsg_send((zsock_t *)client->broker, NRM_CTRLMSG_TYPE_SEND, msg);
	return 0;
}

nrm_msg_t *nrm_role_client_recv(const struct nrm_role_data *data)
{
	struct nrm_role_client_s *client = (struct nrm_role_client_s *)data;
	nrm_msg_t *msg;
	int msgtype;
	msg = nrm_ctrlmsg_recv((zsock_t *)client->broker, &msgtype);
	assert(msgtype == NRM_CTRLMSG_TYPE_RECV);
	return msg;
}

nrm_msg_t *nrm_role_client_sub(const struct nrm_role_data *data)
{
	struct nrm_role_client_s *client = (struct nrm_role_client_s *)data;
	nrm_msg_t *msg;
	int msgtype;
	msg = nrm_ctrlmsg_recv((zsock_t *)client->broker, &msgtype);
	assert(msgtype == NRM_CTRLMSG_TYPE_SUB);
	return msg;
}

struct nrm_role_ops nrm_role_client_ops = {
	nrm_role_client_send,
	nrm_role_client_recv,
	NULL,
	nrm_role_client_sub,
	nrm_role_client_destroy,
};
