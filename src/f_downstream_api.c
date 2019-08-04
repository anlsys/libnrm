/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://xgitlab.cels.anl.gov/argo/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#include "nrm.h"
#include <stdlib.h>
#include <stdint.h>

int f_nrm_ctxt_create_(uintptr_t *ctxt)
{
    *ctxt = (uintptr_t) nrm_ctxt_create();
    return 0;
}

int f_nrm_ctxt_delete_(uintptr_t *ctxt)
{
    return nrm_ctxt_delete(*((struct nrm_context **)ctxt));
}

int f_nrm_init_(uintptr_t *ctxt, char* uuid_in, int* size)
{
    char* uuid = calloc(*size+1, sizeof(char));
    int i, err;
    for (i = 0; i < *size; i++)
    {
      uuid[i] = uuid_in[i];
      if (uuid_in[i] == ' ')
      {
        uuid[i] = 0;
        i = *size;
      }
    }
    uuid[*size] = 0;
    err = nrm_init(*((struct nrm_context **)ctxt), uuid);
    free(uuid);
    return err;
}

int f_nrm_fini_(uintptr_t *ctxt)
{
    return nrm_fini(*((struct nrm_context **)ctxt));
}

int f_nrm_send_progress_(uintptr_t *ctxt, unsigned long *progress)
{
    return nrm_send_progress(*((struct nrm_context **)ctxt), *progress);
}

int f_nrm_send_phase_context_(uintptr_t *ctxt, unsigned int *cpu, 
		unsigned long long int *computeTime)
{
    return nrm_send_phase_context(*((struct nrm_context **)ctxt), *cpu, 
		    *computeTime);
}
