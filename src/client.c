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

struct nrm_client_s {
	nrm_role_t *role;
};

int nrm_client_create(nrm_client_t **client, const char *uri, int pub_port,
		      int rpc_port)
{
	if (client == NULL || uri == NULL)
		return -NRM_EINVAL;

	nrm_client_t *ret = calloc(1, sizeof(nrm_client_t));
	if (ret == NULL)
		return -NRM_ENOMEM;

	ret->role = nrm_role_client_create_fromparams(uri, pub_port, rpc_port);
	if (ret->role == NULL)
		return -NRM_EINVAL;

	*client = ret;
	return 0;
}

int nrm_client_list_sensors(const nrm_client_t *client, nrm_vector_t **sensors)
{
	if (client == NULL || sensors == NULL)
		return -NRM_EINVAL;

	int err;
	/* craft the message we want to send */
	nrm_log_debug("crafting message\n");
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_LIST);
	nrm_msg_set_list_sensors(msg, NULL);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending request\n");
	nrm_role_send(client->role, msg, NULL);

	/* wait for the answer */
	nrm_log_debug("receiving reply\n");
	msg = nrm_role_recv(client->role, NULL);
	nrm_log_debug("parsing reply\n");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);

	nrm_vector_t *ret;
	err = nrm_vector_create(&ret, sizeof(nrm_sensor_t));
	if (err)
		return err;

	assert(msg->type == NRM_MSG_TYPE_LIST);
	assert(msg->list->type == NRM_MSG_TARGET_TYPE_SENSOR);
	for (size_t i = 0; i < msg->list->sensors->n_sensors; i++) {
		nrm_sensor_t *s =
			nrm_sensor_create_frommsg(msg->list->sensors->sensors[i]);
		nrm_vector_push_back(ret, s);
	}
	*sensors = ret;
	return 0;
}

int nrm_client_list_slices(const nrm_client_t *client, nrm_vector_t **slices)
{
	if (client == NULL || slices == NULL)
		return -NRM_EINVAL;

	int err;
	/* craft the message we want to send */
	nrm_log_debug("crafting message\n");
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_LIST);
	nrm_msg_set_list_slices(msg, NULL);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending request\n");
	nrm_role_send(client->role, msg, NULL);

	/* wait for the answer */
	nrm_log_debug("receiving reply\n");
	msg = nrm_role_recv(client->role, NULL);
	nrm_log_debug("parsing reply\n");
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);

	nrm_vector_t *ret;
	err = nrm_vector_create(&ret, sizeof(nrm_slice_t));
	if (err)
		return err;

	assert(msg->type == NRM_MSG_TYPE_LIST);
	assert(msg->list->type == NRM_MSG_TARGET_TYPE_SLICE);
	for (size_t i = 0; i < msg->list->slices->n_slices; i++) {
		nrm_slice_t *s =
			nrm_slice_create_frommsg(msg->list->slices->slices[i]);
		nrm_vector_push_back(ret, s);
	}
	*slices = ret;
	return 0;
}

void nrm_client_destroy(nrm_client_t **client)
{
	if (client == NULL || *client == NULL)
		return;

	nrm_client_t *c = *client;
	nrm_role_destroy(&c->role);
	free(c);
	*client = NULL;
}
