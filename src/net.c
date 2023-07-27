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
 * - the downstream API uses rpc sockets to simulate a 1-1 pubsub without loss
 * of messages, resulting in massive amount of messages going from the client to
 *   the server without any message in the other direction.
 */

int nrm_net_rpc_client_init(zsock_t **socket)
{
	if (!nrm_transmit)
		return 0;

	zsock_t *ret;
	ret = zsock_new(ZMQ_DEALER);
	if (ret == NULL)
		return 1;
	/* avoid pushing messages to incomplete connections */
	zsock_set_immediate(ret, 1);
	/* buffer as many messages as possible */
	zsock_set_unbounded(ret);
	/* set timeout on send/recv */
	zsock_set_sndtimeo(ret, nrm_timeout);
	zsock_set_rcvtimeo(ret, nrm_timeout);
	/* add an identity to the socket */
	nrm_uuid_t *uuid = nrm_uuid_create();
	zsock_set_identity(ret, nrm_uuid_to_char(uuid));
	nrm_uuid_destroy(&uuid);
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
	/* set timeout on send/recv */
	zsock_set_sndtimeo(ret, nrm_timeout);
	zsock_set_rcvtimeo(ret, nrm_timeout);
	*socket = ret;
	return 0;
}

int nrm_net_sub_init(zsock_t **socket)
{
	if (!nrm_transmit)
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

int nrm_net_sub_set_topic(zsock_t *socket, const char *topic)
{
	zsock_set_subscribe(socket, topic);
	return 0;
}

int nrm_net_pub_init(zsock_t **socket)
{
	if (!nrm_transmit)
		return 0;

	zsock_t *ret = zsock_new(ZMQ_PUB);
	if (ret == NULL)
		return 1;
	/* buffer as many messages as possible */
	zsock_set_sndhwm(ret, 0);
	*socket = ret;
	return 0;
}

int nrm_net_connect_and_wait(zsock_t *socket, const char *uri, int port)
{
	/* the process is a little more complicated than you would think, as
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

	nrm_log_debug("creating socket monitor\n");
	zactor_t *monitor = zactor_new(zmonitor, socket);
	assert(monitor != NULL);

	nrm_log_debug("configuring socket monitor\n");
	zstr_sendx(monitor, "LISTEN", "CONNECTED", NULL);
	zstr_send(monitor, "START");
	nrm_log_debug("waiting socket monitor\n");
	zsock_wait(monitor);
	zsock_set_rcvtimeo(monitor, nrm_timeout);

	/* now connect the original client */
	nrm_log_debug("connecting to %s:%d\n", uri, port);
	err = zsock_connect(socket, "%s:%d", uri, port);
	if (err) {
		nrm_log_error("error connecting %d\n", err);
		goto cleanup;
	}

	int connected = 0;
	while (!connected) {
		/* read first frame for event number */
		zmsg_t *msg = zmsg_recv(monitor);
		if (msg == NULL) {
			nrm_log_error("socket monitor timeout\n");
			err = -NRM_FAILURE;
			goto cleanup;
		}

		char *event = zmsg_popstr(msg);

		nrm_log_debug("monitor event %s\n", event);
		if (!strcmp(event, "CONNECTED"))
			connected = 1;
		free(event);
		zmsg_destroy(&msg);
	}
	nrm_log_debug("connection established\n");
cleanup:
	/* cleanup monitor */
	zactor_destroy(&monitor);
	return err;
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
