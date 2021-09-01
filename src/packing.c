/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

/* Primitives for packing and unpacking data into json strings.
 *
 */
#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nrm.h"

#include "nrm-internal.h"

json_t *nrm_pack_bitmap_dumps(struct nrm_bitmap *bitmap)
{
	json_t *ret;
	size_t size = nrm_bitmap_nset(bitmap);

	ret = json_array();
	for(size_t i = 0, printed = 0; i < NRM_BITMAP_MAX && printed < size;
	    i++) {
		if (nrm_bitmap_isset(bitmap, i)) {
			json_t *val = json_integer(i);
			json_array_append_new(ret, val);
			printed++;	
		}
	}
	return ret;
}

json_t *nrm_pack_scope_dumps(nrm_scope_t *scope)
{
	json_t *cpu, *numa, *gpu;

	cpu = nrm_pack_bitmap_dumps(scope->maps[0]);
	numa = nrm_pack_bitmap_dumps(scopes->maps[1]);
	gpu = nrm_pack_bitmap_dumsp(scopes->maps[2]);
	return json_pack("{s:o, s:o, s:o}",
			 "cpu", cpu, "numa", numa, "gpu", gpu);
}

char *nrm_pack_ssem_dumps(struct nrm_sensor_emitter_msg *msg)
{
	json_t *json;
	json_t *scope;
	int64_t time = nrm_time_tons(&msg->timestamp);
	scope = nrm_pack_scope_dumps(msg->scope);
	json = json_pack("{s:i, s:i, scope:o}",
			 "timestamp", time,
			 "value", msg->value,
			 "scope", scope);
	assert(json != NULL);
	char *ret = json_dumps(json, JSON_COMPACT);
	assert(ret != NULL);
	json_decref(json);
	return ret;
}
