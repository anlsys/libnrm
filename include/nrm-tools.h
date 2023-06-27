/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#include "nrm.h"

#ifndef NRM_TOOLS_H
#define NRM_TOOLS_H 1

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Command Line Parsing Helpers
 ******************************************************************************/

typedef struct nrm_tools_common_args_s {
	nrm_string_t upstream_uri;
	unsigned int pub_port;
	unsigned int rpc_port;
	int log_level;
	int ask_help;
	int ask_version;
} nrm_tools_common_args_t;

/* parse common command-line arguments, consuming them */
int nrm_tools_parse_common_args(int argc, char *argv[],
				nrm_tools_common_args_t *args);

int nrm_tools_print_common_help(const char *str);

int nrm_tools_print_common_version(const char *str);

typedef struct nrm_tools_extra_args_s {
	double freq;
} nrm_tools_extra_args_t;

#define NRM_TOOLS_EXTRA_ARG_FREQ 1

int nrm_tools_parse_extra_args(int argc, char *argv[],
				nrm_tools_extra_args_t *args, int flags);

#ifdef __cplusplus
}
#endif

#endif
