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

#include <inttypes.h>
#include <time.h>
#include "nrm_messaging.h"

#ifdef __cplusplus
extern "C" {
#endif

/* min time in nsec between messages: necessary for rate-limiting progress
 * report. For now, 10ms is the threashold. */

struct nrm_context {
    void *context;
    void *socket;
    char *cmdID;
    char *processID;
    char *taskID;
    char *threadID;
    struct timespec time;
    unsigned long acc;
};

#define NRM_DEFAULT_URI "ipc:///tmp/nrm-downstream-event"
#define NRM_ENV_URI "ARGO_NRM_DOWNSTREAM_EVENT_URI"
#define NRM_ENV_RATELIMIT "ARGO_NRM_RATELIMIT"

#define NRM_DEFAULT_RATELIMIT_THRESHOLD (10000000LL)


struct nrm_context* nrm_ctxt_create(void);
int nrm_ctxt_delete(struct nrm_context *);

int nrm_init(struct nrm_context *, const char *);
int nrm_fini(struct nrm_context *);

int nrm_send_progress(struct nrm_context *, unsigned long progress);
int nrm_send_phase_context(struct nrm_context *ctxt,
                           unsigned int cpu,
                           unsigned long long int computeTime);

/* Utility function*/
inline long long int nrm_timediff(struct nrm_context *ctxt, struct timespec
                                  end_time)
{
    long long int timediff = (end_time.tv_nsec - ctxt->time.tv_nsec) +
                             1e9* (end_time.tv_sec - ctxt->time.tv_sec);
    return timediff;
}

#ifdef __cplusplus
}
#endif

#endif
