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

#include "nrm.h"
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal/nrmi.h"

nrm_uuid_t *nrm_uuid_create()
{
	zuuid_t *u;
	nrm_uuid_t *ret;
	ret = calloc(1, sizeof(nrm_uuid_t));
	if (ret == NULL)
		return NULL;
	u = zuuid_new();
	assert(u);
	*ret = nrm_string_fromchar(zuuid_str(u));
	zuuid_destroy(&u);
	return ret;
}

void nrm_uuid_destroy(nrm_uuid_t **uuid)
{
	if (uuid == NULL || *uuid == NULL)
		return;

	nrm_string_decref(**uuid);
	free(*uuid);
	*uuid = NULL;
}

nrm_uuid_t *nrm_uuid_create_fromchar(char *s)
{
	nrm_uuid_t *ret;
	ret = calloc(1, sizeof(nrm_uuid_t));
	if (ret == NULL)
		return NULL;
	*ret = nrm_string_fromchar(s);
	return ret;
}

json_t *nrm_uuid_to_json(nrm_uuid_t *uuid)
{
	if (uuid == NULL)
		return NULL;
	return json_string(*uuid);
}

char *nrm_uuid_to_char(nrm_uuid_t *uuid)
{
	if (uuid == NULL)
		return NULL;
	return *uuid;
}

nrm_string_t nrm_uuid_to_string(nrm_uuid_t *uuid)
{
	if (uuid == NULL)
		return NULL;
	nrm_string_incref(*uuid);
	return *uuid;
}

int nrm_uuid_cmp(const nrm_uuid_t one, const nrm_uuid_t two)
{
	return nrm_string_cmp(one, two);
}
