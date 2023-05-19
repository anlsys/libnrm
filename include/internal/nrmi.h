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

#include <czmq.h>
#include <jansson.h>
#include <time.h>
#include <zmq.h>

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

#define NRM_DEFAULT_UPSTREAM_URI "tcp://127.0.0.1"
#define NRM_DEFAULT_UPSTREAM_PUB_PORT 2345
#define NRM_DEFAULT_UPSTREAM_RPC_PORT 3456
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

extern int nrm_transmit;

/*******************************************************************************
 * NET
 ******************************************************************************/

int nrm_net_rpc_client_init(zsock_t **socket);
int nrm_net_rpc_server_init(zsock_t **socket);
int nrm_net_sub_init(zsock_t **socket);
int nrm_net_sub_set_topic(zsock_t *socket, const char *topic);
int nrm_net_pub_init(zsock_t **socket);
int nrm_net_connect_and_wait(zsock_t *socket, const char *uri);
int nrm_net_connect_and_wait_2(zsock_t *socket, const char *uri, int port);
int nrm_net_bind(zsock_t *socket, const char *uri);
int nrm_net_bind_2(zsock_t *socket, const char *uri, int port);

/*******************************************************************************
 * Utils functions
 ******************************************************************************/

struct nrm_scope {
	nrm_string_t uuid;
	struct nrm_bitmap maps[NRM_SCOPE_TYPE_MAX];
};

int nrm_actuator_from_json(nrm_actuator_t *, json_t *);
int nrm_bitmap_from_json(nrm_bitmap_t *, json_t *);
int nrm_scope_from_json(nrm_scope_t *, json_t *);
int nrm_sensor_from_json(nrm_sensor_t *, json_t *);
int nrm_slice_from_json(nrm_slice_t *, json_t *);
int nrm_time_from_json(nrm_time_t *, json_t *);

json_t *nrm_actuator_to_json(nrm_actuator_t *);
json_t *nrm_bitmap_to_json(nrm_bitmap_t *);
json_t *nrm_uuid_to_json(nrm_uuid_t *);
json_t *nrm_scope_to_json(nrm_scope_t *);
json_t *nrm_sensor_to_json(nrm_sensor_t *);
json_t *nrm_slice_to_json(nrm_slice_t *);
json_t *nrm_time_to_json(nrm_time_t *);

#ifdef __cplusplus
}
#endif

#endif
