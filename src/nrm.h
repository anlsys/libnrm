/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://xgitlab.cels.anl.gov/argo/libnrm
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

/**
 * Sends MPI phase context information
 *
 * @param ctxt: pointer to the libnrm context.
 *
 * @param cpu: TODO
 *
 * @param computeTime: TODO
 *
 */
int nrm_send_phase_context(struct nrm_context *ctxt,
                           unsigned int cpu,
                           unsigned long long int computeTime);

#ifdef __cplusplus
}
#endif

#endif
