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

#include "nrm_messaging.h"
#include <inttypes.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* min time in nsec between messages: necessary for rate-limiting progress
 * report. For now, 10ms is the threashold. */

struct nrm_context {
  void *context;
  void *socket;
  char *cmd_id;
  char *task_id;
  int rank_id;
  int thread_id;
  pid_t process_id;
  struct timespec time;
  unsigned long acc;
};

#define NRM_DEFAULT_URI "ipc:///tmp/nrm-downstream-event"
#define NRM_ENV_URI "ARGO_NRM_DOWNSTREAM_EVENT_URI"
#define NRM_ENV_RATELIMIT "ARGO_NRM_RATELIMIT"
#define NRM_ENV_TRANSMIT "ARGO_NRM_TRANSMIT"
#define NRM_DEFAULT_RATELIMIT_THRESHOLD (10000000LL)

struct nrm_context *nrm_ctxt_create(void);
int nrm_ctxt_delete(struct nrm_context *);

int nrm_init(struct nrm_context *, const char *);
int nrm_fini(struct nrm_context *);

int nrm_send_progress(struct nrm_context *, unsigned long progress);
int nrm_send_phase_context(struct nrm_context *ctxt, unsigned int cpu,
                           unsigned long long int computeTime);

/* Utility function*/
long long int nrm_timediff(struct nrm_context *ctxt, struct timespec end_time);

#ifdef __cplusplus
}
#endif

#endif
