/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#ifndef LIBNRM_INTERNAL_ROLES_H
#define LIBNRM_INTERNAL_ROLES_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "internal/nrmi.h"
#include "internal/messages.h"

/*******************************************************************************
 * Global definitions
 ******************************************************************************/

struct nrm_role_data;

struct nrm_role_ops {
	int (*send)(const struct nrm_role_data *data, nrm_msg_t *msg);
	int (*recv)(const struct nrm_role_data *data, nrm_msg_t *msg);
	void (*destroy)(nrm_role_t **role);
};

struct nrm_role_s {
	struct nrm_role_ops *ops;
	struct nrm_role_data *data;
};

int nrm_role_register_recvcallback(zloop_t *loop, zloop_reader_fn *fn,
				   void *arg);

/*******************************************************************************
 * Monitor:
 * monitors sensor data, recv a message each time a sensor sends something
 ******************************************************************************/

nrm_role_t *nrm_role_monitor_create_fromenv();

/*******************************************************************************
 * Monitor:
 * monitors sensor data, recv a message each time a sensor sends something
 ******************************************************************************/

nrm_role_t *nrm_role_sensor_create_fromenv();

#ifdef __cplusplus
}
#endif

#endif
