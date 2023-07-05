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

#include <getopt.h>

#include "nrm-tools.h"
#include "internal/nrmi.h"

/* parse common command-line arguments, consuming them */
int nrm_tools_parse_common_args(int argc, char *argv[],
				nrm_tools_common_args_t *args)
{
	static const char *shortopts = "+:vhqVl:u:r:p:";
	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'V'},
		{"log-level", required_argument, 0, 'l'},
		{"quiet", no_argument, 0, 'q'},
		{"verbose", no_argument, 0, 'v'},
		{"uri", required_argument, 0, 'u'},
		{"rpc", required_argument, 0, 'r'},
		{"pub", required_argument, 0, 'p'},
		{0, 0, 0, 0}
	};

	/* default values */
	args->ask_help = 0;
	args->ask_version = 0;
	args->log_level = NRM_LOG_NORMAL;
	args->pub_port = NRM_DEFAULT_UPSTREAM_PUB_PORT;
	args->rpc_port = NRM_DEFAULT_UPSTREAM_RPC_PORT;
	args->upstream_uri = NULL;

	int done = 0;
	while (!done) {
		int err;
		int option_index = 0;
		int c = getopt_long(argc, argv, shortopts, long_options,
		                &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'h':
			args->ask_help = 1;
			break;
		case 'V':
			args->ask_version = 1;
			break;
		case 'p':
			err = nrm_parse_uint(optarg, &args->pub_port);
			if (err) {
				nrm_log_error("Can't parse 'p' option argument: '%s'\n", optarg);
				return err;
			}
			break;
		case 'r':
			err = nrm_parse_uint(optarg, &args->rpc_port);
			if (err) {
				nrm_log_error("Can't parse 'r' option argument: '%s'\n", optarg);
				return err;
			}
			break;
		case 'u':
			args->upstream_uri = nrm_string_fromchar(optarg);
			break;
		case 'v':
			args->log_level++;
			break;
		case 'q':
			args->log_level = NRM_LOG_QUIET;
			break;
		case 'l':
			err = nrm_parse_int(optarg, &args->log_level);
			if (err) {
				nrm_log_error("Can't parse 'l' option argument: '%s'\n", optarg);
				return err;
			}
			break;
		case '?':
			/* we just saw an unknown option, stop here and return
			 * to user for extra parsing */
			optind--;
			done = 1;
			break;
		case ':':
		default:
			nrm_log_error("Wrong option argument\n");
			return -NRM_EINVAL;
		}
	}
	if (args->upstream_uri == NULL)
		args->upstream_uri =
			nrm_string_fromchar(NRM_DEFAULT_UPSTREAM_URI);

	return optind;
}

int nrm_tools_print_common_help(const char *str)
{
	static const char *help[] = {
		"Allowed options:\n",
		"--help, -h             : print this help message\n",
		"--version, -V          : print program version\n",
		"--verbose, -v          : increase verbosity\n",
		"--quiet, -q            : no log output\n",
		"--log-level, -l <int>  : set log level (0-5)\n",
		"--uri, -u <str>        : daemon socket uri to connect to\n",
		"--rpc-port, -r  <uint> : daemon rpc port to use\n",
		"--pub-port, -p  <uint> : daemon pub/sub port to use\n",
		NULL};

	fprintf(stdout, "Usage: %s [options]\n\n", str);
	for (int i = 0; help[i] != NULL; i++)
		fprintf(stdout, "%s", help[i]);
	return 0;
}

int nrm_tools_print_common_version(const char *str)
{
	fprintf(stdout, "%s: version %s\n", str, nrm_version_string);
	return 0;
}

int nrm_tools_parse_extra_args(int argc, char *argv[], nrm_tools_extra_args_t
			       *args, int flags)
{
	static const char *shortopts = "+f:";
	static struct option long_options[] = {
		{"freq", required_argument, 0, 'f'},
		{0, 0, 0, 0}
	};

	/* reset getopt */
	optind = 1;

	/* default values */
	args->freq = 1.0;

	while (1) {
		int err;
		int option_index = 0;
		int c = getopt_long(argc, argv, shortopts, long_options,
		                &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'f':
			if ((flags & NRM_TOOLS_EXTRA_ARG_FREQ) == 0) {
				nrm_log_error("unexpected option freq\n");
				return -NRM_EINVAL;
			}
			err = nrm_parse_double(optarg, &args->freq);
			if (err) {
				nrm_log_error("Error during frequency parsing of '%s'\n",
					optarg);
				return err;
			}
			break;
		case '?':
		default:
			nrm_log_error("Wrong option argument\n");
			return -NRM_EINVAL;
		}
	}
	return optind;
}
