/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#include "config.h"

#include "nrm.h"

void nrm_time_gettime(nrm_time_t *now)
{
	clock_gettime(CLOCK_REALTIME, now);
}

int64_t nrm_time_diff(const nrm_time_t *start, const nrm_time_t *end)
{
	int64_t timediff = (end->tv_nsec - start->tv_nsec) +
					1000000000 * (end->tv_sec -
						   start->tv_sec);
	return timediff;
}

int64_t nrm_time_tons(const nrm_time_t *now)
{
	return now->tv_nsec + 1000000000 * now->tv_sec;
}
