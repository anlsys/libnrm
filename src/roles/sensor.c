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
struct nrm_sensor_broker_s {
	/* communication channel with the creator of the broker */
	zsock_t *pipe;
	/* socket used to send out sensor messages */
	zsock_t *out;
	/* monitoring loop */
	zloop_t *loop;
	/* transmit ok */
	int transmit;
};

struct nrm_sensor_broker_args {
	/* uri for out connection */
	const char *uri;
	/* whether or not to transmit messages */
	int transmit;
};

struct nrm_role_sensor_s {
	zactor_t *broker;
	/* command id, not sure this should survive in the long term */
	const char *cmdid;
	/* name of the sensor */
	char *name;
};

int nrm_sensor_out_init(struct nrm_sensor_broker_s *self, const char *uri)
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
	self->out = zsock_new(ZMQ_DEALER); assert(self->out != NULL);
	zsock_set_immediate(self->out, 1);
	zsock_set_unbounded(self->out);

	zactor_t *monitor = zactor_new(zmonitor, self->out);
	assert(monitor != NULL);

	zstr_sendx(monitor, "LISTEN", "CONNECTED");
	zstr_send(monitor, "START");
	zsock_wait(monitor);

	/* now connect the original client */
	err = zsock_connect(self->out, "%s", uri);
	assert(err == 0);

	connected = 0;
	while(!connected) {
		/* read first frame for event number */
		zmsg_t *msg = zmsg_recv(monitor);
		assert(msg != NULL);

		char *event = zmsg_popstr (msg);

		if(!strcmp(event, "CONNECTED"))
			connected = 1;
		free(event);
		zmsg_destroy(&msg);
	}
	/* cleanup monitor */
	zactor_destroy(&monitor);
	return 0;

}

int nrm_sensor_broker_pipe_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_sensor_broker_s *self = (struct nrm_sensor_broker_s *)arg;

	/* pipe messages are in shared memory, not actually packed on
	 * the network */
	int msg_type;
	nrm_msg_t *msg = nrm_ctrlmsg_recv(socket, &msg_type);
	if (msg_type == NRM_CTRLMSG_TYPE_SEND
	    && self->transmit) {
		/* just send it back for now, we'll implement aggregation
		 * another time
		 */
		nrm_msg_send(self->out, msg);
	}
	else if(msg_type == NRM_CTRLMSG_TYPE_TERM) {
		/* returning -1 exits the loop */
		return -1;
	}
	/* we've taken ownership of the message at this point */
	nrm_msg_destroy(&msg);
	return 0;
}

void nrm_sensor_broker_fn(zsock_t *pipe, void *args)
{
	int err;
	struct nrm_sensor_broker_s *self;
	struct nrm_sensor_broker_args *params;

	assert(pipe);
	assert(args);

	/* avoid losing messages */
	zsock_set_unbounded(pipe);
	self = calloc(1, sizeof(struct nrm_sensor_broker_s));
	if (self == NULL)
	{
		zsock_signal(pipe,1);
		return;
	}

	self->pipe = pipe;
	params = (struct nrm_sensor_broker_args *)args;

	/* init network */
	self->transmit = params->transmit;
	err = nrm_sensor_out_init(self, params->uri);
	assert(!err);

	/* set ourselves up to handle messages */
	self->loop = zloop_new();
	assert(self->loop != NULL);

	zloop_reader(self->loop, self->pipe,
		     (zloop_reader_fn*)nrm_sensor_broker_pipe_handler,
		     (void*)self);

	/* notify we are ready */
	zsock_signal(self->pipe, 0);

	/* start: will only return when broker is destroyed */
	zloop_start(self->loop);

	zloop_destroy(&self->loop);
	zsock_destroy(&self->out);
	free(self);
}

struct nrm_role_sensor_s *nrm_role_sensor_create_fromenv(const char *name)
{
	struct nrm_role_sensor_s *role;
	role = calloc(1, sizeof(*role));
	if (role == NULL)
		return NULL;

	/* env init */
	struct nrm_sensor_broker_args bargs;
	bargs.uri = getenv(NRM_ENV_DOWNSTREAM_URI);
	if (bargs.uri == NULL)
		bargs.uri = NRM_DEFAULT_DOWNSTREAM_URI;
	const char *transmit = getenv(NRM_ENV_TRANSMIT);
	if (transmit != NULL)
		bargs.transmit = atoi(transmit);
	else
		bargs.transmit = 1;

	/* context init */
	role->cmdid = getenv(NRM_ENV_DOWNSTREAM_CMDID);
	assert(role->cmdid != NULL);

	/* copy sensor name */
	size_t bufsize = snprintf(NULL, 0, "%s", name);
	bufsize++;
	role->name = calloc(1,bufsize);
	assert(role->name != NULL);
	snprintf(role->name, bufsize, "%s", name);

	/* create broker */
	role->broker = zactor_new(nrm_sensor_broker_fn, &bargs);
	if (role->broker == NULL)
		return NULL;
	zsock_set_unbounded(role->broker);
	return role;
}

int nrm_role_sensor_destroy(struct nrm_role_sensor_s *role)
{
	nrm_msg_t *msg;

	/* need to push a pause message before the exit */
	nrm_time_t now;
	nrm_time_gettime(&now);
	msg = nrm_msg_new_pause(now);
	nrm_ctrlmsg_send((zsock_t *)role->broker, NRM_CTRLMSG_TYPE_SEND, msg);

	/* now exit: in principle this should just send a message on the pipe
	 * and wait for the actor to exit by itself */
	zactor_destroy(&role->broker);

	free(role->name);
	free(role);
	return 0;
}

int nrm_role_sensor_send_progress(struct nrm_role_sensor_s *s,
				  unsigned long progress, nrm_scope_t *scope)
{
	nrm_msg_t *msg;
	nrm_time_t now;
	nrm_time_gettime(&now);
	msg = nrm_msg_new_progress(now, progress, scope);
	nrm_ctrlmsg_send((zsock_t *)s->broker, NRM_CTRLMSG_TYPE_SEND, msg);
	return 0;
}
