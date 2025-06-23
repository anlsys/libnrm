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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal/nrmi.h"

nrm_state_t *nrm_state_create()
{
	nrm_state_t *ret = calloc(1, sizeof(nrm_state_t));
	return ret;
}

int nrm_state_remove_actuator(nrm_state_t *state, const char *uuid)
{
	nrm_actuator_t *actuator = NULL;
	nrm_string_t id = nrm_string_fromchar(uuid);
	nrm_hash_remove(&state->actuators, id, (void *)&actuator);
	nrm_actuator_destroy(&actuator);
	nrm_string_decref(id);
	return 0;
}

int nrm_state_remove_scope(nrm_state_t *state, const char *uuid)
{
	nrm_scope_t *scope = NULL;
	nrm_string_t id = nrm_string_fromchar(uuid);
	nrm_hash_remove(&state->scopes, id, (void *)&scope);
	nrm_scope_destroy(scope);
	nrm_string_decref(id);
	return 0;
}

int nrm_state_remove_sensor(nrm_state_t *state, const char *uuid)
{
	nrm_sensor_t *sensor = NULL;
	nrm_string_t id = nrm_string_fromchar(uuid);
	nrm_hash_remove(&state->sensors, id, (void *)&sensor);
	nrm_sensor_destroy(&sensor);
	nrm_string_decref(id);
	return 0;
}

int nrm_state_remove_slice(nrm_state_t *state, const char *uuid)
{
	nrm_slice_t *slice = NULL;
	nrm_string_t id = nrm_string_fromchar(uuid);
	nrm_hash_remove(&state->slices, id, (void *)&slice);
	nrm_slice_destroy(&slice);
	nrm_string_decref(id);
	return 0;
}

int nrm_state_list_actuators(nrm_state_t *state, nrm_vector_t *vec)
{
	nrm_hash_foreach(state->actuators, iter)
	{
		nrm_actuator_t *actuator = nrm_hash_iterator_get(iter);
		nrm_vector_push_back(vec, actuator);
	}
	return 0;
}

int nrm_state_list_scopes(nrm_state_t *state, nrm_vector_t *vec)
{
	nrm_hash_foreach(state->scopes, iter)
	{
		nrm_scope_t *scope = nrm_hash_iterator_get(iter);
		nrm_vector_push_back(vec, scope);
	}
	return 0;
}

int nrm_state_list_sensors(nrm_state_t *state, nrm_vector_t *vec)
{
	nrm_hash_foreach(state->sensors, iter)
	{
		nrm_sensor_t *sensor = nrm_hash_iterator_get(iter);
		nrm_vector_push_back(vec, sensor);
	}
	return 0;
}

int nrm_state_list_slices(nrm_state_t *state, nrm_vector_t *vec)
{
	nrm_hash_foreach(state->slices, iter)
	{
		nrm_slice_t *slice = nrm_hash_iterator_get(iter);
		nrm_vector_push_back(vec, slice);
	}
	return 0;
}

int nrm_state_add_actuator(nrm_state_t *state, nrm_actuator_t *actuator)
{
	nrm_hash_add(&state->actuators, nrm_actuator_uuid(actuator), actuator);
	return 0;
}

int nrm_state_add_scope(nrm_state_t *state, nrm_scope_t *scope)
{
	nrm_hash_add(&state->scopes, scope->uuid, scope);
	return 0;
}

int nrm_state_add_sensor(nrm_state_t *state, nrm_sensor_t *sensor)
{
	nrm_hash_add(&state->sensors, sensor->uuid, sensor);
	return 0;
}

int nrm_state_add_slice(nrm_state_t *state, nrm_slice_t *slice)
{
	nrm_hash_add(&state->slices, slice->uuid, slice);
	return 0;
}

void nrm_state_destroy(nrm_state_t **state)
{
	if (state == NULL || *state == NULL)
		return;
	nrm_state_t *s = *state;

	nrm_hash_foreach(s->actuators, iter)
	{
		nrm_actuator_t *a = nrm_hash_iterator_get(iter);
		nrm_actuator_destroy(&a);
	}
	nrm_hash_destroy(&s->actuators);

	nrm_hash_foreach(s->sensors, iter)
	{
		nrm_sensor_t *a = nrm_hash_iterator_get(iter);
		nrm_sensor_destroy(&a);
	}
	nrm_hash_destroy(&s->sensors);

	nrm_hash_foreach(s->slices, iter)
	{
		nrm_slice_t *a = nrm_hash_iterator_get(iter);
		nrm_slice_destroy(&a);
	}
	nrm_hash_destroy(&s->slices);

	nrm_hash_foreach(s->scopes, iter)
	{
		nrm_scope_t *a = nrm_hash_iterator_get(iter);
		nrm_scope_destroy(a);
	}
	nrm_hash_destroy(&s->scopes);

	free(s);
	*state = NULL;
}
