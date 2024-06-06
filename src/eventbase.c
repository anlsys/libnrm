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

#include "internal/uthash.h"
#include "internal/utlist.h"

#define TIMESLICE_PERIOD 1000

/* a slice of events, indexed by the start of the slice (a multiple of
 * TIMESLICE_PERIOD nanoseconds
 */
struct nrm_eb_timeslice_s {
	int64_t key;
	nrm_vector_t *events;
	UT_hash_handle hh;
};
typedef struct nrm_eb_timeslice_s nrm_eb_timeslice_t;

/* all the slices for a given scope */
/* TODO maybe we could store them in a ringbuffer and just use the hash for
 * TODO indexing faster? or just index faster into the ringbuffer.
 */
struct nrm_eb_scopebase_s {
	nrm_string_t uuid;
	nrm_eb_timeslice_t *slices;
};
typedef struct nrm_eb_scopebase_s nrm_eb_scopebase_t;

struct nrm_eb_sensorbase_s {
	nrm_string_t uuid;
	nrm_hash_t *scopes;
};
typedef struct nrm_eb_sensorbase_s nrm_eb_sensorbase_t;

/* the typedef is already in nrm.h */
struct nrm_eventbase_s {
	size_t maxperiods;
	nrm_hash_t *sensors;
};

/******************************************************************************
 * Key utils
 ******************************************************************************/

int64_t nrm_eb_time2key(nrm_time_t t)
{
	int64_t key = nrm_time_tons(&t);
	return key - (key % TIMESLICE_PERIOD);
}

/*******************************************************************************
 * Basic Functions
 ******************************************************************************/

nrm_eventbase_t *nrm_eventbase_create(size_t maxperiods)
{
	nrm_eventbase_t *ret = calloc(1, sizeof(nrm_state_t));
	if (ret == NULL)
		return NULL;
	ret->maxperiods = maxperiods;
	ret->sensors = NULL;
	return ret;
}

size_t nrm_eventbase_get_maxperiods(nrm_eventbase_t *eb)
{
	return eb->maxperiods;
}

void nrm_eventbase_destroy(nrm_eventbase_t **eventbase)
{
	if (eventbase == NULL || *eventbase == NULL)
		return;
	nrm_eventbase_t *eb = *eventbase;

	nrm_hash_foreach(eb->sensors, isb)
	{
		nrm_eb_sensorbase_t *sb = nrm_hash_iterator_get(isb);
		nrm_hash_foreach(sb->scopes, isc)
		{
			nrm_eb_scopebase_t *sc = nrm_hash_iterator_get(isc);
			nrm_eb_timeslice_t *ts, *tstmp;
			HASH_ITER(hh, sc->slices, ts, tstmp)
			{
				nrm_vector_destroy(&ts->events);
				HASH_DEL(sc->slices, ts);
				free(ts);
			}
			HASH_CLEAR(hh, sc->slices);
			nrm_string_decref(sc->uuid);
			free(sc);
		}
		nrm_hash_destroy(&sb->scopes);
		nrm_string_decref(sb->uuid);
		free(sb);
	}
	nrm_hash_destroy(&eb->sensors);
	free(eb);
	*eventbase = NULL;
}

/*******************************************************************************
 * Pushing events: we push individual events, or entire timeseries.
 ******************************************************************************/

int nrm_eventbase_add_event(nrm_eb_timeslice_t *ts, nrm_time_t time, double val)
{
	nrm_event_t e;
	e.time = time;
	e.value = val;
	nrm_vector_push_back(ts->events, &e);
	return 0;
}

nrm_eb_timeslice_t *nrm_eventbase_add_timeslice(nrm_eb_scopebase_t *sc,
                                                int64_t key)
{
	nrm_eb_timeslice_t *ret;
	ret = calloc(1, sizeof(nrm_eb_timeslice_t));
	if (ret == NULL)
		return NULL;

	ret->key = key;
	nrm_vector_create(&ret->events, sizeof(nrm_event_t));
	HASH_ADD(hh, sc->slices, key, sizeof(int64_t), ret);
	return ret;
}

nrm_eb_scopebase_t *nrm_eventbase_add_scope(nrm_eb_sensorbase_t *sb,
                                            nrm_scope_t *scope)
{
	nrm_eb_scopebase_t *ret;
	ret = calloc(1, sizeof(nrm_eb_scopebase_t));
	if (ret == NULL)
		return NULL;

	ret->uuid = nrm_scope_uuid(scope);
	nrm_string_incref(ret->uuid);
	nrm_hash_add(&sb->scopes, ret->uuid, ret);
	return ret;
}

