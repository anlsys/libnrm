/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#ifndef NRM_VARIABLES_H
#define NRM_VARIABLES_H 1

/*******************************************************************************
 * Common environment variables
 ******************************************************************************/

/**
 * name of the environment variable to set the default daemon uri
 */
#define NRM_ENV_VAR_UPSTREAM_URI "NRM_UPSTREAM_URI"

/**
 * name of the environment variable to set the defaul daemon rpc port
 */
#define NRM_ENV_VAR_UPSTREAM_RPC_PORT "NRM_UPSTREAM_RPC_PORT"

/**
 * name of the environment variable to set the defaul daemon pub/sub port
 */
#define NRM_ENV_VAR_UPSTREAM_PUB_PORT "NRM_UPSTREAM_PUB_PORT"

/**
 * name of the environment variable to rate limit messages
 */
#define NRM_ENV_VAR_RATELIMIT "NRM_RATELIMIT"

/**
 * name of the environment variable for disabling message transmission
 * (if 0, the library will not open sockets or send messages
 */
#define NRM_ENV_VAR_TRANSMIT "NRM_TRANSMIT"

/**
 * name of the environment variable for default timeout value when
 * sending/receving messages (in milliseconds)
 */
#define NRM_ENV_VAR_TIMEOUT "NRM_TIMEOUT"

/*******************************************************************************
 * Common environment default values
 ******************************************************************************/

/**
 * default daemon uri
 */
#define NRM_DEFAULT_UPSTREAM_URI "tcp://127.0.0.1"

/**
 * defaul daemon rpc port
 */
#define NRM_DEFAULT_UPSTREAM_RPC_PORT 3456

/**
 * default daemon pub/sub port
 */
#define NRM_DEFAULT_UPSTREAM_PUB_PORT 2345

/**
 * default ratelimit threshold to avoid overflowing the socket, as an
 * interval between two messages in nanoseconds
 */
#define NRM_DEFAULT_RATELIMIT (10000000LL)

/**
 * default transmit value (1: enabled)
 */
#define NRM_DEFAULT_TRANSMIT 1

/**
 * default timeout value (1000: one second)
 */
#define NRM_DEFAULT_TIMEOUT 1000

/*******************************************************************************
 * Runtime variables storing these values:
 * - before nrm_init, contain default values
 * - after nrm_init, contain values updated by environment variables
 ******************************************************************************/

extern char *nrm_upstream_uri;
extern unsigned int nrm_upstream_rpc_port;
extern unsigned int nrm_upstream_pub_port;
extern unsigned long long nrm_ratelimit;
extern int nrm_transmit;
extern unsigned int nrm_timeout;

#endif /* NRM_VARIABLES_H */
