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

nrm_actuator_t *nrm_actuator_create(const char *name)
{
	nrm_actuator_t *ret = calloc(1, sizeof(nrm_actuator_t));
	ret->uuid = nrm_string_fromchar(name);
	nrm_vector_create(&ret->choices, sizeof(double));
	return ret;
}

int nrm_actuator_set_value(nrm_actuator_t *actuator, double value)
{
	if (actuator == NULL)
		return -NRM_EINVAL;
	actuator->value = value;
	return 0;
}

int nrm_actuator_set_choices(nrm_actuator_t *actuator,
                             size_t nchoices,
                             double *choices)
{
	if (actuator == NULL || nchoices == 0 || choices == NULL)
		return -NRM_EINVAL;
	nrm_vector_resize(actuator->choices, nchoices);
	nrm_vector_clear(actuator->choices);
	for (size_t i = 0; i < nchoices; i++)
		nrm_vector_push_back(actuator->choices, &choices[i]);
	nrm_vector_sort(actuator->choices, nrm_vector_sort_double_cmp);
	return 0;
}

nrm_string_t nrm_actuator_uuid(nrm_actuator_t *actuator)
{
	return actuator->uuid;
}

nrm_uuid_t *nrm_actuator_clientid(nrm_actuator_t *actuator)
{
	return actuator->clientid;
}

double nrm_actuator_value(nrm_actuator_t *actuator)
{
	return actuator->value;
}

int nrm_actuator_list_choices(nrm_actuator_t *actuator, nrm_vector_t **choices)
{
	if (actuator == NULL || choices == NULL)
		return -NRM_EINVAL;

	nrm_vector_t *ret;
	int err = nrm_vector_copy(&ret, actuator->choices);
	if (err)
		return err;

	*choices = ret;
	return NRM_SUCCESS;
}

int nrm_actuator_closest_choice(nrm_actuator_t *actuator, double *value)
{
	if (actuator == NULL || value == NULL)
		return -NRM_EINVAL;

	double *min, *max;
	size_t length;
	nrm_vector_length(actuator->choices, &length);
	if (length == 0)
		return -NRM_EINVAL;

	nrm_vector_get_withtype(double, actuator->choices, 0, min);
	nrm_vector_get_withtype(double, actuator->choices, length - 1, max);

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
	nrm_vector_foreach(actuator->choices, iterator)
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

json_t *nrm_vector_d_to_json(nrm_vector_t *vector)
{
	json_t *ret;
	ret = json_array();
	nrm_vector_foreach(vector, iterator)
	{
		double *d = nrm_vector_iterator_get(iterator);
		json_array_append_new(ret, json_real(*d));
	}
	return ret;
}

int nrm_vector_d_from_json(nrm_vector_t *vector, json_t *json)
{
	if (!json_is_array(json))
		return -NRM_EINVAL;
	size_t length = json_array_size(json);
	nrm_vector_resize(vector, length);
	nrm_vector_clear(vector);
	size_t index;
	json_t *value;
	json_array_foreach(json, index, value)
	{
		double d = json_real_value(value);
		nrm_vector_push_back(vector, &d);
	}
	return 0;
}

json_t *nrm_actuator_to_json(nrm_actuator_t *actuator)
{
	json_t *clientid = NULL;
	json_t *choices = NULL;
	clientid = nrm_uuid_to_json(actuator->clientid);
	choices = nrm_vector_d_to_json(actuator->choices);
	return json_pack("{s:s, s:o?, s:f, s:o?}", "uuid", actuator->uuid,
	                 "clientid", clientid, "value", actuator->value,
	                 "choices", choices);
}

int nrm_actuator_from_json(nrm_actuator_t *actuator, json_t *json)
{
	json_t *choices = NULL;
	char *uuid = NULL;
	char *clientid = NULL;
	json_error_t error;
	int err;
	err = json_unpack_ex(json, &error, 0, "{s:s, s?:o, s?:o, s?:f}", "uuid",
	                     &uuid, "clientid", &clientid, "choices", &choices,
	                     "value", &actuator->value);
	if (err) {
		nrm_log_error("error unpacking json: %s, %s, %d, %d, %d\n",
		              error.text, error.source, error.line,
		              error.column, error.position);
		return -NRM_EINVAL;
	}
	actuator->uuid = nrm_string_fromchar(uuid);
	if (clientid)
		actuator->clientid = nrm_uuid_create_fromchar(clientid);
	nrm_vector_d_from_json(actuator->choices, choices);
	return 0;
}

void nrm_actuator_destroy(nrm_actuator_t **actuator)
{
	if (actuator == NULL || *actuator == NULL)
		return;

	nrm_actuator_t *a = *actuator;
	nrm_string_decref(a->uuid);
	nrm_uuid_destroy(&a->clientid);
	nrm_vector_destroy(&a->choices);
	free(a);
	*actuator = NULL;
}
