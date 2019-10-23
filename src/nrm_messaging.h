/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://xgitlab.cels.anl.gov/argo/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#define NRM_START_FORMAT "{\"tag\":\"start\", \"container_uuid\": \"%s\", \"application_uuid\": \"%s\"}"
#define NRM_PROGRESS_FORMAT "{\"tag\":\"progress\", \"payload\": %lu, \"application_uuid\": \"%s\"}"
#define NRM_PHASECONTEXT_FORMAT "{\"tag\":\"phasecontext\", \"cpu\": %u, \"aggregation\": %u, \"computetime\": %llu, \"totaltime\": %llu, \"application_uuid\": \"%s\"}"
#define NRM_EXIT_FORMAT "{\"tag\":\"exit\", \"application_uuid\": \"%s\"}"
