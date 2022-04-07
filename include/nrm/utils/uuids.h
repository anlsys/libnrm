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

#define NRM_UUID_SIZE 16

struct nrm_uuid_s {
	char data[NRM_UUID_SIZE];
};

typedef struct nrm_uuid_s nrm_uuid_t;

nrm_uuid_t *nrm_uuid_create();

void nrm_uuid_destroy(nrm_uuid_t **);

nrm_uuid_t *nrm_uuid_create_fromchar(char *s);

const char *nrm_uuid_to_char(nrm_uuid_t *uuid);

#endif
