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

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nrm.h"

#include "internal/nrmi.h"

nrm_slice_t *nrm_slice_create(const char *name)
{
	nrm_slice_t *ret = calloc(1, sizeof(nrm_slice_t));
	ret->uuid = nrm_string_fromchar(name);
	return ret;
}

json_t *nrm_slice_to_json(nrm_slice_t *slice)
{
	return json_pack("{s:s}", "uuid", slice->uuid);
}

void nrm_slice_destroy(nrm_slice_t **slice)
{
	if (slice == NULL || *slice == NULL)
		return;
	nrm_string_decref((*slice)->uuid);
	*slice = NULL;
}
