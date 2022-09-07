/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include "nrm.h"

#include "internal/nrmi.h"
#include "internal/roles.h"

int nrm_role_send(const nrm_role_t *role, nrm_msg_t *msg, nrm_uuid_t *to)
{
	if (role == NULL || role->ops == NULL || role->ops->send == NULL)
		return -1;
	return role->ops->send(role->data, msg, to);
}

nrm_msg_t *nrm_role_recv(const nrm_role_t *role, nrm_uuid_t **from)
{
	if (role == NULL || role->ops == NULL || role->ops->recv == NULL)
		return NULL;
	return role->ops->recv(role->data, from);
}

int nrm_role_pub(const nrm_role_t *role, nrm_string_t topic, nrm_msg_t *msg)
{
	if (role == NULL || role->ops == NULL || role->ops->pub == NULL)
		return -1;
	return role->ops->pub(role->data, topic, msg);
}

int nrm_role_register_sub_cb(const nrm_role_t *role,
                             nrm_role_sub_callback_fn *fn,
                             void *arg)
{
	if (role == NULL || role->ops == NULL ||
	    role->ops->register_sub_cb == NULL)
		return -NRM_ENOTSUP;
	return role->ops->register_sub_cb(role->data, fn, arg);
}

int nrm_role_register_cmd_cb(const nrm_role_t *role,
                             nrm_role_cmd_callback_fn *fn,
                             void *arg)
{
	if (role == NULL || role->ops == NULL ||
	    role->ops->register_cmd_cb == NULL)
		return -NRM_ENOTSUP;
	return role->ops->register_cmd_cb(role->data, fn, arg);
}

int nrm_role_sub(const nrm_role_t *role, nrm_string_t topic)
{
	if (role == NULL || role->ops == NULL || role->ops->sub == NULL)
		return -NRM_ENOTSUP;
	return role->ops->sub(role->data, topic);
}

void nrm_role_destroy(nrm_role_t **role)
{
	if (*role != NULL) {
		(*role)->ops->destroy(role);
	}
}
