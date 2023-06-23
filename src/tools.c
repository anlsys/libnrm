/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include "config.h"

#include "nrm-tools.h"

#include "internal/argtable3.h"

/* parse common command-line arguments, consuming them */
int nrm_tools_parse_common_args(int *argc, char **argv[],
				nrm_tools_common_args_t *args)
{
	struct arg_lit *help, *version;
	struct arg_str *uri;
	struct arg_lit *verbose, *quiet, *log_level;
	struct arg_int *rpc, *pub;
	struct arg_end *end;

	void *argtable[] = {
		help = arg_lit0("h", "help", "display this help and exit"),
		version = arg_lit0("V", "version", "output version information and exit"),
		uri = arg_str0("u", "uri", "zmq uri", "server socket uri"),
		verbose = arg_litn("v", "verbose", 0, NRM_LOG_DEBUG - NRM_LOG_NORMAL,
				   "verbose level (repeat for more verbosity)"),
		quiet = arg_lit0("q", "quiet", "force quiet output"),
		log_level = arg_int0("l", "log-level", "log level (0 is quiet, 5 is debug)"),
		end = arg_end(20),
	};

	int err;
	err = arg_nullcheck(argtable);
	assert(err == 0);

	int nerrors;
	nerrors = arg_parse(*argc, *argv, argtable);

	if (nerrors > 0)
	{
		arg_print_errors(stderr, end, *argv[0]);
		return EXIT_FAILURE;
	}

	if (help->count > 0)
		args->ask_help = 1;
	if (version->count > 0)
		args->ask_version = 1;

	/* handle all the verbosity logic, parsing verbose first, then
	 * log_level, then quiet
	 */
	args->log_level = verbose->count + NRM_LOG_NORMAL;
	if (log_level->count > 0)
		args->log_level = log_level->ival[0];
	if (args->log_level < NRM_LOG_QUIET)
		args->log_level = NRM_LOG_QUIET;
	if (args->log_level > NRM_LOG_DEBUG)
		args->log_level = NRM_LOG_DEBUG;
	/* quiet always forces quiet */
	if (quiet->count > 0)
		args->log_level = NRM_LOG_QUIET;

	/* handle connection options */
	if (uri->count > 0)
		args->uri = nrm_string_fromchar(uri->sval[0]);
	if (rpc->count > 0)
		args->rpc = rpc->ival[0];
	if (pub->count > 0)
		args->pub = pub->ival[0];

	return 0;
}
