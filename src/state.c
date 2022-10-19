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
	nrm_hash_iterator_t *iter = NULL;
	nrm_hash_iterator_create(&iter);
	nrm_hash_t *array_of_hashes[4] = {s->actuators, s->slices, s->sensors,
	                                  s->scopes};
	for (int i = 0; i < 4; i++) {
		nrm_hash_iter(array_of_hashes[i], iter, nrm_actuator_t)
		{
			nrm_string_t uuid = nrm_hash_iterator_get_uuid(iter);
			nrm_actuator_t *ptr = nrm_hash_iterator_get(iter);
			nrm_hash_remove(&array_of_hashes[i], uuid,
			                (void *)&ptr);
		}
		nrm_hash_destroy(&array_of_hashes[i]);
	}

	// nrm_hash_destroy(&s->actuators);
	// nrm_hash_destroy(&s->slices);
	// nrm_hash_destroy(&s->sensors);
	// nrm_hash_destroy(&s->scopes);
	*state = NULL;
}
