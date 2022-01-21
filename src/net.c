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
	return zmq_send(ctxt->socket, buf, strnlen(buf, bufsize), flags);
}
