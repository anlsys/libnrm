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

#define NRM_SCOPE_TYPE_CPU 0
#define NRM_SCOPE_TYPE_NUMA 1
#define NRM_SCOPE_TYPE_GPU 2
#define NRM_SCOPE_TYPE_MAX 3

/**
 * a named scope, as maintained by the daemon.
 */
typedef struct nrm_named_scope_s {
	nrm_string_t name;
	nrm_scope_t *scope;
} nrm_named_scope_t;

nrm_named_scope_t *nrm_named_scope_create(const char *name, nrm_scope_t *scope);
int nrm_named_scope_destroy(nrm_named_scope_t **);

/**
 * Creates and returns a new NRM scope
 */
nrm_scope_t *nrm_scope_create();

/**
 * Add an int corresponding to some hardware ID to the list
 * @param scope: scope pointer
 * @param type: an int for some `NRM_SCOPE_TYPE_`
 * @param num: an integer corresponding to some hardware ID
 */
int nrm_scope_add(nrm_scope_t *scope, unsigned int type, unsigned int num);

int nrm_scope_add_atomic(nrm_scope_t *scope,
                         unsigned int type,
                         unsigned int num);

/**
 * Size of the list (number of elements)
 * @param scope: scope pointer
 * @param type: an int for some `NRM_SCOPE_TYPE_`
 **/
size_t nrm_scope_length(const nrm_scope_t *scope, unsigned int type);

/**
 * Removes an NRM scope. Do this for each scope before an instrumented program
 * exits.
 * @return 0 if successful, an error code otherwise
 */
int nrm_scope_destroy(nrm_scope_t *scope);

/**
 * Creates and returns a pointer to a copy of an NRM scope
 */
nrm_scope_t *nrm_scope_dup(nrm_scope_t *scope);

/**
 * Compares two NRM scopes.
 * @return 0 if equivalent, 1 otherwise
 */
int nrm_scope_cmp(nrm_scope_t *one, nrm_scope_t *two);

/**
 * snprintf the contents of an NRM scope
 * @return 0 if successful, an error code otherwise
 */
int nrm_scope_snprintf(char *buf, size_t bufsize, const nrm_scope_t *scope);

/**
 * Returns the UUID of an NRM scope
 * @return the UUID if successful, an error code otherwise
 */
nrm_string_t nrm_scope_uuid(nrm_scope_t *scope);

/*******************************************************************************
 * Scope Utils
 ******************************************************************************/

int nrm_scope_threadshared(nrm_scope_t *scope);
int nrm_scope_threadprivate(nrm_scope_t *scope);

#endif
