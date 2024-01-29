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
	nrm_scope_t *scope;
	nrm_eb_timeslice_t *slices;
};
typedef struct nrm_eb_scopebase_s nrm_eb_scopebase_t;

struct nrm_eb_sensorbase_s {
	nrm_string_t sensor_uuid;
	nrm_hash_t *scopes;
};
typedef struct nrm_eb_sensorbase_s nrm_eb_sensorbase_t;

/* the typedef is already in nrm.h */
struct nrm_eventbase_s {
	size_t maxperiods;
	nrm_hash_t *sensors;
};

/*******************************************************************************
 * Basic Functions
 ******************************************************************************/


nrm_eventbase_t *nrm_eventbase_create(size_t maxperiods)
{
	nrm_eventbase_t *ret = calloc(1, sizeof(nrm_state_t));
	if (ret == NULL)
		return NULL;
	ret->maxperiods = maxperiods;
	ret->hash = NULL;
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

	nrm_hash_foreach(eb->hash, isb)
	{
		nrm_eb_sensorbase_t *sb = nrm_hash_iterator_get(isb);
		nrm_hash_foreach(sb->scopes, isc)
		{
			nrm_eb_scopebase_t *sc = nrm_hash_iterator_get(isc);
			HASH_ITER(hh, sc->slices, ts, tstmp)
			{
				nrm_vector_destroy(&ts->events);
				HASH_DEL(sc->slices, ts);
				free(ts);
			}
			HASH_CLEAR(hh, sc->slices);
			free(sc);
		}
		nrm_hash_destroy(&sb->scopes);
		free(sb);
	}
	nrm_hash_destroy(&eb->hash);
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
	nrm_vector_create(ret->events, sizeof(nrm_event_t));
	HASH_ADD(hh, sc->slices, key, sizeof(int64_t), ret);
	return ret;
}

nrm_eb_scopebase_t *nrm_eventbase_add_scope(nrm_eventbase_t *eb,
					    nrm_eb_sensorbase_t *sb,
					    nrm_scope_t *scope)
{
	struct nrm_eb_scopebase_t *ret;
	ret = calloc(1, sizeof(struct nrm_eb_scopebase_t));
	if (ret == NULL)
		return NULL;

	ret->scope = nrm_scope_dup(scope);
	nrm_hash_add(sb->scopes, scope->uuid, ret);
	return ret;
}

nrm_eb_sensorbase_t *nrm_eventbase_add_sensor(nrm_eventbase_t *eb,
                                                    nrm_string_t sensor_uuid)
{
	struct nrm_eb_sensorbase_t *ret;
	ret = calloc(1, sizeof(struct nrm_eb_sensorbase_t));
	if (ret == NULL)
		return NULL;
	
	ret->uuid = sensor_uuid;
	nrm_string_incref(sensor_uuid);
	nrm_hash_add(eb->hash, ret->uuid, ret);
	return ret;
}

int nrm_eventbase_push_event(nrm_eventbase_t *eb,
                             nrm_string_t sensor_uuid,
                             nrm_scope_t *scope,
                             nrm_time_t time,
                             double value)
{
	if (eb == NULL || scope == NULL)
		return -NRM_EINVAL;

	nrm_eb_sensorbase_t *sb;
	nrm_hash_find(eb->hash, sensor_uuid, &sb);
	if (sb == NULL)
		sb = nrm_eventbase_add_sensor(eb, sensor_uuid);
	
	nrm_eb_scopebase_t *sc;
	nrm_hash_find(sb->scopes, scope->uuid, &sc);
	if (sc == NULL)
		sc = nrm_eventbase_add_scope(eb, sb, scope);

	/* convert time to key */
	int64_t key = nrm_time_tons(&time);
	key = key - (key % TIMESLICE_PERIOD);

	nrm_eb_timeslice_t *ts;
	HASH_FIND(hh, sc->slices, &key, sizeof(key), &ts);
	if (ts == NULL)
		ts = nrm_eventbase_add_timeslice(eb, sb, sc, key);

	nrm_eventbase_add_event(ts, time, value);
	return 0;
}

/*******************************************************************************
 * Pulling events: we pull entire timeseries
 ******************************************************************************/

int nrm_eventbase_new_period(struct nrm_scope2ring_s *s, nrm_time_t time)
{
	nrm_event_t period;
	period.time = time;
	period.value = 0.0;
	nrm_vector_foreach(s->events, iterator)
	{
		nrm_event_t *f = nrm_vector_iterator_get(iterator);
		period.value += f->value;
	}
	nrm_vector_clear(s->events);
	nrm_ringbuffer_push_back(s->past, &period);
	nrm_log_debug("period value: %f\n", period.value);
	return 0;
}

int nrm_eventbase_tick(nrm_eventbase_t *eb, nrm_time_t time)
{
	struct nrm_sensor2scope_s *s2s;

	for (s2s = eb->hash; s2s != NULL; s2s = s2s->hh.next) {
		struct nrm_scope2ring_s *s2r;
		DL_FOREACH(s2s->list, s2r)
		{
			nrm_log_debug("new period: %s:%s\n", s2s->uuid,
			              s2r->scope->uuid);
			nrm_eventbase_new_period(s2r, time);
		}
	}
	return 0;
}

