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

int nrm_actuator_set_choices(nrm_actuator_t *actuator, size_t nchoices, double
			     *choices)
{
	if (actuator == NULL || nchoices == 0 || choices == NULL)
		return -NRM_EINVAL;
	nrm_vector_resize(actuator->choices, nchoices);
	for (size_t i = 0; i < nchoices; i++)
		nrm_vector_push_back(actuator->choices, &choices[i]);
	return 0;
}

json_t *nrm_vector_d_to_json(nrm_vector_t *vector)
{
	json_t *ret;
	ret = json_array();
	size_t nitems;
	nrm_vector_length(vector, &nitems);
	for (size_t i = 0; i < nitems; i++) {
		double *d;
		void *p;
		nrm_vector_get(vector, i, &p);
		d = (double *)p;
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
	json_t *uuid = NULL;
	json_t *choices = NULL;
	if (actuator->uuid != NULL)
		uuid = nrm_uuid_to_json(actuator->uuid);
	choices = nrm_vector_d_to_json(actuator->choices);
	return json_pack("{s:s, s:o?, s:f, s:o?}", "name", actuator->name, 
			 "uuid", uuid, "value", actuator->value,
			 "choices", choices);
}

int nrm_actuator_from_json(nrm_actuator_t *actuator, json_t *json)
{
	json_t *choices = NULL;
	char *uuid = NULL;
	json_error_t error;
	int err;
	err = json_unpack_ex(json, &error, 0, "{s?:s, s?:o, s?:f}",
			     "uuid", &uuid, "choices", &choices,
			     "value", &actuator->value);
	if (err) {
		nrm_log_error("error unpacking json: %s, %s, %d, %d, %d\n",
		              error.text, error.source, error.line,
		              error.column, error.position);
		return -NRM_EINVAL;
	}
	if (uuid)
		actuator->uuid = nrm_uuid_create_fromchar(uuid);
	nrm_vector_d_from_json(actuator->choices, choices);
	return 0;
}

void nrm_actuator_destroy(nrm_actuator_t **actuator)
{
	if (actuator == NULL || *actuator == NULL)
		return;
	nrm_string_decref(&(*actuator)->name);
	nrm_uuid_destroy(&(*actuator)->uuid);
	nrm_vector_destroy(&(*actuator)->choices);
	*actuator = NULL;
}
