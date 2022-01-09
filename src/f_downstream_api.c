/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#include "config.h"

#include <stdint.h>
#include <stdlib.h>

#include "nrm.h"

#include "internal-downstream.h"

int FC_FUNC_(nrmf_ctxt_create, NRMF_CTXT_CREATE)(uintptr_t *ctxt)
{
	*ctxt = (uintptr_t)nrm_ctxt_create();
	return 0;
}

int FC_FUNC_(nrmf_ctxt_delete, NRMF_CTXT_DELETE)(uintptr_t *ctxt)
{
	return nrm_ctxt_delete(*((struct nrm_context **)ctxt));
}

int FC_FUNC_(nrmf_init, NRMF_INIT)(
        uintptr_t *ctxt, char *uuid_in, int *size, int *rank_id, int *thread_id)
{
	char *uuid = calloc(*size + 1, sizeof(char));
	int i, err;
	for (i = 0; i < *size; i++) {
		uuid[i] = uuid_in[i];
		if (uuid_in[i] == ' ') {
			uuid[i] = 0;
			i = *size;
		}
	}
	uuid[*size] = 0;
	err = nrm_init(*((struct nrm_context **)ctxt), uuid, *rank_id,
	               *thread_id);
	free(uuid);
	return err;
}

int FC_FUNC_(nrmf_fini, NRMF_FINI)(uintptr_t *ctxt)
{
	return nrm_fini(*((struct nrm_context **)ctxt));
}

int FC_FUNC_(nrmf_send_progress, NRMF_SEND_PROGRESS)(uintptr_t *ctxt,
                                                     unsigned long *progress, 
                                                     int *init, int *array_size,
                                                     int *input_mode,
                                                     int *input_gpu_array[],
                                                     int *input_gpu_size)
{
	return nrm_send_progress(*((struct nrm_context **)ctxt), *progress, *init, *array_size, *input_mode, *input_gpu_array, *input_gpu_size);
}
