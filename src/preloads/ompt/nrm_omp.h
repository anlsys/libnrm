/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the nrm-extra project.
 * For more info, see https://github.com/anlsys/nrm-extra
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#ifndef NRM_OMP_H
#define NRM_OMP_H 1

#include <omp-tools.h>

#include <nrm.h>

#ifdef __cplusplus
extern "C" {
#endif

extern nrm_client_t *global_client;
extern nrm_scope_t *global_scope;
extern nrm_sensor_t *global_sensor;

extern char *upstream_uri;
extern int pub_port;
extern int rpc_port;

extern ompt_set_callback_t nrm_ompt_set_callback;

void nrm_ompt_register_cbs(void);

#ifdef __cplusplus
}
#endif

#endif
