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

nrm_scope_t *nrm_scope_create(const char *name)
{
	nrm_scope_t *ret = calloc(1, sizeof(nrm_scope_t));
	ret->uuid = nrm_string_fromchar(name);
	return ret;
}

int nrm_scope_add(nrm_scope_t *s, unsigned int type, unsigned int num)
{
	return nrm_bitmap_set(&s->maps[type], num);
}

int nrm_scope_add_atomic(nrm_scope_t *s, unsigned type, unsigned int num)
{
	return nrm_bitmap_set_atomic(&s->maps[type], num);
}

size_t nrm_scope_length(const nrm_scope_t *s, unsigned int type)
{
	return nrm_bitmap_nset(&s->maps[type]);
}

int nrm_scope_destroy(nrm_scope_t *s)
{
	nrm_string_decref(s->uuid);
	free(s);
	return 0;
}

nrm_scope_t *nrm_scope_dup(nrm_scope_t *s)
{
	nrm_scope_t *ret = nrm_scope_create(s->uuid);
	for (size_t i = 0; i < NRM_SCOPE_TYPE_MAX; i++)
		nrm_bitmap_copy(&ret->maps[i], &s->maps[i]);
	return ret;
}

int nrm_scope_cmp(nrm_scope_t *one, nrm_scope_t *two)
{
	for (size_t i = 0; i < NRM_SCOPE_TYPE_MAX; i++)
		if (nrm_bitmap_cmp(&one->maps[i], &two->maps[i]))
			return 1;
	return 0;
}

int nrm_scope_snprintf(char *buf, size_t bufsize, const nrm_scope_t *s)
{
	char *scopes[NRM_SCOPE_TYPE_MAX];
	for (int i = 0; i < NRM_SCOPE_TYPE_MAX; i++)
		scopes[i] = nrm_bitmap_to_string(&s->maps[i]);
	/* count the terminating byte directly */
	size_t length = 0;
	for (int i = 0; i < NRM_SCOPE_TYPE_MAX; i++)
		length += strlen(scopes[i]);

	const char format[] =
	        "name: %s: {\"cpu\": [%s], \"numa\": [%s], \"gpu\": [%s]}";
	/* "-6 for the %s */
	if (bufsize < length + sizeof(format) - 6)
		return -1;

	snprintf(buf, bufsize, format, s->uuid, scopes[0], scopes[1],
	         scopes[2]);
	for (int i = 0; i < NRM_SCOPE_TYPE_MAX; i++)
		free(scopes[i]);
	return 0;
}

nrm_string_t nrm_scope_uuid(nrm_scope_t *scope)
{
	return scope->uuid;
}

json_t *nrm_scope_to_json(nrm_scope_t *scope)
{
	json_t *cpu, *numa, *gpu;

	cpu = nrm_bitmap_to_json(&scope->maps[0]);
	numa = nrm_bitmap_to_json(&scope->maps[1]);
	gpu = nrm_bitmap_to_json(&scope->maps[2]);
	return json_pack("{s:s*, s:o, s:o, s:o}", "uuid", scope->uuid, "cpu",
	                 cpu, "numa", numa, "gpu", gpu);
}

int nrm_scope_from_json(nrm_scope_t *scope, json_t *json)
{
	json_t *cpu = NULL, *numa = NULL, *gpu = NULL;
	char *uuid = NULL;
	json_error_t error;
	int err;
	err = json_unpack_ex(json, &error, 0, "{s?:s, s?:o, s?:o, s?:o}",
	                     "uuid", &uuid, "cpu", &cpu, "numa", &numa, "gpu",
	                     &gpu);
	if (err) {
		nrm_log_error("error unpacking json: %s, %s, %d, %d, %d\n",
		              error.text, error.source, error.line,
		              error.column, error.position);
		return -NRM_EINVAL;
	}
	if (uuid)
		scope->uuid = nrm_string_fromchar(uuid);
	nrm_bitmap_from_json(&scope->maps[0], cpu);
	nrm_bitmap_from_json(&scope->maps[1], numa);
	nrm_bitmap_from_json(&scope->maps[2], gpu);
	return 0;
}

/* UTIL FUNCTIONS: helpful when users want to build out scopes that are
 * meaningful to an OpenMP application.
 */

/* nrm_scope_t *region_scope, *thread_scope[NUM_THREADS];
 * #omp parallel for
 * for(int i = 0; i < max_iter; i++) {
 *	nrm_scope_t *th_scp = thread_scope[omp_get_num_thread()];
 * 	// touched by every thread
 * 	nrm_scope_threadshared(region_scope);
 * 	// one per thread
 * 	nrm_scope_threadprivate(th_scp);
 *
 *	do_work(i);
 *
 *	nrm_send_progress(ctxt, th_scp, 1);
 * }
 *
 * nrm_send_progress(ctxt, region_scope, 1);
 *
 * #omp parallel for
 * for(int i = 0; i < max_iter; i++) {
 *	nrm_scope_t *th_scp = thread_scope[omp_get_num_thread()];
 *
 *      do_another_work(i);
 *	nrm_send_progress(ctxt, th_scp, 1);
 * }
 *
 * nrm_send_progress(ctxt, region_scope, 1);
 */

/* called once per thread involved, maps to the union of all resources in use
 * by ALL threads.
 */
int nrm_scope_threadshared(nrm_scope_t *s)
{
	unsigned int cpu;
	unsigned int node;
	getcpu(&cpu, &node);
	nrm_scope_add_atomic(s, NRM_SCOPE_TYPE_CPU, cpu);
	nrm_scope_add_atomic(s, NRM_SCOPE_TYPE_NUMA, node);
	return 0;
}

/* called once by the thread involved, maps to the resources in use by this
 * thread ONLY
 */
int nrm_scope_threadprivate(nrm_scope_t *s)
{
	unsigned int cpu;
	unsigned int node;
	getcpu(&cpu, &node);
	nrm_scope_add(s, NRM_SCOPE_TYPE_CPU, cpu);
	nrm_scope_add(s, NRM_SCOPE_TYPE_NUMA, node);
	return 0;
}
