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
#include <errno.h>
#include <stdlib.h>

int nrm_parse_int(const char *str, int *p)
{
	int ret;
	errno = 0;
	ret = (int)strtoul(str, NULL, 10);
	if (errno != 0) {
		nrm_log_perror("error during conversion to int\n");
		return -NRM_EINVAL;
	}
	*p = (int)ret;
	return 0;
}

int nrm_parse_uint(const char *str, unsigned int *p)
{
	int ret;
	errno = 0;
	ret = (int)strtoul(str, NULL, 10);
	if (errno != 0) {
		nrm_log_perror("error during conversion to int\n");
		return -NRM_EINVAL;
	}
	if (ret < 0) {
		return -NRM_EDOM;
	}
	*p = (unsigned int)ret;
	return 0;
}

int nrm_parse_double(const char *str, double *p)
{
	double ret;
	errno = 0;
	char *endptr;
	ret = strtod(str, &endptr);
	if (errno != 0) {
		nrm_log_perror("error during conversion to double\n");
		return -NRM_EINVAL;
	}
	if (ret == 0 && endptr == str) {
		nrm_log_error("error during conversion to double\n");
		return -NRM_EINVAL;
	}
	*p = ret;
	return 0;
}
