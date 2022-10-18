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
	nrm_vector_destroy(&s->actuators);
	nrm_vector_destroy(&s->slices);
	nrm_vector_destroy(&s->sensors);
	nrm_vector_destroy(&s->scopes);
	*state = NULL;
}
