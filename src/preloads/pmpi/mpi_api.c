/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the nrm-extra project.
 * For more info, see https://github.com/anlsys/nrm-extra
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

#define _GNU_SOURCE
#include <assert.h>
#include <ctype.h>
#include <dlfcn.h>
#include <mpi.h>
#include <sched.h> // sched_getcpu
#include <stdio.h> // printf
#include <stdlib.h> // exit, atoi

#include "nrm.h"

#include "extra.h"
#include "nrm_mpi.h"

static nrm_client_t *client;
static nrm_scope_t *scope;
static nrm_sensor_t *sensor;
int added;

static char *upstream_uri = "tcp://127.0.0.1";
static int pub_port = 2345;
static int rpc_port = 3456;

NRM_MPI_DECL(MPI_Allreduce,
             int,
             const void *sendbuf,
             void *recvbuf,
             int count,
             MPI_Datatype datatype,
             MPI_Op op,
             MPI_Comm comm)
{
	nrm_time_t nrmtime;

	NRM_MPI_RESOLVE(MPI_Allreduce);
	nrm_time_gettime(&nrmtime);
	nrm_client_send_event(client, nrmtime, sensor, scope, 1);

	int ret = NRM_MPI_REALNAME(MPI_Allreduce, sendbuf, recvbuf, count,
	                           datatype, op, comm);
	nrm_client_send_event(client, nrmtime, sensor, scope, 1);
	return ret;
}

NRM_MPI_DECL(MPI_Barrier, int, MPI_Comm comm)
{
	nrm_time_t nrmtime;

	NRM_MPI_RESOLVE(MPI_Barrier);
	nrm_time_gettime(&nrmtime);
	nrm_client_send_event(client, nrmtime, sensor, scope, 1);

	int ret = NRM_MPI_REALNAME(MPI_Barrier, comm);
	nrm_client_send_event(client, nrmtime, sensor, scope, 1);

	return ret;
}

NRM_MPI_DECL(MPI_Comm_size, int, MPI_Comm comm, int *size)
{
	NRM_MPI_RESOLVE(MPI_Comm_size);
	return NRM_MPI_REALNAME(MPI_Comm_size, comm, size);
}

NRM_MPI_DECL(MPI_Comm_rank, int, MPI_Comm comm, int *rank)
{
	NRM_MPI_RESOLVE(MPI_Comm_rank);
	return NRM_MPI_REALNAME(MPI_Comm_rank, comm, rank);
}

NRM_MPI_DECL(MPI_Finalize, int, void)
{
	NRM_MPI_RESOLVE(MPI_Finalize);
	if (added)
		nrm_client_remove_scope(client, scope);
	nrm_scope_destroy(scope);
	nrm_client_destroy(&client);
	nrm_finalize();
	return NRM_MPI_REALNAME(MPI_Finalize);
}

NRM_MPI_DECL(MPI_Init, int, int *argc, char ***argv)
{

	int ret, rank, cpu;

	NRM_MPI_RESOLVE(MPI_Init);
	ret = NRM_MPI_REALNAME(MPI_Init, argc, argv);
	cpu = sched_getcpu();

	NRM_MPI_INNER_NAME(MPI_Comm_rank, MPI_COMM_WORLD, &rank);

	nrm_init(NULL, NULL);
	nrm_client_create(&client, upstream_uri, pub_port, rpc_port);

	scope = nrm_scope_create("nrm.pmpi.global");
	nrm_scope_threadshared(scope);
	nrm_extra_find_scope(client, &scope, &added);

	char *name = "nrm-mpi-init";
	sensor = nrm_sensor_create(name);
	nrm_client_add_sensor(client, sensor);

	return ret;
}
