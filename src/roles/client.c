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
#include <sys/signalfd.h>

#include "internal/nrmi.h"

#include "internal/messages.h"
#include "internal/roles.h"

int signo;

struct nrm_client_sub_cb_s {
	nrm_role_sub_callback_fn *fn;
	void *arg;
};

struct nrm_client_cmd_cb_s {
	nrm_role_cmd_callback_fn *fn;
	void *arg;
};

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
	/* pointer to the sub callback */
	struct nrm_client_sub_cb_s *sub_cb;
	/* pointer to the cmd callback */
	struct nrm_client_cmd_cb_s *cmd_cb;
};

struct nrm_client_broker_args {
	/* uri for out connection */
	const char *uri;
	/* port to use for client */
	int rpc_port;
	/* port to use for client */
	int sub_port;
	/* pointer to the sub callback */
	struct nrm_client_sub_cb_s *sub_cb;
	/* pointer to the cmd callback */
	struct nrm_client_cmd_cb_s *cmd_cb;
};

struct nrm_role_client_s {
	zactor_t *broker;
	struct nrm_client_sub_cb_s sub_cb;
	struct nrm_client_cmd_cb_s cmd_cb;
};

int nrm_client_broker_pipe_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_client_broker_s *self = (struct nrm_client_broker_s *)arg;
	(void)self;

	/* pipe messages are in shared memory, not actually packed on
	 * the network */
	int msg_type;
	void *p, *q;
	nrm_msg_t *msg = NULL;
	nrm_string_t s = NULL;
	nrm_ctrlmsg__recv(socket, &msg_type, &p, &q);
	switch (msg_type) {
	case NRM_CTRLMSG_TYPE_SEND:
		NRM_CTRLMSG_2SEND(p, q, msg);
		nrm_log_debug("client sending message\n");
		nrm_log_printmsg(NRM_LOG_DEBUG, msg);
		nrm_msg_send(self->rpc, msg);
		nrm_msg_destroy_created(&msg);
		break;
	case NRM_CTRLMSG_TYPE_SUB:
		NRM_CTRLMSG_2SUB(p, q, s);
		nrm_log_debug("client subscribing to %s\n", s);
		nrm_net_sub_set_topic(self->sub, s);
		break;
	case NRM_CTRLMSG_TYPE_TERM:
		/* returning -1 exits the loop */
		return -1;
	}
	return 0;
}

int nrm_client_broker_rpc_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_client_broker_s *self = (struct nrm_client_broker_s *)arg;

	/* this should be a rpc answer, that we need to transmit to the pipe */
	nrm_msg_t *msg = nrm_msg_recv(socket);
	nrm_log_debug("received rpc message\n");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	if (nrm_msg_is_reply(msg))
		nrm_ctrlmsg_sendmsg(self->pipe, NRM_CTRLMSG_TYPE_RECV, msg,
		                    NULL);
	else {
		if (self->cmd_cb->fn != NULL)
			self->cmd_cb->fn(msg, self->cmd_cb->arg);
		else
			nrm_log_debug("no cmd callback to call\n");
		nrm_msg_destroy_received(&msg);
	}
	return 0;
}

int nrm_client_broker_sub_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_client_broker_s *self = (struct nrm_client_broker_s *)arg;

	/* this should be a sub message, that we need to transmit to the
	 * callback */
	nrm_string_t topic;
	nrm_msg_t *msg = nrm_msg_sub(socket, &topic);
	nrm_log_debug("received subscribed message\n");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	if (self->sub_cb == NULL || self->sub_cb->fn == NULL)
		nrm_log_debug("no callback to call\n");
	else
		self->sub_cb->fn(msg, self->sub_cb->arg);
	nrm_msg_destroy_received(&msg);
	return 0;
}

int nrm_client_broker_signal_callback(zloop_t *loop,
                                      zmq_pollitem_t *poller,
                                      void *arg)
{
	struct signalfd_siginfo fdsi;
	ssize_t s = read(poller->fd, &fdsi, sizeof(struct signalfd_siginfo));
	assert(s == sizeof(struct signalfd_siginfo));
	signo = fdsi.ssi_signo;
	nrm_log_info("Caught SIGINT\n");
	return -1;
}

