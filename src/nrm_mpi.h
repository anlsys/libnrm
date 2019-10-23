/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://xgitlab.cels.anl.gov/argo/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#ifndef NRM_MPI_H
#define NRM_MPI_H 1

#ifdef __cplusplus
extern "C" {
#endif

#define NRM_MPI_INNER_NAME(fname, ...) nrm_ ## fname(__VA_ARGS__)

#define NRM_MPI_REALNAME(fname, ...) __nrm_real_ ## fname(__VA_ARGS__)

#define NRM_MPI_DECL(fname, ret_type, ...)    \
    static ret_type (*__nrm_real_ ## fname )(__VA_ARGS__) = NULL;    \
    ret_type nrm_ ## fname(__VA_ARGS__)

#define NRM_MPI_RESOLVE(fname) \
    if (__nrm_real_ ## fname == NULL) {    \
        __nrm_real_ ## fname = dlsym(RTLD_NEXT, "P" #fname);    \
        if(__nrm_real_ ## fname == NULL) {    \
            __nrm_real_ ## fname = dlsym(RTLD_NEXT, #fname);    \
            assert(__nrm_real_ ## fname != NULL);    \
        }    \
    }


int NRM_MPI_INNER_NAME(MPI_Allreduce, const void *sendbuf, void *recvbuf,
		       int count, MPI_Datatype datatype, MPI_Op op,
		       MPI_Comm comm);
int NRM_MPI_INNER_NAME(MPI_Barrier, MPI_Comm comm);
int NRM_MPI_INNER_NAME(MPI_Comm_size, MPI_Comm comm, int *size);
int NRM_MPI_INNER_NAME(MPI_Comm_rank, MPI_Comm comm, int *rank);
int NRM_MPI_INNER_NAME(MPI_Finalize, void);
int NRM_MPI_INNER_NAME(MPI_Init, int *argc, char ***argv);

#ifdef __cplusplus
}
#endif

#endif
