/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#ifndef LIBNRM_INTERNAL_H
#define LIBNRM_INTERNAL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <jansson.h>
#include <time.h>
#include <zmq.h>
#include <czmq.h>

/*******************************************************************************
 * Compat functions
 ******************************************************************************/

#ifndef HAVE_GETCPU
int getcpu(unsigned int *cpu, unsigned int *node);
#endif

/*******************************************************************************
 * LIBNRM Defaults Values
 ******************************************************************************/

/** default URI for the NRM zmq socket for the downstream API */
#define NRM_DEFAULT_DOWNSTREAM_URI "ipc:///tmp/nrm-downstream-event"

/** default ratelimit threshold to avoid overflowing the socket, as an
 * interval between two messages in nanoseconds */
#define NRM_DEFAULT_RATELIMIT_THRESHOLD (10000000LL)

/*******************************************************************************
 * LIBNRM Environmnent variables
 ******************************************************************************/

/** env variable to change the downstream URI **/
#define NRM_ENV_DOWNSTREAM_URI "NRM_DOWNSTREAM_EVENT_URI"

/** env variable for downstream client uuid */
#define NRM_ENV_DOWNSTREAM_CMDID "NRM_CMDID"

/** env variable for ratelimit on messages */
#define NRM_ENV_RATELIMIT "NRM_RATELIMIT"

/** env variable for disabling message transmission (if 0, the library will not
 * open sockets or send messages
 */
#define NRM_ENV_TRANSMIT "NRM_TRANSMIT"

/*******************************************************************************
 * Utils functions
 ******************************************************************************/

json_t *nrm_time_to_json(nrm_time_t *t);

json_t *nrm_scope_to_json(nrm_scope_t *s);

json_t *nrm_bitmap_to_json(nrm_bitmap_t *b);

#ifdef __cplusplus
}
#endif

#endif
