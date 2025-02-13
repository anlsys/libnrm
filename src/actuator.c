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

nrm_string_t nrm_actuator_uuid(nrm_actuator_t *actuator)
{
	assert(actuator != NULL);
	assert(actuator->data != NULL);
	return actuator->data->uuid;
}

nrm_uuid_t *nrm_actuator_clientid(nrm_actuator_t *actuator)
{
	assert(actuator != NULL);
	assert(actuator->data != NULL);
	return actuator->data->clientid;
}

int nrm_actuator_set_clientid(nrm_actuator_t *actuator, nrm_uuid_t *clientid)
{
	assert(actuator != NULL);
	assert(actuator->data != NULL);
	actuator->data->clientid = clientid;
	return 0;
}

double nrm_actuator_value(nrm_actuator_t *actuator)
{
	assert(actuator != NULL);
	assert(actuator->data != NULL);
	return actuator->data->value;
}

int nrm_actuator_set_value(nrm_actuator_t *actuator, double value)
{
	assert(actuator != NULL);
	assert(actuator->data != NULL);
	actuator->data->value = value;
	return 0;
}

void nrm_actuator_destroy(nrm_actuator_t **actuator)
{
	if (actuator == NULL || *actuator == NULL)
		return;

	(*actuator)->ops->destroy(actuator);
}

int nrm_actuator_validate_value(nrm_actuator_t *actuator, double value)
{
	assert(actuator != NULL);
	assert(actuator->data != NULL);
	assert(actuator->ops != NULL);
	return actuator->ops->validate_value(actuator, value);
}

int nrm_actuator_corrected_value(nrm_actuator_t *actuator, double *value)
{
	assert(actuator != NULL);
	assert(actuator->data != NULL);
	assert(actuator->ops != NULL);
	return actuator->ops->corrected_value(actuator, value);
}
	
/*******************************************************************************
 * JSON Converters
 *******************************************************************************/

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

struct nrm_actuator_type_table_s {
	int type;
	const char *s;
};

typedef struct nrm_actuator_type_table_s nrm_actuator_type_table_t;

/* clang-format off */
static const nrm_actuator_type_table_t nrm_actuator_type_table[] = {
	{NRM_ACTUATOR_TYPE_DISCRETE, "DISCRETE"},
	{NRM_ACTUATOR_TYPE_CONTINUOUS, "CONTINUOUS"},
};
/* clang-format on */

const char *nrm_actuator_type_t2s(int type)
{
	if (type < 0 || type > NRM_ACTUATOR_TYPE_MAX)
		return "UNKNOWN";
	for (int i = 0; i < NRM_ACTUATOR_TYPE_MAX; i++)
		if (nrm_actuator_type_table[i].type == type)
			return nrm_actuator_type_table[i].s;
	return "UNKNOWN";
}

int nrm_actuator_type_s2t(const char *string)
{
	for (int i = 0; i < NRM_ACTUATOR_TYPE_MAX; i++)
		if (!strcmp(string, nrm_actuator_type_table[i].s))
			return i;
	return -1;
}

json_t *nrm_actuator_discrete_to_json(nrm_actuator_t *actuator)
{
	json_t *ret;
	json_t *choices = nrm_vector_d_to_json(actuator->data->u.choices);
	ret = json_pack("{s:o?}", "choices", choices);
	return ret;
}

json_t *nrm_actuator_to_json(nrm_actuator_t *actuator)
{
	json_t *ret;
	json_t *sub;
	switch (actuator->data->type) {
	case NRM_ACTUATOR_TYPE_DISCRETE:
		sub = nrm_actuator_discrete_to_json(actuator);
		break;
	default:
		sub = NULL;
	}
	json_t *clientid = NULL;
	clientid = nrm_uuid_to_json(actuator->data->clientid);
	ret = json_pack("{s:s, s:o?, s:f, s:s, s:o?}", "uuid", actuator->data->uuid,
	                 "clientid", clientid, "value", actuator->data->value,
			 "type", nrm_actuator_type_t2s(actuator->data->type),
	                 "subtype", sub);
	return ret;
}

int nrm_actuator_discrete_from_json(nrm_actuator_t *actuator, json_t *json)
{
	json_t *choices = NULL;
	json_error_t error;
	int err;
	err = json_unpack_ex(json, &error, 0, "{s?:o}", "choices", &choices);
	if (err) {
		nrm_log_error("error unpacking json: %s, %s, %d, %d, %d\n",
		              error.text, error.source, error.line,
		              error.column, error.position);
		return -NRM_EINVAL;
	}
	nrm_vector_d_from_json(actuator->data->u.choices, choices);
	return 0;
}

int nrm_actuator_from_json(nrm_actuator_t *actuator, json_t *json)
{
	json_t *sub = NULL;
	char *uuid = NULL;
	char *clientid = NULL;
	char *type;
	json_error_t error;
	int err;
	err = json_unpack_ex(json, &error, 0, "{s:s, s?:o, s:s, s?:f, s?:o}", "uuid",
	                     &uuid, "clientid", &clientid, "type", &type,
	                     "value", &actuator->data->value,
			     "subtype", &sub);
	if (err) {
		nrm_log_error("error unpacking json: %s, %s, %d, %d, %d\n",
		              error.text, error.source, error.line,
		              error.column, error.position);
		return -NRM_EINVAL;
	}
	actuator->data->uuid = nrm_string_fromchar(uuid);
	if (clientid)
		actuator->data->clientid = nrm_uuid_create_fromchar(clientid);
	actuator->data->type = nrm_actuator_type_s2t(type);
	switch (actuator->data->type) {
	case NRM_ACTUATOR_TYPE_DISCRETE:
		return nrm_actuator_discrete_from_json(actuator, sub);
	default:
		return -NRM_EINVAL;
	}
}