nrm_eb_sensorbase_t *nrm_eventbase_add_sensor(nrm_eventbase_t *eb,
                                              nrm_string_t sensor_uuid)
{
	nrm_eb_sensorbase_t *ret;
	ret = calloc(1, sizeof(nrm_eb_sensorbase_t));
	if (ret == NULL)
		return NULL;

	ret->uuid = sensor_uuid;
	nrm_string_incref(ret->uuid);
	nrm_hash_add(&eb->sensors, ret->uuid, ret);
	return ret;
}

int nrm_eventbase_push_event(nrm_eventbase_t *eb,
                             nrm_string_t sensor_uuid,
                             nrm_scope_t *scope,
                             nrm_time_t time,
                             double value)
{
	int err;
	if (eb == NULL || scope == NULL)
		return -NRM_EINVAL;

	nrm_eb_sensorbase_t *sb;
	if (eb->sensors == NULL)
		sb = nrm_eventbase_add_sensor(eb, sensor_uuid);
	else {
		err = nrm_hash_find(eb->sensors, sensor_uuid, (void *)&sb);
		if (err)
			sb = nrm_eventbase_add_sensor(eb, sensor_uuid);
	}

	nrm_eb_scopebase_t *sc;
	if (sb->scopes == NULL)
		sc = nrm_eventbase_add_scope(sb, scope);
	else {
		err = nrm_hash_find(sb->scopes, scope->uuid, (void *)&sc);
		if (err)
			sc = nrm_eventbase_add_scope(sb, scope);
	}

	/* convert time to key */
	int64_t key = nrm_time_tons(&time);
	key = key - (key % TIMESLICE_PERIOD);

	nrm_eb_timeslice_t *ts;
	if (sc->slices == NULL)
		ts = nrm_eventbase_add_timeslice(sc, key);
	else {
		HASH_FIND(hh, sc->slices, &key, sizeof(key), ts);
		if (ts == NULL)
			ts = nrm_eventbase_add_timeslice(sc, key);
	}

	nrm_eventbase_add_event(ts, time, value);
	return 0;
}

/*******************************************************************************
 * Pulling events: we pull entire timeseries
 ******************************************************************************/

int nrm_eventbase_pull_timeserie(nrm_eventbase_t *eb,
                                 nrm_string_t sensor_uuid,
                                 nrm_scope_t *scope,
                                 nrm_time_t since,
                                 nrm_timeserie_t **ts)
{
	if (eb == NULL)
		return -NRM_EINVAL;

	/* event if we can't find any data, we'll return an initialized but
	 * empty timeserie.
	 */
	nrm_timeserie_t *ret;
	nrm_timeserie_create(&ret, sensor_uuid, scope);

	nrm_eb_sensorbase_t *sb;
	nrm_hash_find(eb->sensors, sensor_uuid, (void *)&sb);
	if (sb == NULL)
		goto end;

	nrm_eb_scopebase_t *sc;
	nrm_hash_find(sb->scopes, scope->uuid, (void *)&sc);
	if (sc == NULL)
		goto end;

	/* the hash table is there to speed up receiving events, as we expect
	 * pulling events happens less often.
	 * We also expect some kind of "expiration policy" for events to exist,
	 * removing extra timeslices on a regular basis.
	 *
	 * So, yes, iterating over keys in not the efficient thing, but in
	 * practice this hash table should be small.
	 */
	nrm_time_t now;
	nrm_time_gettime(&now);
	int64_t ksince = nrm_eb_time2key(since);
	int64_t know = nrm_eb_time2key(now);

	nrm_eb_timeslice_t *tl, *tltmp;
	HASH_ITER(hh, sc->slices, tl, tltmp)
	{
		if (tl->key >= ksince && tl->key < know)
			nrm_timeserie_add_events(ret, tl->events);
	}
end:
	*ts = ret;
	return 0;
}

/******************************************************************************
 * State management
 ******************************************************************************/

int nrm_eventbase_tick(nrm_eventbase_t *eb, nrm_time_t time)
{
	/* should eventually remove old events from the database */
	(void)eb;
	(void)time;
	return 0;
}
