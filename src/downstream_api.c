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

#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "nrm.h"

static long long int nrm_ratelimit_threshold;

struct nrm_context* nrm_ctxt_create(void)
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

int nrm_init(struct nrm_context *ctxt, const char *uuid)
{
    assert(ctxt != NULL);
    assert(uuid != NULL);
    const char *uri = getenv(NRM_ENV_URI);
    size_t buff_size;
    int immediate = 1;
    const char *rate = getenv(NRM_ENV_RATELIMIT);
    if(uri == NULL)
        uri = NRM_DEFAULT_URI;
    if(rate == NULL)
        nrm_ratelimit_threshold = NRM_DEFAULT_RATELIMIT_THRESHOLD;
    else {
        /* see strtoul(3) for details. */
        errno = 0;
        nrm_ratelimit_threshold = strtoull(rate, NULL, 10);
        assert(errno == 0);
    }
    assert(nrm_ratelimit_threshold > 0);
    ctxt -> cmdID = getenv("NRM_CmdID");
    assert(ctxt ->cmdID!= NULL);
    buff_size = strnlen(uuid, 255) + 1;
    buff_size += 16;
    ctxt->app_uuid = malloc(buff_size*sizeof(char));
    /*strncpy(ctxt->app_uuid, uuid, buff_size);*/
    snprintf(ctxt->app_uuid,buff_size,"%s-%u",uuid,getpid());
    ctxt->context = zmq_ctx_new();
    ctxt->socket = zmq_socket(ctxt->context, ZMQ_DEALER);
    zmq_setsockopt(ctxt->socket, ZMQ_IDENTITY, ctxt->app_uuid, strnlen(ctxt->app_uuid, buff_size));
    zmq_setsockopt(ctxt->socket, ZMQ_IMMEDIATE, &immediate, sizeof(immediate));
    int err = zmq_connect(ctxt->socket, uri);
    assert(err == 0);
    char buf[512];
    snprintf(buf, 512, NRM_THREADSTART_FORMAT, ctxt->container_uuid, ctxt->app_uuid);
    sleep(1);
    err = zmq_send(ctxt->socket, buf, strnlen(buf, 512), 0);
    assert(err > 0);
    assert(!clock_gettime(CLOCK_MONOTONIC, &ctxt->time));
    ctxt->acc = 0;
    return 0;
}

int nrm_fini(struct nrm_context *ctxt)
{
    assert(ctxt != NULL);
    char buf[512];
    if (ctxt->acc != 0) {
        snprintf(buf, 512, NRM_THREADPROGRESS_FORMAT, ctxt->acc, ctxt->app_uuid);
        int err = zmq_send(ctxt->socket, buf, strnlen(buf, 512), 0);
    }
    snprintf(buf, 512, NRM_THREADEXIT_FORMAT, ctxt->app_uuid);
    int err = zmq_send(ctxt->socket, buf, strnlen(buf, 512), 0);
    assert(err > 0);
    free(ctxt->app_uuid);
    zmq_close(ctxt->socket);
    zmq_ctx_destroy(ctxt->context);
    return 0;
}

int nrm_send_progress(struct nrm_context *ctxt, unsigned long progress)
{
    char buf[512];
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    long long int timediff = nrm_timediff(ctxt, now);
    ctxt->acc += progress;
    if(timediff > nrm_ratelimit_threshold)
    {
        snprintf(buf, 512, NRM_PROGRESS_FORMAT, ctxt->acc, ctxt->app_uuid);
        int err = zmq_send(ctxt->socket, buf, strnlen(buf, 512), ZMQ_DONTWAIT);
        if(err == -1)
        {
            assert(errno == EAGAIN);
            /* send would block, so act like a ratelimit */
        }
        else
        {
            assert(err > 0);
            ctxt->acc = 0;
            ctxt->time = now;
        }
    }
    return 0;
}

int nrm_send_phase_context(struct nrm_context *ctxt, unsigned int cpu,
                           unsigned long long int computeTime)
{
    char buf[512];
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    long long int timediff = nrm_timediff(ctxt, now);
    ctxt->acc++;
    if(timediff > nrm_ratelimit_threshold)
    {
        snprintf(buf, 512, NRM_PHASECONTEXT_FORMAT, cpu, (unsigned int)(ctxt->acc),
                 computeTime, timediff, ctxt->app_uuid);
        // should we use ZMQDONTWAIT in next command?
        int err = zmq_send(ctxt->socket, buf, strnlen(buf, 512), 0);
        if(err == -1)
        {
            assert(errno == EAGAIN);
            /* send would block, so act like a ratelimit */
        }
        else
        {
            assert(err > 0);
            ctxt->acc = 0;
            ctxt->time = now;
        }
    }
    return 0;
}
