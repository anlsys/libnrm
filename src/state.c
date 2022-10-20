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
		nrm_scope_destroy(&a);
	}
	nrm_hash_destroy(&s->scopes);

	free(s);
	*state = NULL;
}
