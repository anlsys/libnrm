/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

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
#include "nrm.h"
#include <assert.h>
#include <ctype.h>
#include <dlfcn.h>
#include <mpi.h>
#include <sched.h> // sched_getcpu
#include <stdio.h> // printf
#include <stdlib.h> // exit, atoi

#include "nrm_mpi.h"

static nrm_client_t *client;
static nrm_scope_t *scope;
static nrm_sensor_t *sensor;

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
	nrm_scope_destroy(scope);
	nrm_client_destroy(&client);
	nrm_finalize();
	return NRM_MPI_REALNAME(MPI_Finalize);
}

int find_scope(nrm_client_t *client, int rank, nrm_scope_t **scope)
{
	/* create a scope based on current processor,
	 */
	nrm_string_t name = nrm_string_fromprintf("nrm.pmpi.%u", rank);
	nrm_scope_t *ret = nrm_scope_create(name);
	nrm_string_decref(name);
	nrm_scope_threadshared(ret);

	nrm_vector_t *nrmd_scopes;
	nrm_client_list_scopes(client, &nrmd_scopes);

	size_t numscopes;
	int newscope = 0;
	nrm_vector_length(nrmd_scopes, &numscopes);
	for (size_t i = 0; i < numscopes; i++) {
		nrm_scope_t *s;
		nrm_vector_pop_back(nrmd_scopes, &s);
		if (!nrm_scope_cmp(s, ret)) {
			nrm_scope_destroy(ret);
			ret = s;
			newscope = 1;
			continue;
		}
		nrm_scope_destroy(s);
	}
	if (!newscope) {
		nrm_log_error("Could not find an existing scope to match\n");
		return -NRM_EINVAL;
	}
	nrm_vector_destroy(&nrmd_scopes);
	*scope = ret;
	return 0;
}

NRM_MPI_DECL(MPI_Init, int, int *argc, char ***argv)
{

	int ret, rank;

	NRM_MPI_RESOLVE(MPI_Init);
	ret = NRM_MPI_REALNAME(MPI_Init, argc, argv);

	NRM_MPI_INNER_NAME(MPI_Comm_rank, MPI_COMM_WORLD, &rank);

	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "nrm.pmpi");
	nrm_client_create(&client, nrm_upstream_uri, nrm_upstream_pub_port,
	                  nrm_upstream_rpc_port);

	find_scope(client, rank, &scope);

	nrm_string_t name = nrm_string_fromprintf("nrm.pmpi.%u", rank);
	sensor = nrm_sensor_create(name);
	nrm_string_decref(name);
	nrm_client_add_sensor(client, sensor);

	return ret;
}
