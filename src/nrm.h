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
int nrm_send_progress(struct nrm_context *ctxt, unsigned long progress);

void nrm_set(int array_size, char* config);
void nrm_topo(int iter);
void nrm_openmp();
void nrm_mpi();
void nrm_hybrid();
void nrm_gpu();

#ifdef __cplusplus
}
#endif

#endif
