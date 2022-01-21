/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#ifndef NRM_SCOPES_H
#define NRM_SCOPES_H 1

/*******************************************************************************
 * Scopes
 * List of resources (uniqued ints across 3 types: cpu, numa nodes, gpus)
 * corresponding to a progress report.
 ******************************************************************************/

/**
 * Define a type used internally to represent a list of unique unsigned ints
 **/
typedef struct nrm_scope nrm_scope_t;

nrm_scope_t *nrm_scope_create(void);

#define NRM_SCOPE_TYPE_CPU 0
#define NRM_SCOPE_TYPE_NUMA 1
#define NRM_SCOPE_TYPE_GPU 2
#define NRM_SCOPE_TYPE_MAX 3

/**
 * Add an int to the list
 */
int nrm_scope_add(nrm_scope_t *, unsigned int type, unsigned int num);

int nrm_scope_add_atomic(nrm_scope_t *, unsigned int type, unsigned int num);

/**
 * Size of the list (number of elements)
 **/
size_t nrm_scope_length(const nrm_scope_t *, unsigned int type);

int nrm_scope_delete(nrm_scope_t *);

int nrm_scope_snprintf(char *buf, size_t bufsize, const nrm_scope_t *);

/*******************************************************************************
 * Scope Utils
 ******************************************************************************/

int nrm_scope_threadshared(nrm_scope_t *);
int nrm_scope_threadprivate(nrm_scope_t *);

#endif
