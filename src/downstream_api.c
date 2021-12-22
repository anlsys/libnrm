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

#include <sched.h>
#include "cvector.h"

int* node_dummy_array = NULL;
int* core_dummy_array = NULL;
int* gpu_dummy_array  = NULL;
int node_dummy_size   = 0;
int core_dummy_size   = 0;
int gpu_dummy_size    = 0;

cvector_vector_type(int) cores_vector = NULL;
cvector_vector_type(int) gpus_vector  = NULL;
cvector_vector_type(int) nodes_vector = NULL;

int warmup   = 1;
int mode = 0;

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

static int nrm_net_send(struct nrm_context *ctxt, char *buf, size_t bufsize, int flags)
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
    
    cvector_free(cores_vector);
    cvector_free(nodes_vector);
    cvector_free(gpus_vector);
    
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

    // Only one CPU is used anyway in the warmup step
    if (warmup == 1)
    {
        unsigned int cpu;
        unsigned int node;
        getcpu(&cpu, &node);
        for (int i = 0; i < core_dummy_size; i++) /* Workaround in case of cpu number != 0 getting pushed in the vector */
        {
            core_dummy_array[i] = cpu;
        }
        for (int i = 0; i < core_dummy_size; i++)
        {
            node_dummy_array[i] = node;
        }
        cvector_push_back(cores_vector, cpu);
        cvector_push_back(nodes_vector, node);
    }
    else
    {
        nrm_get_topo();
    }

#ifdef VERBOSE
    printf("******************\n");
    printf("Used cores: ");
    for (int i = 0; i < cvector_size(cores_vector); i++)
    {
        printf("%d ", cores_vector[i]);
    }
    printf("\n");

    printf("Used nodes: ");
    for (int i = 0; i < cvector_size(nodes_vector); i++)
    {
        printf("%d ", nodes_vector[i]);
    }
    printf("\n");

    if (mode != 0)
    { 
        printf("Used gpus: ");
        for (int i = 0; i < cvector_size(gpus_vector); i++)
        {
            printf("%d ", gpus_vector[i]);
        }
        printf("\n");
    }
#endif

    return 0;
}

void nrm_set(int array_size, int input_mode, int input_gpu_array[], int input_gpu_size)  //mode 0: cpu, mode 1: gpu, mode 2: hybrid
{
    node_dummy_array = malloc(array_size * sizeof(int));
    core_dummy_array = malloc(array_size * sizeof(int));
    node_dummy_size = array_size;
    core_dummy_size = array_size;

    mode = input_mode;
    if (mode != 0)
    {
        gpu_dummy_size = input_gpu_size;
        gpu_dummy_array = input_gpu_array;
    }
}

void nrm_topo(int iter)
{
    if (mode != 1)
    {
        unsigned int cpu;
        unsigned int node;
        getcpu(&cpu, &node);
        core_dummy_array[iter] = cpu;
        node_dummy_array[iter] = node;
    }
    warmup = 0;
}

void nrm_get_topo()
{
    // CPUs
    for (int i = 0; i < core_dummy_size; i++)
    {
        int var = 0;
        for (int j = 0; j < cvector_size(cores_vector); j++)
        {
            if (cores_vector[j] == core_dummy_array[i])
            {
                var++;
                break;
            }
        }
        if (var == 0)
        {
            cvector_push_back(cores_vector, core_dummy_array[i]);
        }
    }
    // Nodes
    for (int i = 0; i < node_dummy_size; i++)
    {
        int var = 0;
        for (int j = 0; j < cvector_size(nodes_vector); j++)
        {
            if (nodes_vector[j] == node_dummy_array[i])
            {
                var++;
                break;
            }
        }
        if (var == 0)
        {
            cvector_push_back(nodes_vector, node_dummy_array[i]);
        }
    }
    // GPUs
    if (mode != 0)
    {
        for (int i = 0; i < gpu_dummy_size; i++)
        {
            int var = 0;
            for (int j = 0; j < cvector_size(gpus_vector); j++)
            {
                if (gpus_vector[j] == gpu_dummy_array[i])
                {
                    var++;
                    break;
                }
            }
            if (var == 0)
            {
                cvector_push_back(gpus_vector, gpu_dummy_array[i]);
            }
        }
    }
}
