/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include "nrm.h"
#include "internal/nrmi.h"
#include "internal/roles.h"

void nrm_role_destroy(nrm_role_t **role)
{
	if (*role != NULL) {
		(*role)->ops->destroy(role);
	}
}
