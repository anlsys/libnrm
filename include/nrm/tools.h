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

#define NRM_TOOLS_ARGS_FLAG_FREQ 1
#define NRM_TOOLS_ARGS_FLAG_MAX 1

typedef struct nrm_tools_args_s {
	/* needs to be set ahead of time */
	char *progname;
	int flags;
	/* common options */
	nrm_string_t upstream_uri;
	unsigned int pub_port;
	unsigned int rpc_port;
	int log_level;
	int ask_help;
	int ask_version;
	/* beginning of extra options */
	double freq;
} nrm_tools_args_t;

/* parse command-line arguments, consuming them,
 *
 * returns the index of first unrecognized option or positional argument.
 */
int nrm_tools_parse_args(int argc, char *argv[], nrm_tools_args_t *args);

int nrm_tools_print_help(const nrm_tools_args_t *args);

int nrm_tools_print_version(const nrm_tools_args_t *args);

#ifdef __cplusplus
}
#endif

#endif