void nrm_client_broker_fn(zsock_t *pipe, void *args)
{
	int err, sfd;
	sigset_t sigmask;
	struct nrm_client_broker_s *self;
	struct nrm_client_broker_args *params;

	assert(pipe);
	assert(args);

	/* avoid losing messages */
	zsock_set_unbounded(pipe);
	self = calloc(1, sizeof(struct nrm_client_broker_s));
	if (self == NULL) {
		zsock_signal(pipe, 1);
		return;
	}

	self->pipe = pipe;
	params = (struct nrm_client_broker_args *)args;

	self->sub_cb = params->sub_cb;
	self->cmd_cb = params->cmd_cb;

	/* init network */
	fprintf(stderr, "client: creating rpc socket\n");
	err = nrm_net_rpc_client_init(&self->rpc);
	assert(!err);
	err = nrm_net_connect_and_wait_2(self->rpc, params->uri,
	                                 params->rpc_port);
	assert(!err);

	fprintf(stderr, "client: creating sub socket\n");
	err = nrm_net_sub_init(&self->sub);
	assert(!err);
	err = nrm_net_connect_and_wait_2(self->sub, params->uri,
	                                 params->sub_port);
	assert(!err);

	/* set ourselves up to handle messages */
	fprintf(stderr, "client: finishing setup\n");
	self->loop = zloop_new();
	assert(self->loop != NULL);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGINT);
	sfd = signalfd(-1, &sigmask, 0);
	assert(sfd != -1);

	zmq_pollitem_t signal_poller = {0, sfd, ZMQ_POLLIN, 0};
	/* register signal handler callback */
	zloop_poller(self->loop, &signal_poller,
	             (zloop_fn *)nrm_client_broker_signal_callback, NULL);

	zloop_reader(self->loop, self->pipe,
	             (zloop_reader_fn *)nrm_client_broker_pipe_handler,
	             (void *)self);
	zloop_reader(self->loop, self->rpc,
	             (zloop_reader_fn *)nrm_client_broker_rpc_handler,
	             (void *)self);
	zloop_reader(self->loop, self->sub,
	             (zloop_reader_fn *)nrm_client_broker_sub_handler,
	             (void *)self);
	/* notify we are ready */
	zsock_signal(self->pipe, 0);

	/* start: will only return when broker is destroyed */
	zloop_start(self->loop);

	zloop_destroy(&self->loop);
	zsock_destroy(&self->rpc);
	zsock_destroy(&self->sub);
	free(self);
}

nrm_role_t *
nrm_role_client_create_fromparams(const char *uri, int sub_port, int rpc_port)
{
	nrm_role_t *role;
	struct nrm_role_client_s *data;

	role = NRM_INNER_MALLOC(nrm_role_t, struct nrm_role_client_s);
	if (role == NULL)
		return NULL;

	data = NRM_INNER_MALLOC_GET_FIELD(role, 2, nrm_role_t,
	                                  struct nrm_role_client_s);
	role->data = (struct nrm_role_data *)data;
	role->ops = &nrm_role_client_ops;

	/* env init */
	struct nrm_client_broker_args bargs;
	bargs.uri = uri;
	bargs.sub_port = sub_port;
	bargs.rpc_port = rpc_port;
	bargs.sub_cb = &(data->sub_cb);
	bargs.cmd_cb = &(data->cmd_cb);

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

	struct nrm_role_client_s *client =
	        (struct nrm_role_client_s *)(*role)->data;
	/* now exit: in principle this should just send a message on the pipe
	 * and wait for the actor to exit by itself */
	zactor_destroy(&client->broker);

	free(*role);
	*role = NULL;
}

int nrm_role_client_send(const struct nrm_role_data *data,
                         nrm_msg_t *msg,
                         nrm_uuid_t *to)
{
	struct nrm_role_client_s *client = (struct nrm_role_client_s *)data;
	nrm_ctrlmsg_sendmsg((zsock_t *)client->broker, NRM_CTRLMSG_TYPE_SEND,
	                    msg, to);
	return 0;
}

nrm_msg_t *nrm_role_client_recv(const struct nrm_role_data *data,
                                nrm_uuid_t **from)
{
	struct nrm_role_client_s *client = (struct nrm_role_client_s *)data;
	nrm_msg_t *msg;
	int msgtype;
	msg = nrm_ctrlmsg_recvmsg((zsock_t *)client->broker, &msgtype, from);
	assert(msgtype == NRM_CTRLMSG_TYPE_RECV);
	return msg;
}

int nrm_role_client_register_sub_cb(const struct nrm_role_data *data,
                                    nrm_role_sub_callback_fn *fn,
                                    void *arg)
{
	struct nrm_role_client_s *client = (struct nrm_role_client_s *)data;
	client->sub_cb.fn = fn;
	client->sub_cb.arg = arg;
	return 0;
}

int nrm_role_client_register_cmd_cb(const struct nrm_role_data *data,
                                    nrm_role_cmd_callback_fn *fn,
                                    void *arg)
{
	struct nrm_role_client_s *client = (struct nrm_role_client_s *)data;
	client->cmd_cb.fn = fn;
	client->cmd_cb.arg = arg;
	return 0;
}

int nrm_role_client_sub(const struct nrm_role_data *data, nrm_string_t topic)
{
	struct nrm_role_client_s *client = (struct nrm_role_client_s *)data;
	nrm_ctrlmsg_sub((zsock_t *)client->broker, NRM_CTRLMSG_TYPE_SUB, topic);
	return 0;
}

struct nrm_role_ops nrm_role_client_ops = {
        nrm_role_client_send,
        nrm_role_client_recv,
        NULL,
        nrm_role_client_register_sub_cb,
        nrm_role_client_sub,
        nrm_role_client_register_cmd_cb,
        nrm_role_client_destroy,
};
