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

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nrm.h"

#include "internal/nrmi.h"

nrm_uuid_t *nrm_uuid_create()
{
	zuuid_t *u;
	nrm_uuid_t *ret;
	ret = calloc(1, sizeof(nrm_uuid_t));
	if (ret == NULL)
		return NULL;

	u = zuuid_new();
	assert(zuuid_size(u) == 16);
	memcpy(ret->data, zuuid_data(u), 16);
	zuuid_destroy(&u);
	return ret;
}

void nrm_uuid_destroy(nrm_uuid_t **uuid)
{
	if (uuid == NULL || *uuid == NULL)
		return;

	free(*uuid);
	*uuid = NULL;
}

nrm_uuid_t *nrm_uuid_create_fromchar(char *s)
{
	 nrm_uuid_t *ret;
	 ret = calloc(1, sizeof(nrm_uuid_t));
	 if (ret == NULL)
		 return NULL;
	 assert(strnlen(s, 17) == 16);
	 memcpy(ret->data, s, 16);
	 return ret;
}

json_t *nrm_uuid_to_json(nrm_uuid_t *uuid)
{
	zuuid_t *u;
	u = zuuid_new_from((const byte *)uuid->data);
	json_t *ret = json_pack("s", zuuid_str(u));
	zuuid_destroy(&u);
	return ret;
}
