/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://xgitlab.cels.anl.gov/argo/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/
#define _GNU_SOURCE
#include "config.h"
#include "nrm_mpi.h"
#include <inttypes.h>
#include <mpi.h>
#include <stdlib.h>

void FC_FUNC_(mpi_allreduce, MPI_ALLREDUCE)(void *sendbuf, void *recvbuf,
                                            MPI_Fint *count, MPI_Fint *datatype,
                                            MPI_Fint *op, MPI_Fint *comm,
                                            MPI_Fint *ierror) {
  int cn = (int)*count;
  MPI_Datatype dt = MPI_Type_f2c(*datatype);
  MPI_Op o = MPI_Op_f2c(*op);
  MPI_Comm cc = MPI_Comm_f2c(*comm);
  *ierror = (MPI_Fint)NRM_MPI_INNER_NAME(MPI_Allreduce, sendbuf, recvbuf, cn,
                                         dt, o, cc);
}

void FC_FUNC_(mpi_barrier, MPI_BARRIER)(MPI_Fint *comm, MPI_Fint *ierror) {
  MPI_Comm cc = MPI_Comm_f2c(*comm);
  *ierror = (MPI_Fint)NRM_MPI_INNER_NAME(MPI_Barrier, cc);
}

void FC_FUNC_(mpi_comm_size, MPI_COMM_SIZE)(MPI_Fint *comm, MPI_Fint *size,
                                            MPI_Fint *ierror) {
  MPI_Comm cc = MPI_Comm_f2c(*comm);
  *ierror = (MPI_Fint)NRM_MPI_INNER_NAME(MPI_Comm_size, cc, size);
}

void FC_FUNC_(mpi_comm_rank, MPI_COMM_RANK)(MPI_Fint *comm, MPI_Fint *rank,
                                            MPI_Fint *ierror) {
  MPI_Comm cc = MPI_Comm_f2c(*comm);
  *ierror = (MPI_Fint)NRM_MPI_INNER_NAME(MPI_Comm_rank, cc, rank);
}

void FC_FUNC_(mpi_finalize, MPI_FINALIZE)(MPI_Fint *ierror) {
  *ierror = (MPI_Fint)NRM_MPI_INNER_NAME(MPI_Finalize);
}

void FC_FUNC_(mpi_init, MPI_INIT)(MPI_Fint *ierror) {
  *ierror = (MPI_Fint)NRM_MPI_INNER_NAME(MPI_Init, 0, NULL);
}
