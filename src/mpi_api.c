/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://xgitlab.cels.anl.gov/argo/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

/* Filename: mpi_api.c
 *
 * Description: This Message Passing Interface(MPI) libary allows
 * application of runtime policies for energy efficiency through the MPI
 * standard profiling interface(PMPI).
 *
 * The current implementation passes phase contextual information (compute
 * time and barrier time) to the Argo Node Resource Manager(NRM).
 * The NRM using this information invokes power policies to improve energy
 * efficiency of the node.
 */

#include <ctype.h>
#include <sched.h>      // sched_getcpu
#include <stdio.h>      // printf
#include <stdlib.h>     // exit, atoi
#include <mpi.h>

#include "nrm.h"

static unsigned int _transmit = 0;
static unsigned int cpu;
static int rank;
static uint64_t computeTime;
static struct nrm_context ctxt;
struct timespec now;

int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    clock_gettime(CLOCK_MONOTONIC, &now);
    computeTime = nrm_gettime(&ctxt, now)
                  int ret_value = PMPI_Allreduce(sendbuf, recvbuf, count, datatype, op,
                                  comm);

    if(_transmit)
        nrm_send_phase_context(&ctxt, cpu, computeTime);
    return ret_value;
}

int MPI_Barrier(MPI_Comm comm)
{
    clock_gettime(CLOCK_MONOTONIC, &now);
    computeTime = nrm_timediff(&ctxt, now);
    int ret_value = PMPI_Barrier(comm);

    if(_transmit)
        nrm_send_phase_context(&ctxt, cpu, computeTime);
    return ret_value;
}

int MPI_Finalize(void)
{
    // Cleanup NRM context
    nrm_fini(&ctxt);
    return PMPI_Finalize();
}

int MPI_Init(int *argc, char ***argv)
{
    int ret_value = PMPI_Init(argc, argv);

    cpu = sched_getcpu();
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(getenv("NRM_TRANSMIT"))
        _transmit = atoi(getenv("NRM_TRANSMIT"));

    // Initialize context to communicate with Argo Node Resource Manager(NRM)
    nrm_init(&ctxt, (*argv)[0]);
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    &ctxt->time = now;
    return ret_value;
}
