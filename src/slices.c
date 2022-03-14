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


nrm_slice_t *nrm_slice_create(char *name)
{
	nrm_slice_t *ret = calloc(1, sizeof(nrm_slice_t));
	ret->name = name;
	ret->uuid = NULL;
	return ret;
}

json_t *nrm_slice_to_json(nrm_slice_t *slice)
{
	json_t *uuid = NULL;

	if (slice->uuid != NULL)
		uuid = nrm_uuid_to_json(slice->uuid);
	return json_pack("{s:s, s:o*}",
			 "name", slice->name, "uuid", uuid);
}

void nrm_slice_destroy(nrm_slice_t **slice)
{
	if (slice == NULL || *slice == NULL)
		return;
	free((*slice)->name);
	nrm_uuid_destroy(&(*slice)->uuid);
	*slice = NULL;
}
