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

int nrm_timeserie_create(nrm_timeserie_t **ts,
                         nrm_string_t sensor_uuid,
                         nrm_scope_t *scope)
{
	if (ts == NULL || sensor_uuid == NULL || scope == NULL)
		return -NRM_EINVAL;

	nrm_timeserie_t *t = calloc(1, sizeof(nrm_timeserie_t));
	if (t == NULL)
		return -NRM_ENOMEM;

	t->sensor_uuid = sensor_uuid;
	nrm_string_incref(sensor_uuid);
	t->scope = scope;
	t->start = nrm_time_fromns(0);

	int err = nrm_vector_create(&t->events, sizeof(nrm_event_t));
	if (err)
		goto error;

	*ts = t;
	return 0;
error:
	free(t);
	return err;
}

void nrm_timeserie_destroy(nrm_timeserie_t **ts)
{
	if (ts == NULL)
		return;

	nrm_timeserie_t *t = *ts;
	nrm_string_decref(t->sensor_uuid);
	nrm_vector_destroy(&t->events);
	free(t);
}

int nrm_timeserie_add_event(nrm_timeserie_t *ts, nrm_time_t time, double val)
{
	if (ts == NULL)
		return -NRM_EINVAL;

	nrm_event_t e;
	e.time = time;
	e.value = val;
	nrm_vector_push_back(ts->events, &e);

	if (nrm_time_tons(&ts->start) < nrm_time_tons(&time))
		ts->start = time;
	return 0;
}

int nrm_timeserie_add_events(nrm_timeserie_t *ts, nrm_vector_t *events)
{
	if (ts == NULL)
		return -NRM_EINVAL;

	nrm_vector_foreach(events, iter)
	{
		nrm_event_t *e = nrm_vector_iterator_get(iter);
		nrm_timeserie_add_event(ts, e->time, e->value);
	}
	return 0;
}

nrm_vector_t *nrm_timeserie_get_events(nrm_timeserie_t *ts)
{
	if (ts == NULL)
		return NULL;
	return ts->events;
}
