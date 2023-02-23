/*******************************************************************************
 * Copyright 2022 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include "nrm.h"
#include <hwloc.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int nrm_scope_hwloc_scopes(nrm_hash_t **scopes)
{
	int depth_of_machine;
	int depth_of_osdev;
	char buffer[128];
	char *namespace = "nrm.hwloc.";
	char scope_name[256];
	hwloc_topology_t topology;

	hwloc_topology_init(&topology);
	hwloc_topology_set_io_types_filter(topology,
	                                   HWLOC_TYPE_FILTER_KEEP_IMPORTANT);
	hwloc_topology_load(topology);

	depth_of_machine = hwloc_topology_get_depth(topology);
	// Iterating over Machine objects
	for (int i = 0; i < depth_of_machine; i++) {
		int number_of_elements_in_depth =
		        hwloc_get_nbobjs_by_depth(topology, i);
		for (int j = 0; j < number_of_elements_in_depth; j++) {
			nrm_scope_t *this_scope = NULL;
			hwloc_obj_t object =
			        hwloc_get_obj_by_depth(topology, i, j);
			hwloc_obj_type_snprintf(buffer, sizeof(buffer), object,
			                        0);

			// Warning: HWLoc only provides physical numerotation in
			// bitmaps
			if (!strcmp(buffer, "L2") || !strcmp(buffer, "L1d"))
				continue;

			snprintf(scope_name, sizeof(scope_name), "%s%s.%u",
			         namespace, buffer, object->logical_index);
			this_scope = nrm_scope_create(scope_name);
			unsigned int bit;
			// Nodeset
			hwloc_bitmap_foreach_begin(bit, object->nodeset)
			        nrm_scope_add(this_scope, NRM_SCOPE_TYPE_NUMA,
			                      bit);
			hwloc_bitmap_foreach_end();
			// Cpuset
			hwloc_bitmap_foreach_begin(bit, object->cpuset)
			        nrm_scope_add(this_scope, NRM_SCOPE_TYPE_CPU,
			                      bit);
			hwloc_bitmap_foreach_end();
			nrm_hash_add(scopes, nrm_scope_uuid(this_scope),
			             this_scope);
		}
	}

	depth_of_osdev =
	        hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_OS_DEVICE);
	// Iterating over OS objects
	// OS objects don't have cpusets/nodesets, see
	// https://www.open-mpi.org/projects/hwloc/doc/v2.5.0/a00363.php
	for (int i = 0, counter = 0; i < depth_of_osdev; i++) {
		hwloc_obj_t object =
		        hwloc_get_obj_by_type(topology, HWLOC_OBJ_OS_DEVICE, i);
		assert(object->cpuset == NULL);
		if (object->attr->osdev.type == HWLOC_OBJ_OSDEV_GPU ||
		    object->attr->osdev.type == HWLOC_OBJ_OSDEV_COPROC) {
			nrm_scope_t *this_scope = NULL;
			snprintf(scope_name, sizeof(scope_name), "%s%s",
			         namespace, object->name);
			this_scope = nrm_scope_create(scope_name);
			nrm_scope_add(this_scope, NRM_SCOPE_TYPE_GPU, counter);
			counter++;
			nrm_hash_add(scopes, nrm_scope_uuid(this_scope),
			             this_scope);
		}
	}
	hwloc_topology_destroy(topology);
	return 0;
}

nrm_scope_t *nrm_scope_create_hwloc_allowed(const char *name)
{
	/* retrieve allowed cpus and nodes, and create the corresponding scope
	 */
	hwloc_topology_t topology;
	hwloc_const_cpuset_t cpuset;
	hwloc_const_nodeset_t nodeset;
	hwloc_topology_init(&topology);
	hwloc_topology_set_flags(
	        topology, HWLOC_TOPOLOGY_FLAG_IS_THISSYSTEM &
	                          HWLOC_TOPOLOGY_FLAG_RESTRICT_TO_CPUBINDING &
	                          HWLOC_TOPOLOGY_FLAG_RESTRICT_TO_MEMBINDING);
	hwloc_topology_load(topology);
	cpuset = hwloc_topology_get_allowed_cpuset(topology);
	nodeset = hwloc_topology_get_allowed_nodeset(topology);

	nrm_scope_t *ret = nrm_scope_create(name);
	unsigned int bit;
	hwloc_bitmap_foreach_begin(bit, cpuset)
	        nrm_scope_add(ret, NRM_SCOPE_TYPE_CPU, bit);
	hwloc_bitmap_foreach_end();
	hwloc_bitmap_foreach_begin(bit, nodeset)
	        nrm_scope_add(ret, NRM_SCOPE_TYPE_NUMA, bit);
	hwloc_bitmap_foreach_end();
	return ret;
}
