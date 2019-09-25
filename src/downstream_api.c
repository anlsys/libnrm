/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://xgitlab.cels.anl.gov/argo/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

/* Filename: downstream.c
 *
 * Description: This file contains the implementation of downstream API to
 * transmit application context information to NRM.
 *
 * The application context information transmitted can be used to monitor
 * application progress and/or invoke power policies to improve energy
 * efficiency at the node level.
 */

#include <assert.h>
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>

#include "nrm.h"

static long long int nrm_ratelimit_threshold;

struct nrm_context *nrm_ctxt_create(void) {
  struct nrm_context *ctxt;
  ctxt = calloc(1, sizeof(struct nrm_context));
  assert(ctxt != NULL);
  return ctxt;
}

int nrm_ctxt_delete(struct nrm_context *ctxt) {
  assert(ctxt != NULL);
  free(ctxt);
  return 0;
}

int nrm_init(struct nrm_context *ctxt, const char *task_id) {
  assert(ctxt != NULL);
  assert(task_id != NULL);
  const char *uri = getenv(NRM_ENV_URI);
  size_t buff_size;
  int immediate = 1;
  const char *rate = getenv(NRM_ENV_RATELIMIT);
  if (uri == NULL)
    uri = NRM_DEFAULT_URI;
  if (rate == NULL)
    nrm_ratelimit_threshold = NRM_DEFAULT_RATELIMIT_THRESHOLD;
  else {
    /* see strtoul(3) for details. */
    errno = 0;
    nrm_ratelimit_threshold = strtoull(rate, NULL, 10);
    assert(errno == 0);
  }
  assert(nrm_ratelimit_threshold > 0);

  // Initializing the context.

  // cmdID: an identifier used to reconcile messages with commands at the NRM
  // level.
  ctxt->cmd_id = getenv("NRM_CMDID");
  assert(ctxt->cmd_id != NULL);
  // process_id: the PID.
  ctxt->process_id = getpid();
  // task_id: a static application-specified identifier.
  ctxt->task_id = strdup(task_id);
  // rank_id: The MPI rank.
  MPI_Comm_rank(MPI_COMM_WORLD, &(ctxt->rank_id));
  // thread_id: The OpenMP thread number.
  ctxt->thread_id = omp_get_thread_num();

  ctxt->context = zmq_ctx_new();
  ctxt->socket = zmq_socket(ctxt->context, ZMQ_DEALER);
  zmq_setsockopt(ctxt->socket, ZMQ_IDENTITY, taskID, strlen(taskID));
  zmq_setsockopt(ctxt->socket, ZMQ_IMMEDIATE, &immediate, sizeof(immediate));
  int err = zmq_connect(ctxt->socket, uri);
  assert(err == 0);
  char buf[512];
  snprintf(buf, 512, NRM_THREADSTART_FORMAT, ctxt->cmd_id);
  sleep(1);
  err = zmq_send(ctxt->socket, buf, strnlen(buf, 512), 0);
  assert(err > 0);
  assert(!clock_gettime(CLOCK_MONOTONIC, &ctxt->time));
  ctxt->acc = 0;
  return 0;
}

int nrm_fini(struct nrm_context *ctxt) {
  assert(ctxt != NULL);
  char buf[512];
  if (ctxt->acc != 0) {
    snprintf(buf, 512, NRM_THREADPROGRESS_FORMAT, ctxt->cmd_id, ctxt->acc);
    int err = zmq_send(ctxt->socket, buf, strnlen(buf, 512), 0);
  }
  snprintf(buf, 512, NRM_THREADEXIT_FORMAT, ctxt->cmd_id);
  int err = zmq_send(ctxt->socket, buf, strnlen(buf, 512), 0);
  assert(err > 0);
  free(ctxt->cmd_id);
  zmq_close(ctxt->socket);
  zmq_ctx_destroy(ctxt->context);
  return 0;
}

int nrm_send_progress(struct nrm_context *ctxt, unsigned long progress) {
  char buf[512];
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  long long int timediff = nrm_timediff(ctxt, now);
  ctxt->acc += progress;
  if (timediff > nrm_ratelimit_threshold) {
    snprintf(buf, 512, NRM_THREADPROGRESS_FORMAT, ctxt->cmd_id, ctxt->acc);
    int err = zmq_send(ctxt->socket, buf, strnlen(buf, 512), ZMQ_DONTWAIT);
    if (err == -1) {
      assert(errno == EAGAIN);
      /* send would block, so act like a ratelimit */
    } else {
      assert(err > 0);
      ctxt->acc = 0;
      ctxt->time = now;
    }
  }
  return 0;
}

int nrm_send_phase_context(struct nrm_context *ctxt, unsigned int cpu,
                           unsigned long long int computeTime) {
  char buf[512];
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  long long int timediff = nrm_timediff(ctxt, now);
  ctxt->acc++;
  if (timediff > nrm_ratelimit_threshold) {
    snprintf(buf, 512, NRM_THREADPHASECONTEXT_FORMAT, cpu,
             (unsigned int)(ctxt->acc), ctxt->cmd_id, computeTime, timediff);
    // should we use ZMQDONTWAIT in next command?
    int err = zmq_send(ctxt->socket, buf, strnlen(buf, 512), 0);
    if (err == -1) {
      assert(errno == EAGAIN);
      /* send would block, so act like a ratelimit */
    } else {
      assert(err > 0);
      ctxt->acc = 0;
      ctxt->time = now;
    }
  }
  return 0;
}
