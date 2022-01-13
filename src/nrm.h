/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

/* Filename: downstream_api.h
 *
 * Includes required headers, functions and parameters used by NRM downstream
 * interface
 *
 */

#ifndef NRM_H
#define NRM_H 1

/**
 * @defgroup nrm "nrmMlib API"
 * @brief nrmlib C instrumentation API.
 *
 * Scratchpad creates one thread to trigger synchronous dma movements.
 * @{
 **/

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <time.h>

/*******************************************************************************
 * High Resolution Timers
 * type and functions to save a timestamp and compute a difference.
 * Resolution should be in nanoseconds.
 ******************************************************************************/

/**
 * Define type used to internally save timestamps (in nanoseconds since epoch)
 **/
typedef struct timespec nrm_time_t;

/**
 * Save timestamps into timer
 **/
void nrm_time_gettime(nrm_time_t *now);

/**
 * Compute the time difference between two timestamps, as nanoseconds.
 **/
int64_t nrm_time_diff(const nrm_time_t *start, const nrm_time_t *end);

/**
 * Convert timestamp into nanoseconds since epoch, as an int64_t value
 **/
int64_t nrm_time_tons(const nrm_time_t *time);

/*******************************************************************************
 * Bitmaps
 ******************************************************************************/

#define NRM_BITMAP_MAX 2048
/** The size in Bytes of aml_bitmap **/
#define NRM_BITMAP_BYTES (NRM_BITMAP_MAX / 8)
/** The type used to store bits **/
#define NRM_BITMAP_TYPE unsigned long
/** The number of basic type elements used to store bits **/
#define NRM_BITMAP_SIZE ((size_t)(NRM_BITMAP_BYTES / sizeof(NRM_BITMAP_TYPE)))
/** The number of bits held in each basic type element **/
#define NRM_BITMAP_NBITS ((size_t)(8 * sizeof(NRM_BITMAP_TYPE)))

struct nrm_bitmap {
	unsigned long mask[NRM_BITMAP_SIZE];
};

/**
 * Check whether a bit in bitmap is set.
 **/
int nrm_bitmap_isset(const struct nrm_bitmap *bitmap, const unsigned int i);

/**
 * Check whether a bit in bitmap is empty.
 **/
int nrm_bitmap_iszero(const struct nrm_bitmap *bitmap);

/**
 * Set a bit in bitmap.
 **/
int nrm_bitmap_set(struct nrm_bitmap *bitmap, const unsigned int i);

/**
 * Set a bit in bitmap atomically
 **/
int nrm_bitmap_set_atomic(struct nrm_bitmap *bitmap, const unsigned int i);

/**
 * Count the number of bits set in bitmap.
 **/
int nrm_bitmap_nset(const struct nrm_bitmap *bitmap);

/**
 * Bitmap conversion to string. Output strings are index numbers wrapped in
 *brackets [].
 **/
char *nrm_bitmap_to_string(const struct nrm_bitmap *bitmap);

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

/*******************************************************************************
 * Downstream API
 ******************************************************************************/

struct nrm_context;

struct nrm_context *nrm_ctxt_create(void);

/**
 * deletes a nrm_context structure
 *
 * @param ctxt: pointer to a nrm_context structure.
 *
 */
int nrm_ctxt_delete(struct nrm_context *);

/**
 * Initializes a context for libnrm
 *
 * @param ctxt: pointer to a nrm_context structure.
 *
 */
int nrm_init(struct nrm_context *ctxt, const char *name, int rank, int thread);

/**
 * Ends libnrm's operation.
 *
 * @param ctxt: pointer to a nrm_context structure.
 *
 */
int nrm_fini(struct nrm_context *ctxt);

/**
 * Sends MPI phase context information
 *
 * @param ctxt: pointer to the libnrm context.
 *
 * @param progress: cumulative value that represents the application progress
 * since the last progress report.
 */
int nrm_send_progress(struct nrm_context *ctxt,
                      unsigned long progress,
                      nrm_scope_t *scope);

#ifdef __cplusplus
}
#endif

#endif
