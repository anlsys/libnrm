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
#include <stdlib.h>

const int nrm_version_major = NRM_VERSION_MAJOR;
const int nrm_version_minor = NRM_VERSION_MINOR;
const int nrm_version_patch = NRM_VERSION_PATCH;
const char *nrm_version_revision = NRM_VERSION_REVISION;
const char *nrm_version_string = NRM_VERSION_STRING;

char *nrm_upstream_uri = NRM_DEFAULT_UPSTREAM_URI;
unsigned int nrm_upstream_rpc_port = NRM_DEFAULT_UPSTREAM_RPC_PORT;
unsigned int nrm_upstream_pub_port = NRM_DEFAULT_UPSTREAM_PUB_PORT;
unsigned long long nrm_ratelimit = NRM_DEFAULT_RATELIMIT;
unsigned int nrm_timeout = NRM_DEFAULT_TIMEOUT;
int nrm_transmit = NRM_DEFAULT_TRANSMIT;
int nrm_errno = 0;

int nrm_init(int *argc, char **argv[])
{
	(void)argc;
	(void)argv;
	char *upstream_uri = getenv(NRM_ENV_VAR_UPSTREAM_URI);
	char *upstream_rpc = getenv(NRM_ENV_VAR_UPSTREAM_RPC_PORT);
	char *upstream_pub = getenv(NRM_ENV_VAR_UPSTREAM_PUB_PORT);
	char *rate = getenv(NRM_ENV_VAR_RATELIMIT);
	char *transmit = getenv(NRM_ENV_VAR_TRANSMIT);
	char *timeout = getenv(NRM_ENV_VAR_TIMEOUT);
	int err;

	/* setup a default log config to handle errors in this part of the
	 * initialization
	 */
	if (!nrm_log_initialized)
		nrm_log_init(stderr, "libnrm");

	if (upstream_uri != NULL)
		nrm_upstream_uri = upstream_uri;
	if (upstream_rpc != NULL) {
		err = nrm_parse_uint(upstream_rpc, &nrm_upstream_rpc_port);
		if (err) {
			nrm_log_error("can't parse %s variable\n",
			              NRM_ENV_VAR_UPSTREAM_RPC_PORT);
			return err;
		}
	}

	if (upstream_pub != NULL) {
		err = nrm_parse_uint(upstream_pub, &nrm_upstream_pub_port);
		if (err) {
			nrm_log_error("can't parse %s variable\n",
			              NRM_ENV_VAR_UPSTREAM_PUB_PORT);
			return err;
		}
	}

	if (rate != NULL) {
		err = nrm_parse_llu(rate, &nrm_ratelimit);
		if (err) {
			nrm_log_error("can't parse %s variable\n",
			              NRM_ENV_VAR_RATELIMIT);
			return err;
		}
	}

	if (transmit != NULL) {
		err = nrm_parse_int(transmit, &nrm_transmit);
		if (err) {
			nrm_log_error("can't parse %s variable\n",
			              NRM_ENV_VAR_TRANSMIT);
			return err;
		}
	}

	if (timeout != NULL) {
		err = nrm_parse_uint(timeout, &nrm_timeout);
		if (err) {
			nrm_log_error("can't parse %s variable\n",
			              NRM_ENV_VAR_TIMEOUT);
			return err;
		}
	}

	/* disable signal handling by zmq */
	zsys_handler_set(NULL);
	return 0;
}

int nrm_finalize(void)
{
	return 0;
}
