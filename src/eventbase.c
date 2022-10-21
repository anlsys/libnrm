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

/* the typedef is already in nrm.h */
struct nrm_eventbase_s {
	size_t maxperiods;
	struct nrm_sensor2scope_s *hash;
};

/* events, as stored in one of the ringbuffers */
struct nrm_event_s {
	nrm_time_t time;
	double value;
};
typedef struct nrm_event_s nrm_event_t;

/* the main structure, matches a scope to two ringbuffers, one for the current
 * window (one event per signal), one for past windows (one event per period).
 */
typedef struct nrm_scope2ring_s {
	nrm_scope_t *scope;
	nrm_ringbuffer_t *past;
	nrm_vector_t *events;
	/* needed to link them together */
	struct nrm_scope2ring_s *prev;
	struct nrm_scope2ring_s *next;
} nrm_scope2ring_s;
typedef struct nrm_scope2ring_s nrm_scope2ring_t;

nrm_scope2ring_s *head = NULL;

/* a way to hash a sensor uuid to a list of scope2ring struct. */
struct nrm_sensor2scope_s {
	nrm_string_t uuid;
	struct nrm_scope2ring_s *list;
	UT_hash_handle hh;
};
typedef struct nrm_sensor2scope_s nrm_sensor2scope_t;

int nrm_eventbase_new_period(struct nrm_scope2ring_s *s, nrm_time_t time)
{
	nrm_event_t period;
	period.time = time;
	period.value = 0;
	nrm_vector_foreach(s->events, iterator)
	{
		nrm_event_t *f = nrm_vector_iterator_get(iterator);
		period.value += f->value;
	}
	nrm_vector_clear(s->events);
	nrm_ringbuffer_push_back(s->past, &period);
	return 0;
}

int nrm_eventbase_tick(nrm_eventbase_t *eb, nrm_time_t time)
{
	struct nrm_sensor2scope_s *s2s;
	for (s2s = eb->hash; s2s != NULL; s2s = s2s->hh.next) {
		struct nrm_scope2ring_s *s2r;
		DL_FOREACH(s2s->list, s2r)
		{
			nrm_eventbase_new_period(s2r, time);
		}
	}
	return 0;
}

int nrm_eventbase_add_event(struct nrm_scope2ring_s *s,
                            nrm_time_t time,
                            double val)
{
	nrm_event_t e;
	e.time = time;
	e.value = val;
	nrm_vector_push_back(s->events, &e);
	return 0;
}

struct nrm_scope2ring_s *nrm_eventbase_add_scope(nrm_eventbase_t *eb,
                                                 nrm_scope_t *scope)
{
	struct nrm_scope2ring_s *ret;
	ret = calloc(1, sizeof(struct nrm_scope2ring_s));
	if (ret == NULL)
		return NULL;
	ret->scope = nrm_scope_dup(scope);
	nrm_ringbuffer_create(&ret->past, eb->maxperiods, sizeof(nrm_event_t));
	nrm_vector_create(&ret->events, sizeof(nrm_event_t));
	return ret;
}

struct nrm_sensor2scope_s *nrm_eventbase_add_sensor(nrm_eventbase_t *eb,
                                                    nrm_string_t sensor_uuid)
{
	/* add a sensor2scope to the hash table */
	struct nrm_sensor2scope_s *s;
	s = calloc(1, sizeof(struct nrm_sensor2scope_s));
	if (s == NULL)
		return NULL;
	s->uuid = sensor_uuid;
	nrm_string_incref(sensor_uuid);
	s->list = NULL;
	HASH_ADD_STR(eb->hash, uuid, s);
	return s;
}

int nrm_eventbase_remove_sensor(nrm_eventbase_t *eb, nrm_uuid_t *sensor_uuid)
{
	struct nrm_sensor2scope_s *s;
	char *uuid = nrm_uuid_to_char(sensor_uuid);
	HASH_FIND_STR(eb->hash, uuid, s);
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
	HASH_FIND_STR(eb->hash, sensor_uuid, s2s);
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
	HASH_FIND_STR(eb->hash, sensor_uuid, s2s);
	if (s2s == NULL) {
		*value = 0.0;
		return -NRM_EINVAL;
	}
	DL_FOREACH(s2s->list, s2r)
	{
		if (!nrm_string_cmp(scope_uuid, s2r->scope->uuid)) {
			nrm_ringbuffer_back(s2r->past, &value);
			return 0;
		}
	}
	*value = 0.0;
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

void nrm_eventbase_destroy(nrm_eventbase_t **eventbase)
{
	if (eventbase == NULL || *eventbase == NULL)
		return;
	nrm_eventbase_t *s = *eventbase;

	/* TODO: iterate over the hash and destroy sub structures. */
	s->maxperiods = NULL;
	nrm_hash_foreach(s->hash, iter)
	{
		nrm_sensor2scope_t *a = nrm_hash_iterator_get(iter);
		nrm_uuid_destroy(&a->uuid);
		free(a);

		nrm_scope2ring_t *elt, *s2rtmp;
		DL_FOREACH_SAFE(s->hash->list, elt, s2rtmp)
		{
			nrm_scope_destroy(elt->scope);
			nrm_ringbuffer_destroy(&elt->past);
			nrm_vector_destroy(&elt->events);
			DL_DELETE(head, elt);
			free(elt);
		}
	}
	nrm_hash_destroy(&s->hash);
	free(s);
	*eventbase = NULL;
}
