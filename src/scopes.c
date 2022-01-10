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

#include <stdlib.h>

struct nrm_scope {
	size_t size;
	int *array;
};

nrm_scope_t *nrm_scope_create() 
{
	return calloc(1, sizeof(nrm_scope_t));
}

int nrm_scope_add(const nrm_scope_t *s, unsigned int num)
{
	(void)s;
	(void)num;
	return 0;
}

size_t nrm_scope_length(const nrm_scope_t *s)
{
	(void)s;
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

/* Given a loop like this:
 * #omp parallel for (+ any other options)
 * for(int i = 0; i < max_iter; i++) {
 *	nrm_scope_from_omp(scope[tid], i);
 * }
 * builds out a scope for a thread tid.
 */
int nrm_scope_from_omp(const nrm_scope_t *s, int i)
{
	(void)s;
	(void)i;
	return 0;
}

#if 0

/* make it so that it builds a scope based on openmp loop.
 * TODO: should we add openmp support in the library, or elsewhere ?
 */

void nrm_topo(int iter)
{
    if (mode != 1)
    {
        unsigned int cpu;
        unsigned int node;
        getcpu(&cpu, &node);
        core_dummy_array[iter] = cpu;
        node_dummy_array[iter] = node;
    }
    warmup = 0;
}

void nrm_get_topo()
{
    // CPUs
    for (int i = 0; i < core_dummy_size; i++)
    {
        int var = 0;
        for (int j = 0; j < cvector_size(cpus_vector); j++)
        {
            if (cpus_vector[j] == core_dummy_array[i])
            {
                var++;
                break;
            }
        }
        if (var == 0)
        {
            cvector_push_back(cpus_vector, core_dummy_array[i]);
        }
    }
    // Nodes
    for (int i = 0; i < node_dummy_size; i++)
    {
        int var = 0;
        for (int j = 0; j < cvector_size(nodes_vector); j++)
        {
            if (nodes_vector[j] == node_dummy_array[i])
            {
                var++;
                break;
            }
        }
        if (var == 0)
        {
            cvector_push_back(nodes_vector, node_dummy_array[i]);
        }
    }
    // GPUs
    if (mode != 0)
    {
        for (int i = 0; i < gpu_dummy_size; i++)
        {
            int var = 0;
            for (int j = 0; j < cvector_size(gpus_vector); j++)
            {
                if (gpus_vector[j] == gpu_dummy_array[i])
                {
                    var++;
                    break;
                }
            }
            if (var == 0)
            {
                cvector_push_back(gpus_vector, gpu_dummy_array[i]);
            }
        }
    }
}

#endif
