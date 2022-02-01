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

#include "nrm-internal.h"

const int nrm_version_major = NRM_VERSION_MAJOR;
const int nrm_version_minor = NRM_VERSION_MINOR;
const int nrm_version_patch = NRM_VERSION_PATCH;
const char *nrm_version_revision = NRM_VERSION_REVISION;
const char *nrm_version_string = NRM_VERSION_STRING;

long long int nrm_ratelimit_threshold;
int nrm_transmit;

int nrm_init(int *argc, char **argv[])
{
	(void) argc;
	(void) argv;
	const char *rate = getenv(NRM_ENV_RATELIMIT);
	const char *transmit = getenv(NRM_ENV_TRANSMIT);
	if (rate == NULL)
		nrm_ratelimit_threshold = NRM_DEFAULT_RATELIMIT_THRESHOLD;
	else {
		/* see strtoul(3) for details. */
		errno = 0;
		nrm_ratelimit_threshold = strtoull(rate, NULL, 10);
		assert(errno == 0);
	}
	if (transmit != NULL)
		nrm_transmit = atoi(transmit);
	else
		nrm_transmit = 1;

	/* context init */
	if (nrm_ratelimit_threshold <= 0)
		return -1;
	return 0;
}

int nrm_finalize(void)
{
	return 0;
}

