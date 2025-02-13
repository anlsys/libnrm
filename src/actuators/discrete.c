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

nrm_actuator_t *nrm_actuator_discrete_create(const char *name)
{
	nrm_actuator_t *ret = calloc(1, sizeof(nrm_actuator_t));
	ret->data = calloc(1, sizeof(nrm_actuator_data_t));
	ret->data->uuid = nrm_string_fromchar(name);
	ret->data->type = NRM_ACTUATOR_TYPE_DISCRETE;
	nrm_vector_create(&ret->data->u.choices, sizeof(double));
	ret->ops = &nrm_actuator_discrete_ops;
	return ret;
}

void nrm_actuator_discrete_destroy(nrm_actuator_t **actuator)
{
	if (actuator == NULL || *actuator == NULL)
		return;

	nrm_actuator_t *a = *actuator;
	nrm_string_decref(a->data->uuid);
	nrm_uuid_destroy(&a->data->clientid);
	nrm_vector_destroy(&a->data->u.choices);
	free(a->data);
	free(a);
	*actuator = NULL;
}

/*******************************************************************************
 * Type-specific operations
 *******************************************************************************/

int nrm_actuator_discrete_set_choices(nrm_actuator_t *actuator,
                                      size_t nchoices,
                                      double *choices)
{
	if (actuator == NULL || nchoices == 0 || choices == NULL)
		return -NRM_EINVAL;
	nrm_vector_resize(actuator->data->u.choices, nchoices);
	nrm_vector_clear(actuator->data->u.choices);
	for (size_t i = 0; i < nchoices; i++)
		nrm_vector_push_back(actuator->data->u.choices, &choices[i]);
	nrm_vector_sort(actuator->data->u.choices, nrm_vector_sort_double_cmp);
	return 0;
}

int nrm_actuator_discrete_list_choices(nrm_actuator_t *actuator,
                                       nrm_vector_t **choices)
{
	if (actuator == NULL || choices == NULL)
		return -NRM_EINVAL;

	nrm_vector_t *ret;
	int err = nrm_vector_copy(&ret, actuator->data->u.choices);
	if (err)
		return err;

	*choices = ret;
	return NRM_SUCCESS;
}

int nrm_actuator_discrete_corrected_value(nrm_actuator_t *actuator,
                                          double *value)
{
	if (actuator == NULL || value == NULL)
		return -NRM_EINVAL;

	double *min, *max;
	size_t length;
	nrm_vector_length(actuator->data->u.choices, &length);
	if (length == 0)
		return -NRM_EINVAL;

	nrm_vector_get_withtype(double, actuator->data->u.choices, 0, min);
	nrm_vector_get_withtype(double, actuator->data->u.choices, length - 1,
	                        max);

	if (*value < *min) {
		*value = *min;
		return 0;
	}

	if (*value > *max) {
		*value = *max;
		return 0;
	}

	double dist = DBL_MAX;
	double ret = *value;
	nrm_vector_foreach(actuator->data->u.choices, iterator)
	{
		double *d = nrm_vector_iterator_get(iterator);
		if (fabs(*value - *d) < dist) {
			ret = *d;
			dist = fabs(*value - ret);
		}
	}
	*value = ret;
	return 0;
}

int nrm_actuator_discrete_validate_value(nrm_actuator_t *a, double value)
{
	size_t i, len = 0;
	nrm_vector_length(a->data->u.choices, &len);
	for (i = 0; i < len; i++) {
		double d;
		void *p;
		nrm_vector_get(a->data->u.choices, i, &p);
		d = *(double *)p;
		if (d == value)
			break;
	}
	if (i == len) {
		return -NRM_EDOM;
	}
	return 0;
}

struct nrm_actuator_ops_s nrm_actuator_discrete_ops = {
        nrm_actuator_discrete_validate_value,
        nrm_actuator_discrete_corrected_value,
        nrm_actuator_discrete_destroy,
};
