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

/* RPC setup gets unbounded messages in both directions because:
 * - messages are going in both directions
 * - the downstream API uses rpc sockets to simulate a 1-1 pubsub without loss of
 *   messages, resulting in massive amount of messages going from the client to
 *   the server without any message in the other direction.
 */

int nrm_net_rpc_client_init(zsock_t **socket)
{
	if(!nrm_transmit)
		return 0;

	zsock_t *ret;
	ret = zsock_new(ZMQ_DEALER);
	if (ret == NULL)
		return 1;
	/* avoid pushing messages to incomplete connections */
	zsock_set_immediate(ret, 1);
	/* buffer as many messages as possible */
	zsock_set_unbounded(ret);
	/* add an identity to the socket */
	nrm_uuid_t *uuid = nrm_uuid_create();
	nrm_string_t identity = nrm_string_frombuf(uuid->data, 16);
	zsock_set_identity(ret, identity);
	nrm_uuid_destroy(&uuid);
	nrm_string_decref(&identity);
	*socket = ret;
	return 0;
}

int nrm_net_rpc_server_init(zsock_t **socket)
{
	if (!nrm_transmit)
		return 0;

	zsock_t *ret;
	ret = zsock_new(ZMQ_ROUTER);
	if (ret == NULL)
		return 1;
	/* avoid pushing messages to incomplete connections */
	zsock_set_immediate(ret, 1);
	/* buffer as many messages as possible */
	zsock_set_unbounded(ret);
	*socket = ret;
	return 0;
}

int nrm_net_sub_init(zsock_t **socket)
{
	if(!nrm_transmit)
		return 0;

	zsock_t *ret = zsock_new(ZMQ_SUB);
	if (ret == NULL)
		return 1;
	/* buffer as many messages as possible */
	zsock_set_rcvhwm(ret, 0);
	/* subscribe to everything */
	zsock_set_subscribe(ret, "");
	*socket = ret;
	return 0;
}

int nrm_net_pub_init(zsock_t **socket)
{
	if(!nrm_transmit)
		return 0;

	zsock_t *ret = zsock_new(ZMQ_PUB);
	if (ret == NULL)
		return 1;
	/* buffer as many messages as possible */
	zsock_set_sndhwm(ret, 0);
	*socket = ret;
	return 0;
}

int nrm_net_connect_and_wait(zsock_t *socket, const char *uri)
{
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
	int err;

	if (!nrm_transmit)
		return 0;

	zactor_t *monitor = zactor_new(zmonitor, socket);
	assert(monitor != NULL);

	zstr_sendx(monitor, "LISTEN", "CONNECTED", NULL);
	zstr_send(monitor, "START");
	zsock_wait(monitor);

	/* now connect the original client */
	err = zsock_connect(socket, "%s", uri);
	assert(err == 0);

	int connected = 0;
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

int nrm_net_connect_and_wait_2(zsock_t *socket, const char *uri, int port)
{
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
	int err;

	if (!nrm_transmit)
		return 0;

	zactor_t *monitor = zactor_new(zmonitor, socket);
	assert(monitor != NULL);

	zstr_sendx(monitor, "LISTEN", "CONNECTED", NULL);
	zstr_send(monitor, "START");
	zsock_wait(monitor);

	/* now connect the original client */
	err = zsock_connect(socket, "%s:%d", uri, port);
	assert(err == 0);

	int connected = 0;
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

int nrm_net_bind(zsock_t *socket, const char *uri)
{
	int err;
	if (!nrm_transmit)
		return 0;

	err = zsock_bind(socket, "%s", uri);
	if (err == -1)
		return -errno;
	return 0;
}

int nrm_net_bind_2(zsock_t *socket, const char *uri, int port)
{
	int err;
	if (!nrm_transmit)
		return 0;

	err = zsock_bind(socket, "%s:%d", uri, port);
	if (err == -1) {
		return -errno;
	}
	return 0;
}
