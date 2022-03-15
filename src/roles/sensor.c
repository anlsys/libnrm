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
};

struct nrm_role_sensor_s {
	zactor_t *broker;
	/* command id, not sure this should survive in the long term */
	const char *cmdid;
	/* name of the sensor */
	char *name;
};

int nrm_sensor_broker_pipe_handler(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	struct nrm_sensor_broker_s *self = (struct nrm_sensor_broker_s *)arg;

	/* pipe messages are in shared memory, not actually packed on
	 * the network */
	int msg_type;
	nrm_msg_t *msg = nrm_ctrlmsg_recv(socket, &msg_type);
	if (msg_type == NRM_CTRLMSG_TYPE_SEND
	    && nrm_transmit) {
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
	err = nrm_net_rpc_client_init(&self->out);
	assert(!err);
	err = nrm_net_connect_and_wait(self->out, params->uri);
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

nrm_role_t *nrm_role_sensor_create_fromenv(const char *name)
{
	nrm_role_t *role;
	struct nrm_role_sensor_s *data;
	
	role = NRM_INNER_MALLOC(nrm_role_t, struct nrm_role_sensor_s);
	if (role == NULL)
		return NULL;

	data = NRM_INNER_MALLOC_GET_FIELD(role, 2, nrm_role_t, struct
					  nrm_role_sensor_s);
	role->data = (struct nrm_role_data *)data;
	role->ops = &nrm_role_sensor_ops;

	/* env init */
	struct nrm_sensor_broker_args bargs;
	bargs.uri = getenv(NRM_ENV_DOWNSTREAM_URI);
	if (bargs.uri == NULL)
		bargs.uri = NRM_DEFAULT_DOWNSTREAM_URI;

	/* context init */
	data->cmdid = getenv(NRM_ENV_DOWNSTREAM_CMDID);
	assert(data->cmdid != NULL);

	/* copy sensor name */
	size_t bufsize = snprintf(NULL, 0, "%s", name);
	bufsize++;
	data->name = calloc(1,bufsize);
	assert(data->name != NULL);
	snprintf(data->name, bufsize, "%s", name);

	/* create broker */
	data->broker = zactor_new(nrm_sensor_broker_fn, &bargs);
	if (data->broker == NULL)
		return NULL;
	zsock_set_unbounded(data->broker);
	return role;
}

void nrm_role_sensor_destroy(nrm_role_t **role)
{
	if (role == NULL || *role == NULL)
		return;

	struct nrm_role_sensor_s *sensor = (struct nrm_role_sensor_s
					    *)(*role)->data;
	nrm_msg_t *msg;

	/* need to push a pause message before the exit */
	nrm_time_t now;
	nrm_time_gettime(&now);
	msg = nrm_msg_new_pause(now);
	nrm_ctrlmsg_send((zsock_t *)sensor->broker, NRM_CTRLMSG_TYPE_SEND, msg);

	/* now exit: in principle this should just send a message on the pipe
	 * and wait for the actor to exit by itself */
	zactor_destroy(&sensor->broker);

	free(sensor->name);
	free(*role);
	*role = NULL;	
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

int nrm_role_sensor_send(const struct nrm_role_data *data,
			 nrm_msg_t *msg)
{
	struct nrm_role_sensor_s *sensor = (struct nrm_role_sensor_s *)data;
	nrm_ctrlmsg_send((zsock_t *)sensor->broker, NRM_CTRLMSG_TYPE_SEND, msg);
	return 0;
}

struct nrm_role_ops nrm_role_sensor_ops = {
	nrm_role_sensor_send,
	NULL,
	NULL,
	NULL,
	nrm_role_sensor_destroy,
};
