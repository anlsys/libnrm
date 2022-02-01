/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

/* Primitives for communication between NRM clients and servers.
 * This file mostly contains code needed to handle zmq socket setup.
 *
 */
#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nrm.h"

#include "nrm-internal.h"

int nrm_net_down_client_init(struct nrm_net_ctxt *ctxt, const char *uri)
{
	int err;
	int immediate = 1;
	void *monitor;
	zmq_msg_t msg;
	if (!nrm_transmit)
		return 0;
	ctxt->context = zmq_ctx_new();

	/* the process is a little more complicated that you would think,
	 * as we're trying to avoid returning from this function before the
	 * socket is fully connected.
	 * To do that, we also create an ephemeral socket pair that acts as
	 * a monitor on our emitter socket, and loop until we actually receive a
	 * connection message.
	 * To avoid missing that message on the monitor, we're also careful of
	 * connecting the monitor before connecting the real socket to the
	 * server.
	 */
	ctxt->socket = zmq_socket(ctxt->context, ZMQ_DEALER);
	assert(ctxt->socket != NULL);
	err = zmq_setsockopt(ctxt->socket, ZMQ_IMMEDIATE, &immediate,
			     sizeof(immediate));
	assert(err == 0);

	/* not at all safe if a different thread tries to create a similar
	 * socket inside the same process */
	err = zmq_socket_monitor(ctxt->socket,
				 "inproc://nrm-net-down-client-monitor",
				 ZMQ_EVENT_ALL);
	assert(err == 0);

	monitor = zmq_socket(ctxt->context, ZMQ_PAIR);
	assert(monitor != NULL);

	err = zmq_connect(monitor, "inproc://nrm-net-down-client-monitor");
	assert(err == 0);

	/* now connect the original client */
	err = zmq_connect(ctxt->socket, uri);
	assert(err == 0);

	while(1) {
		/* read first frame for event number */
		zmq_msg_init(&msg);
		zmq_msg_recv(&msg, monitor, 0);

		uint8_t *data = (uint8_t *)zmq_msg_data(&msg);
		uint16_t event  = * (uint16_t *)(data);

		/* discard second frame */
		zmq_msg_init(&msg);
		zmq_msg_recv(&msg, monitor, 0);

		if(event == ZMQ_EVENT_CONNECTED)
			break;
	}
	/* cleanup monitor */
	zmq_close(monitor);
	return 0;
}

int nrm_net_down_server_init(struct nrm_net_ctxt *ctxt, const char *uri)
{
	int err;
	if (!nrm_transmit)
		return 0;
	ctxt->context = zmq_ctx_new();

	ctxt->socket = zmq_socket(ctxt->context, ZMQ_ROUTER);
	assert(ctxt->socket != NULL);

	/* now accept connections */
	err = zmq_bind(ctxt->socket, uri);
	assert(err == 0);

	return 0;
}

int nrm_net_fini(struct nrm_net_ctxt *ctxt)
{
	if (!nrm_transmit)
		return 0;
	zmq_close(ctxt->socket);
	zmq_ctx_destroy(ctxt->context);
	return 0;
}

int nrm_net_send(struct nrm_net_ctxt *ctxt, char *buf, size_t bufsize, int flags)
{
	if (!nrm_transmit)
		return 1;
	return zmq_send(ctxt->socket, buf, bufsize, flags);
}

int nrm_net_copy_frame(void **dest, zmq_msg_t *frame)
{
	size_t size;

	assert(dest != NULL);
	assert(frame != NULL);

	size = zmq_msg_size(frame);
	*dest = malloc(size);
	assert(*dest != NULL);

	memcpy(*dest, zmq_msg_data(frame), size);

	return 0;
}

/* the only multipart messages we receive are in the form <identity, content>, 
 * and that's mainly because zmq router sockets append that stuff to it in the
 * first place.
 * We use the msg infra of zmq to convert both into an identity buffer and a
 * content buffer.
 */
int nrm_net_recv_multipart(struct nrm_net_ctxt *ctxt, char **identity, char **content)
{
	int err;
	zmq_msg_t ident, cont;
	if (!nrm_transmit)
		return 1;

	/* receive the two frames in zmq_msg_t */
	err = zmq_msg_init(&ident);
	assert(err == 0);
	zmq_msg_init(&cont);
	assert(err == 0);
	err = zmq_msg_recv(&ident, ctxt->socket, 0);
	assert(err != -1);
	err = zmq_msg_recv(&cont, ctxt->socket, 0);
	assert(err != -1);

	/* the zmq doc is not very clear on ownership, but it seems that the
	 * data will be freed once we close the msg structs, so we need to copy
	 * them
	 */
	nrm_net_copy_frame((void**)identity, &ident);
	nrm_net_copy_frame((void**)content, &cont);
	
	zmq_msg_close(&ident);
	zmq_msg_close(&cont);

	return 0;
}
