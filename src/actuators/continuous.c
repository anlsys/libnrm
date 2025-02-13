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
#include <sched.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal/nrmi.h"
#include "internal/actuators.h"

/*******************************************************************************
 * Creation/Deletion
 *******************************************************************************/

nrm_actuator_t *nrm_actuator_continuous_create(const char *name)
{
	nrm_actuator_t *ret = calloc(1, sizeof(nrm_actuator_t));
	ret->data = calloc(1, sizeof(nrm_actuator_data_t));
	ret->data->uuid = nrm_string_fromchar(name);
	ret->data->type = NRM_ACTUATOR_TYPE_CONTINUOUS;
	ret->ops = &nrm_actuator_continuous_ops;
	return ret;
}

void nrm_actuator_continuous_destroy(nrm_actuator_t **actuator)
{
	if (actuator == NULL || *actuator == NULL)
		return;

	nrm_actuator_t *a = *actuator;
	nrm_string_decref(a->data->uuid);
	nrm_uuid_destroy(&a->data->clientid);
	free(a->data);
	free(a);
	*actuator = NULL;
}

/*******************************************************************************
 * Type-specific operations
 *******************************************************************************/

int nrm_actuator_continuous_set_limits(nrm_actuator_t *actuator,
                             double min, double max)
{
	if (actuator == NULL)
		return -NRM_EINVAL;
	actuator->data->u.limits[0] = min;
	actuator->data->u.limits[1] = max;
	return 0;
}

int nrm_actuator_continuous_corrected_value(nrm_actuator_t *actuator, double *value)
{
	if (actuator == NULL || value == NULL)
		return -NRM_EINVAL;

	double min = actuator->data->u.limits[0];
	double max = actuator->data->u.limits[1];

	if (*value < min) {
		*value = min;
		return 0;
	}

	if (*value > max) {
		*value = max;
		return 0;
	}
	return 0;
}

int nrm_actuator_continuous_validate_value(nrm_actuator_t *a, double value)
{
	if (a == NULL)
		return -NRM_EINVAL;
	if (value < a->data->u.limits[0] || value > a->data->u.limits[1])
		return -NRM_EDOM;
	return 0;	
}

struct nrm_actuator_ops_s nrm_actuator_continuous_ops = {
	nrm_actuator_continuous_validate_value,
	nrm_actuator_continuous_corrected_value,
	nrm_actuator_continuous_destroy,
};
