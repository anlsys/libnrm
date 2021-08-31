/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
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
#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nrm.h"

#include "internal-downstream.h"

static long long int nrm_ratelimit_threshold;
static int nrm_transmit = 1;

struct nrm_context *nrm_ctxt_create(void)
{
	struct nrm_context *ctxt;
	ctxt = calloc(1, sizeof(struct nrm_context));
	assert(ctxt != NULL);
	return ctxt;
}

int nrm_ctxt_delete(struct nrm_context *ctxt)
{
	assert(ctxt != NULL);
	free(ctxt);
	return 0;
}

static void nrm_net_init(struct nrm_context *ctxt, const char *uri)
{
	int immediate = 1;
	if (!nrm_transmit)
		return;
	ctxt->context = zmq_ctx_new();
	ctxt->socket = zmq_socket(ctxt->context, ZMQ_DEALER);
	zmq_setsockopt(ctxt->socket, ZMQ_IMMEDIATE, &immediate,
	               sizeof(immediate));
	int err = zmq_connect(ctxt->socket, uri);
	assert(err == 0);
}

static void nrm_net_fini(struct nrm_context *ctxt)
{
	if (!nrm_transmit)
		return;
	zmq_close(ctxt->socket);
	zmq_ctx_destroy(ctxt->context);
}

static int
nrm_net_send(struct nrm_context *ctxt, char *buf, size_t bufsize, int flags)
{
	if (!nrm_transmit)
		return 1;
	return zmq_send(ctxt->socket, buf, strnlen(buf, bufsize), flags);
}

int nrm_init(struct nrm_context *ctxt,
             const char *task_id,
             int rank_id,
             int thread_id)
{
	assert(ctxt != NULL);
	assert(task_id != NULL);
	size_t buff_size;

	/* env init */
	const char *uri = getenv(NRM_ENV_URI);
	const char *rate = getenv(NRM_ENV_RATELIMIT);
	const char *transmit = getenv(NRM_ENV_TRANSMIT);
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
	if (transmit != NULL)
		nrm_transmit = atoi(transmit);

	/* context init */
	assert(nrm_ratelimit_threshold > 0);
	ctxt->cmd_id = getenv(NRM_ENV_CMDID);
	assert(ctxt->cmd_id != NULL);
	// process_id: the PID.
	ctxt->process_id = getpid();
	// task_id: a static application-specified identifier.
	buff_size = strnlen(task_id, 255) + 1;
	ctxt->task_id = malloc(buff_size * sizeof(char));
	snprintf(ctxt->task_id, buff_size, "%s", task_id);

	ctxt->rank_id = rank_id;
	ctxt->thread_id = thread_id;

	/* net init */
	nrm_net_init(ctxt, uri);
	sleep(1);

	/* app init */
	nrm_time_gettime(&ctxt->time);
	ctxt->acc = 0;
	return 0;
}

int nrm_fini(struct nrm_context *ctxt)
{
	char buf[512];
	int err;
	nrm_time_t now;
	int64_t tm;
	assert(ctxt != NULL);
	nrm_time_gettime(&now);
	tm = nrm_time_tons(&now);
	if (ctxt->acc != 0) {
		snprintf(buf, 512, NRM_THREADPROGRESS_FORMAT, tm, ctxt->acc,
		         ctxt->cmd_id, ctxt->task_id, ctxt->process_id,
		         ctxt->rank_id, ctxt->thread_id);
		err = nrm_net_send(ctxt, buf, 512, 0);
	}
	snprintf(buf, 512, NRM_THREADPAUSE_FORMAT, tm, ctxt->cmd_id, ctxt->task_id,
	         ctxt->process_id, ctxt->rank_id, ctxt->thread_id);
	err = nrm_net_send(ctxt, buf, 512, 0);
	assert(err > 0);
	free(ctxt->task_id);
	nrm_net_fini(ctxt);
	return 0;
}

int nrm_send_progress(struct nrm_context *ctxt, unsigned long progress)
{
	char buf[512];
	nrm_time_t now;
	int64_t tm;
	nrm_time_gettime(&now);
	tm = nrm_time_tons(&now);
	int64_t timediff = nrm_time_diff(&ctxt->time, &now);
	ctxt->acc += progress;
	if (timediff > nrm_ratelimit_threshold) {
		snprintf(buf, 512, NRM_THREADPROGRESS_FORMAT, tm, ctxt->acc,
		         ctxt->cmd_id, ctxt->task_id, (int)ctxt->process_id,
		         ctxt->rank_id, ctxt->thread_id);
		int err = nrm_net_send(ctxt, buf, 512, ZMQ_DONTWAIT);
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
