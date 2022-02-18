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

struct nrm_scope {
	struct nrm_bitmap maps[NRM_SCOPE_TYPE_MAX];
};

nrm_scope_t *nrm_scope_create()
{
	return calloc(1, sizeof(nrm_scope_t));
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

int nrm_scope_delete(nrm_scope_t *s)
{
	free(s);
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

	const char format[] = "{\"cpu\": [%s], \"numa\": [%s], \"gpu\": [%s]}";
	/* "-6 for the %s */
	if (bufsize < length + sizeof(format) - 6)
		return -1;

	snprintf(buf, bufsize, format, scopes[0], scopes[1], scopes[2]);
	for (int i = 0; i < NRM_SCOPE_TYPE_MAX; i++)
		free(scopes[i]);
	return 0;
}

json_t *nrm_scope_to_json(nrm_scope_t *scope)
{
	json_t *cpu, *numa, *gpu;

	cpu = nrm_bitmap_to_json(&scope->maps[0]);
	numa = nrm_bitmap_to_json(&scope->maps[1]);
	gpu = nrm_bitmap_to_json(&scope->maps[2]);
	return json_pack("{s:o, s:o, s:o}",
			 "cpu", cpu, "numa", numa, "gpu", gpu);
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
