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
#include "internal/uthash.h"
#include "internal/utlist.h"

/* the typedef is already in nrm.h */
struct nrm_eventbase_s {
	size_t maxevents;
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
struct nrm_scope2ring_s {
	nrm_scope_t *scope;
	nrm_ringbuffer_t *past;
	nrm_ringbuffer_t *events;
	/* needed to link them together */
	struct nrm_scope2ring_s *prev;
	struct nrm_scope2ring_s *next;
};

/* a way to hash a sensor uuid to a list of scope2ring struct. */
struct nrm_sensor2scope_s {
	nrm_string_t uuid;
	struct nrm_scope2ring_s *list;	
	UT_hash_handle hh;
};

int nrm_eventbase_new_period(struct nrm_scope2ring_s *s, nrm_time_t time)
{
	nrm_event_t period;
	period.time = time;
	period.value = 0;
	size_t len;
	nrm_ringbuffer_length(s->events, &len);
	for (size_t i = 0; i < len; i++) {
		nrm_event_t *f;
		nrm_ringbuffer_get(s->events, i, (void **)&f);
		period.value += f->value;
	}
	nrm_ringbuffer_clear(s->events);
	nrm_ringbuffer_push_back(s->past, &period);
	return 0;
}

int nrm_eventbase_add_event(struct nrm_scope2ring_s *s, nrm_time_t time, double
			    val)
{
	if (nrm_ringbuffer_isfull(s->events)) {
		/* in this case, we aggregate the entire ringbuffer */
		size_t len;
		nrm_event_t *f;
		nrm_event_t agg;
		agg.value = 0;
		nrm_ringbuffer_length(s->events, &len);
		for (size_t i = 0; i < len; i++) {
			nrm_ringbuffer_get(s->events, i, (void **)&f);
			agg.value += f->value;
		}
		agg.time = f->time;
		nrm_ringbuffer_clear(s->events);
		nrm_ringbuffer_push_back(s->events, &agg);
	}
	nrm_event_t e;
	e.time = time;
	e.value = val;
	nrm_ringbuffer_push_back(s->events, &e);
	return 0;
}

struct nrm_scope2ring_s *nrm_eventbase_add_scope(nrm_eventbase_t *eb, nrm_scope_t *scope)
{
	struct nrm_scope2ring_s *ret;
	ret = calloc(1, sizeof(struct nrm_scope2ring_s));
	if (ret == NULL)
		return NULL;
	ret->scope = nrm_scope_dup(scope);
	nrm_ringbuffer_create(&ret->past, eb->maxperiods, sizeof(nrm_event_t));
	nrm_ringbuffer_create(&ret->events, eb->maxevents, sizeof(nrm_event_t));
	return ret;
}

struct nrm_sensor2scope_s *nrm_eventbase_add_sensor(nrm_eventbase_t *eb, nrm_uuid_t *sensor_uuid)
{
	/* add a sensor2scope to the hash table */
	struct nrm_sensor2scope_s *s;
	s = calloc(1, sizeof(struct nrm_sensor2scope_s));
	if (s == NULL)
		return NULL;
	s->uuid = nrm_uuid_to_string(sensor_uuid);
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
		DL_FOREACH_SAFE(s->list, elt, tmp) {
			nrm_scope_destroy(elt->scope);
			nrm_ringbuffer_destroy(&elt->past);
			nrm_ringbuffer_destroy(&elt->events);
			free(elt);
		}
		HASH_DEL(eb->hash, s);
		free(s);
	}
	return 0;
}

int nrm_eventbase_push_event(nrm_eventbase_t *eb, nrm_uuid_t *sensor_uuid, nrm_scope_t *scope,
			    nrm_time_t time, double value)
{
	struct nrm_sensor2scope_s *s2s;
	struct nrm_scope2ring_s *s2r;
	char *uuid = nrm_uuid_to_char(sensor_uuid);
	HASH_FIND_STR(eb->hash, uuid, s2s);
	if (s2s == NULL)
		s2s = nrm_eventbase_add_sensor(eb, sensor_uuid);
	DL_FOREACH(s2s->list, s2r) {
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

nrm_eventbase_t *nrm_eventbase_create(size_t maxevents, size_t maxperiods)
{
	nrm_eventbase_t *ret = calloc(1, sizeof(nrm_state_t));
	if (ret == NULL)
		return NULL;
	ret->maxevents = maxevents;
	ret->maxperiods = maxperiods;
	ret->hash = NULL;
	return ret;
}

void nrm_eventbase_destroy(nrm_eventbase_t **eventbase)
{
	if (eventbase == NULL || *eventbase == NULL)
		return;
	nrm_eventbase_t *s = *eventbase;
	(void)s;
	/* TODO: iterate over the hash and destroy sub structures. */
	*eventbase = NULL;
}
