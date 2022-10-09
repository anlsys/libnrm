/*******************************************************************************
* Copyright 2022 UChicago Argonne, LLC.
* (c.f. AUTHORS, LICENSE)
*
* This file is part of the libnrm project.
* For more info, see https://github.com/anlsys/libnrm
*
* SPDX-License-Identifier: BSD-3-Clause
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <hwloc.h>

#define debug 1

int get_scopes(struct nrm_daemon_s *my_daemon)
{
    int depth_of_machine;
    int depth_of_osdev;
    char buffer[128];

    hwloc_topology_t topology;

    hwloc_topology_init(&topology);
    hwloc_topology_set_io_types_filter(topology, HWLOC_TYPE_FILTER_KEEP_IMPORTANT);
    hwloc_topology_load(topology);

    depth_of_machine = hwloc_topology_get_depth(topology);
    // Iterating over Machine objects
    for (int i = 0; i < depth_of_machine; i++)
    {
        int number_of_elements_in_depth = hwloc_get_nbobjs_by_depth(topology, i);
        for (int j = 0; j < number_of_elements_in_depth; j++)
        {
            hwloc_obj_type_snprintf(buffer, sizeof(buffer), hwloc_get_obj_by_depth(topology, i, j), 0);
            hwloc_obj_t object = hwloc_get_obj_by_depth(topology, i, j);

            // Warning: HWLoc only provides physical numerotation in bitmaps
            if ((strcmp(buffer, "L2") == 0) || (strcmp(buffer, "L1d") == 0))
            {
                continue;
            }
            else
            {
                nrm_scope_t *this_scope = nrm_scope_create(buffer);
                unsigned bit, bit2;
                // Nodeset
                hwloc_bitmap_foreach_begin(bit, object->nodeset)
                nrm_scope_add(this_scope, 1, bit);
                hwloc_bitmap_foreach_end();
                // Cpuset
                hwloc_bitmap_foreach_begin(bit2, object->cpuset)
                nrm_scope_add(this_scope, 0, bit2);
                hwloc_bitmap_foreach_end();
#ifdef debug
                char tmp[128];
                nrm_scope_snprintf(tmp, sizeof(tmp), this_scope);
                printf("Printing scope from NRM function  : %s\n", tmp);
#endif
            }
        }
    }

    depth_of_osdev = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_OS_DEVICE);
    // Iterating over OS objects
    // OS objects don't have cpusets/nodesets, see https://www.open-mpi.org/projects/hwloc/doc/v2.5.0/a00363.php
    int counter = 0;
	 for (int i = 0; i < depth_of_osdev; i++)
    {
        hwloc_obj_t object = hwloc_get_obj_by_type(topology, HWLOC_OBJ_OS_DEVICE, i);
        assert(object->cpuset == NULL);
        if (object->attr->osdev.type == HWLOC_OBJ_OSDEV_GPU)
        {
            nrm_scope_t *this_scope = nrm_scope_create(object->name);
            nrm_scope_add(this_scope, 2, counter);
				counter++;
#ifdef debug
				char tmp[128];
            nrm_scope_snprintf(tmp, sizeof(tmp), this_scope);
            printf("Printing scope from NRM function  : %s\n", tmp);
#endif
        }
        else if (object->attr->osdev.type == HWLOC_OBJ_OSDEV_COPROC)
        {
            nrm_scope_t *this_scope = nrm_scope_create(object->name);
            nrm_scope_add(this_scope, 2, counter);
				counter++;
#ifdef debug
				char tmp[128];
            nrm_scope_snprintf(tmp, sizeof(tmp), this_scope);
            printf("Printing scope from NRM function  : %s\n", tmp);
#endif
        }
    }

    hwloc_topology_destroy(topology);
    return 0;
}
