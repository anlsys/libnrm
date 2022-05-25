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

#include <stdio.h>

#include "nrm.h"

static const char *const nrm_error_strings[] = {
        [NRM_SUCCESS] = "Success",
        [NRM_FAILURE] = "Generic error",
        [NRM_ENOMEM] = "Not enough memory",
        [NRM_EINVAL] = "Invalid argument",
        [NRM_EDOM] = "Value out of bound",
        [NRM_EBUSY] = "Underlying resource is not available for operation",
        [NRM_ENOTSUP] = "Operation not supported",
        [NRM_EPERM] = "Insufficient permissions",
};

const char *nrm_strerror(const int err)
{
	if (err < 0 || err >= NRM_ERROR_MAX)
		return "Unknown error";
	return nrm_error_strings[err];
}

void nrm_perror(const char *msg)
{
	fprintf(stderr, "%s:%s\n", msg, nrm_strerror(nrm_errno));
}
