/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#ifndef NRM_UUIDS_H
#define NRM_UUIDS_H 1

/* our uuids are a bit special, since they're stored in their string form to
 * avoid issues related to printing and pack/unpack
 */
typedef nrm_string_t nrm_uuid_t;

nrm_uuid_t *nrm_uuid_create();

void nrm_uuid_destroy(nrm_uuid_t **);

nrm_uuid_t *nrm_uuid_create_fromchar(char *s);

nrm_uuid_t *nrm_uuid_create_frombuf(char *s, size_t);

char *nrm_uuid_to_char(nrm_uuid_t *uuid);

nrm_string_t nrm_uuid_to_string(nrm_uuid_t *uuid);

int nrm_uuid_cmp(const nrm_uuid_t one, const nrm_uuid_t two);

#endif
