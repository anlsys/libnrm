/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://xgitlab.cels.anl.gov/argo/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#include "config.h"
#include "nrm.h"
#include <stdint.h>
#include <stdlib.h>

int FC_FUNC_(f_nrm_ctxt_create, F_NRM_CTXT_CREATE)(uintptr_t *ctxt) {
  *ctxt = (uintptr_t)nrm_ctxt_create();
  return 0;
}

int FC_FUNC_(f_nrm_ctxt_delete, F_NRM_CTXT_DELETE)(uintptr_t *ctxt) {
  return nrm_ctxt_delete(*((struct nrm_context **)ctxt));
}

int FC_FUNC_(f_nrm_init, F_NRM_INIT)(uintptr_t *ctxt, char *uuid_in,
                                     int *size) {
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
  err = nrm_init(*((struct nrm_context **)ctxt), uuid);
  free(uuid);
  return err;
}

int FC_FUNC_(f_nrm_fini, F_NRM_FINI)(uintptr_t *ctxt) {
  return nrm_fini(*((struct nrm_context **)ctxt));
}

int FC_FUNC_(f_nrm_send_progress,
             F_NRM_SEND_PROGRESS)(uintptr_t *ctxt, unsigned long *progress) {
  return nrm_send_progress(*((struct nrm_context **)ctxt), *progress);
}

int FC_FUNC_(f_nrm_send_phase_context,
             F_NRM_SEND_PHASE_CONTEXT)(uintptr_t *ctxt, unsigned int *cpu,
                                       unsigned long long int *computeTime) {
  return nrm_send_phase_context(*((struct nrm_context **)ctxt), *cpu,
                                *computeTime);
}
