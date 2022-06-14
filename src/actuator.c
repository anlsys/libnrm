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

nrm_actuator_t *nrm_actuator_create(char *name)
{
	nrm_actuator_t *ret = calloc(1, sizeof(nrm_actuator_t));
	ret->name = nrm_string_fromchar(name);
	ret->uuid = NULL;
	return ret;
}

json_t *nrm_actuator_to_json(nrm_actuator_t *actuator)
{
	json_t *uuid = NULL;

	if (actuator->uuid != NULL)
		uuid = nrm_uuid_to_json(actuator->uuid);
	return json_pack("{s:s, s:o*}", "name", actuator->name, "uuid", uuid);
}

void nrm_actuator_destroy(nrm_actuator_t **actuator)
{
	if (actuator == NULL || *actuator == NULL)
		return;
	nrm_string_decref(&(*actuator)->name);
	nrm_uuid_destroy(&(*actuator)->uuid);
	*actuator = NULL;
}
