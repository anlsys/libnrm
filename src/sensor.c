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

nrm_sensor_t *nrm_sensor_create(const char *name)
{
	nrm_sensor_t *ret = calloc(1, sizeof(nrm_sensor_t));
	ret->uuid = nrm_string_fromchar(name);
	return ret;
}

nrm_string_t nrm_sensor_uuid(nrm_sensor_t *sensor)
{
	return sensor->uuid;
}

json_t *nrm_sensor_to_json(nrm_sensor_t *sensor)
{
	return json_pack("{s:s}", "uuid", sensor->uuid);
}

void nrm_sensor_destroy(nrm_sensor_t **sensor)
{
	if (sensor == NULL || *sensor == NULL)
		return;
	nrm_string_decref((*sensor)->uuid);
	free(*sensor);
	*sensor = NULL;
}