/*******************************************************************************
 * State management
 ******************************************************************************/

int nrm_eventbase_remove_sensor(nrm_eventbase_t *eb, nrm_string_t sensor_uuid)
{
	struct nrm_sensor2scope_s *s;
	HASH_FIND(hh, eb->hash, sensor_uuid, nrm_string_strlen(sensor_uuid), s);
	if (s != NULL) {
		struct nrm_scope2ring_s *elt, *tmp;
		DL_FOREACH_SAFE(s->list, elt, tmp)
		{
			nrm_scope_destroy(elt->scope);
			nrm_ringbuffer_destroy(&elt->past);
			nrm_vector_destroy(&elt->events);
			free(elt);
		}
		HASH_DEL(eb->hash, s);
		free(s);
	}
	return 0;
}

int nrm_eventbase_push_event(nrm_eventbase_t *eb,
                             nrm_string_t sensor_uuid,
                             nrm_scope_t *scope,
                             nrm_time_t time,
                             double value)
{
	struct nrm_sensor2scope_s *s2s;
	struct nrm_scope2ring_s *s2r;
	HASH_FIND(hh, eb->hash, sensor_uuid, nrm_string_strlen(sensor_uuid),
	          s2s);
	if (s2s == NULL)
		s2s = nrm_eventbase_add_sensor(eb, sensor_uuid);
	DL_FOREACH(s2s->list, s2r)
	{
		if (!nrm_scope_cmp(s2r->scope, scope)) {
			nrm_eventbase_add_event(s2r, time, value);
			return 0;
		}
	}
	/* did not find the scope */
	s2r = nrm_eventbase_add_scope(eb, scope);
	DL_APPEND(s2s->list, s2r);
	nrm_eventbase_add_event(s2r, time, value);
	return 0;
}

int nrm_eventbase_last_value(nrm_eventbase_t *eb,
                             nrm_string_t sensor_uuid,
                             nrm_string_t scope_uuid,
                             double *value)
{
	struct nrm_sensor2scope_s *s2s;
	struct nrm_scope2ring_s *s2r;
	HASH_FIND(hh, eb->hash, sensor_uuid, nrm_string_strlen(sensor_uuid),
	          s2s);
	if (s2s == NULL) {
		*value = 0.0;
		return -NRM_EINVAL;
	}
	DL_FOREACH(s2s->list, s2r)
	{
		if (!nrm_string_cmp(scope_uuid, s2r->scope->uuid)) {
			void *p;
			int err;
			nrm_event_t *e;
			*value = 0.0;
			err = nrm_ringbuffer_back(s2r->past, &p);
			if (err)
				return err;
			e = (nrm_event_t *)p;
			*value = e->value;
			return 0;
		}
	}
	*value = 0.0;
	return -NRM_EINVAL;
}

int nrm_eventbase_current_events(nrm_eventbase_t *eb,
                                 nrm_string_t sensor_uuid,
                                 nrm_string_t scope_uuid,
                                 nrm_vector_t **events)
{
	struct nrm_sensor2scope_s *s2s;
	struct nrm_scope2ring_s *s2r;
	HASH_FIND(hh, eb->hash, sensor_uuid, nrm_string_strlen(sensor_uuid),
	          s2s);
	if (s2s == NULL) {
		*events = NULL;
		return -NRM_EINVAL;
	}
	DL_FOREACH(s2s->list, s2r)
	{
		if (!nrm_string_cmp(scope_uuid, s2r->scope->uuid)) {
			*events = s2r->events;
			return 0;
		}
	}
	*events = NULL;
	return -NRM_EINVAL;
}

nrm_eventbase_t *nrm_eventbase_create(size_t maxperiods)
{
	nrm_eventbase_t *ret = calloc(1, sizeof(nrm_state_t));
	if (ret == NULL)
		return NULL;
	ret->maxperiods = maxperiods;
	ret->hash = NULL;
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
	nrm_eventbase_t *s = *eventbase;

	nrm_sensor2scope_t *current, *tmp;
	HASH_ITER(hh, s->hash, current, tmp) // hh_name, head, item_ptr,
	                                     // tmp_item_ptr
	{
		nrm_scope2ring_t *elt, *s2rtmp;
		DL_FOREACH_SAFE(current->list, elt, s2rtmp)
		{
			nrm_scope_destroy(elt->scope);
			nrm_ringbuffer_destroy(&(elt->past));
			nrm_vector_destroy(&(elt->events));
			DL_DELETE(current->list, elt);
			free(elt);
		}

		nrm_string_decref(current->uuid);
		HASH_DEL(s->hash, current); // head, item_ptr
		free(current);
	}
	HASH_CLEAR(hh, s->hash); // just in case. hh_name, head
	free(s);
	*eventbase = NULL;
}
