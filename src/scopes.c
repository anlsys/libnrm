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
#include <stdlib.h>

struct nrm_bitmap {
	size_t size;
	unsigned long *array;
};

struct nrm_scope {
	struct nrm_bitmap maps[NRM_SCOPE_TYPE_MAX];
};

nrm_scope_t *nrm_scope_create() 
{
	return calloc(1, sizeof(nrm_scope_t));
}

int nrm_scope_add(const nrm_scope_t *s, unsigned int type, unsigned int num)
{
	(void)s;
	(void)type;
	(void)num;
	return 0;
}

int nrm_scope_add_atomic(const nrm_scope_t *s, unsigned type, unsigned int num)
{
	(void)s;
	(void)type;
	(void)num;
	return 0;
}

size_t nrm_scope_length(const nrm_scope_t *s, unsigned int type)
{
	(void)s;
	(void)type;
	return 0;
}


int nrm_scope_delete(nrm_scope_t *s)
{
	(void)s;
	return 0;
}

int nrm_scope_snprintf(char *buf, size_t bufsize, const nrm_scope_t *s)
{
	(void)s;
	(void)buf;
	(void)bufsize;
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
int nrm_scope_threadshared(const nrm_scope_t *s)
{
        unsigned int cpu;
        unsigned int node;
        getcpu(&cpu, &node);
	nrm_scope_add_atomic(s, NRM_SCOPE_TYPE_CPU, cpu);
	nrm_scope_add_atomic(s, NRM_SCOPE_TYPE_NUMA, cpu);
	return 0;
}

/* called once by the thread involved, maps to the resources in use by this
 * thread ONLY
 */
int nrm_scope_threadprivate(const nrm_scope_t *s)
{
        unsigned int cpu;
        unsigned int node;
        getcpu(&cpu, &node);
	nrm_scope_add(s, NRM_SCOPE_TYPE_CPU, cpu);
	nrm_scope_add(s, NRM_SCOPE_TYPE_NUMA, cpu);
	return 0;
}
