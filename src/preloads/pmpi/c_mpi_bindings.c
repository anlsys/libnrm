/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include <mpi.h>

#include "nrm_mpi.h"

int MPI_Allreduce(const void *sendbuf,
                  void *recvbuf,
                  int count,
                  MPI_Datatype datatype,
                  MPI_Op op,
                  MPI_Comm comm)
{
	return NRM_MPI_INNER_NAME(MPI_Allreduce, sendbuf, recvbuf, count,
	                          datatype, op, comm);
}

int MPI_Barrier(MPI_Comm comm)
{
	return NRM_MPI_INNER_NAME(MPI_Barrier, comm);
}

int MPI_Comm_size(MPI_Comm comm, int *size)
{
	return NRM_MPI_INNER_NAME(MPI_Comm_size, comm, size);
}

int MPI_Comm_rank(MPI_Comm comm, int *rank)
{
	return NRM_MPI_INNER_NAME(MPI_Comm_rank, comm, rank);
}

int MPI_Finalize(void)
{
	return NRM_MPI_INNER_NAME(MPI_Finalize);
}

int MPI_Init(int *argc, char ***argv)
{
	return NRM_MPI_INNER_NAME(MPI_Init, argc, argv);
}
