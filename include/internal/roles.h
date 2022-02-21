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

#include <time.h>
#include <zmq.h>
#include <czmq.h>
#include <jansson.h>

#include "internal/nrmi.h"

struct nrm_role_monitor_s;

struct nrm_role_monitor_s *nrm_role_monitor_create_fromenv();

zsock_t *nrm_role_monitor_broker(struct nrm_role_monitor_s *role);

#ifdef __cplusplus
}
#endif

#endif
