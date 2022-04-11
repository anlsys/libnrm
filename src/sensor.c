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


nrm_sensor_t *nrm_sensor_create(char *name)
{
	nrm_sensor_t *ret = calloc(1, sizeof(nrm_sensor_t));
	ret->name = nrm_string_fromchar(name);
	ret->uuid = NULL;
	return ret;
}

json_t *nrm_sensor_to_json(nrm_sensor_t *sensor)
{
	json_t *uuid = NULL;

	if (sensor->uuid != NULL)
		uuid = nrm_uuid_to_json(sensor->uuid);
	return json_pack("{s:s, s:o*}",
			 "name", sensor->name, "uuid", uuid);
}

void nrm_sensor_destroy(nrm_sensor_t **sensor)
{
	if (sensor == NULL || *sensor == NULL)
		return;
	nrm_string_decref(&(*sensor)->name);
	nrm_uuid_destroy(&(*sensor)->uuid);
	*sensor = NULL;
}
