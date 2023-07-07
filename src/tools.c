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

#include "nrm/tools.h"
#include <getopt.h>

#include "internal/nrmi.h"

int nrm_tools_parse_args(int argc, char *argv[], nrm_tools_args_t *args)
{
	static const char common_shortopts[] = "+:vhqVl:u:r:p:";
	static struct option common_longopts[] = {
	        {"help", no_argument, 0, 'h'},
	        {"version", no_argument, 0, 'V'},
	        {"log-level", required_argument, 0, 'l'},
	        {"quiet", no_argument, 0, 'q'},
	        {"verbose", no_argument, 0, 'v'},
	        {"uri", required_argument, 0, 'u'},
	        {"rpc", required_argument, 0, 'r'},
	        {"pub", required_argument, 0, 'p'},
	        {0, 0, 0, 0}};
	/* keep the short_size like strlen, but number of elements for long_size
	 */
	size_t common_short_size = sizeof(common_shortopts);
	size_t common_long_size =
	        sizeof(common_longopts) / sizeof(common_longopts[0]);

	/* as much as we would like this to stay simple, we need to handle
	 * extra options at the same time.
	 *
	 * The way we do that is by building a list of extra options to parse at
	 * runtime.
	 */

	/* at most two extra characters per extra option */
	size_t full_short_size =
	        common_short_size + (2 * NRM_TOOLS_ARGS_FLAG_MAX);
	size_t full_long_size = common_long_size + NRM_TOOLS_ARGS_FLAG_MAX;
	char full_shortopts[full_short_size];
	struct option full_longopts[full_long_size];

	/* make sure there's nothing in there */
	memset(full_shortopts, 0, full_short_size);
	memset(full_longopts, 0, full_long_size * sizeof(struct option));

	/* set to the common ones */
	strcpy(full_shortopts, common_shortopts);
	/* last field of common is {0,0,0,0} */
	memcpy(full_longopts, common_longopts,
	       common_long_size * sizeof(struct option));

	/* handle flags */
	if ((args->flags & NRM_TOOLS_ARGS_FLAG_FREQ) != 0) {
		args->freq = 1.0;
		char *fs = "f:";
		struct option fl = {"freq", required_argument, 0, 'f'};
		/* it's okay, we know we have enough room */
		strcat(full_shortopts, fs);
		full_longopts[common_long_size - 1] = fl;
	}

	/* default values */
	args->ask_help = 0;
	args->ask_version = 0;
	args->log_level = NRM_LOG_NORMAL;
	args->pub_port = NRM_DEFAULT_UPSTREAM_PUB_PORT;
	args->rpc_port = NRM_DEFAULT_UPSTREAM_RPC_PORT;
	args->upstream_uri = NULL;

	/* make sure the parser is reset */
	optind = 1;

	/* parsing itself */
	while (1) {
		int err;
		int option_index = 0;
		int c = getopt_long(argc, argv, full_shortopts, full_longopts,
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
				nrm_log_error(
				        "Can't parse 'p' option argument: '%s'\n",
				        optarg);
				return err;
			}
			break;
		case 'r':
			err = nrm_parse_uint(optarg, &args->rpc_port);
			if (err) {
				nrm_log_error(
				        "Can't parse 'r' option argument: '%s'\n",
				        optarg);
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
				nrm_log_error(
				        "Can't parse 'l' option argument: '%s'\n",
				        optarg);
				return err;
			}
			break;
		/* beginning of extra options */
		case 'f':
			assert((args->flags & NRM_TOOLS_ARGS_FLAG_FREQ) != 0);
			err = nrm_parse_double(optarg, &args->freq);
			if (err) {
				nrm_log_error(
				        "Error during frequency parsing of '%s'\n",
				        optarg);
				return err;
			}
			break;
		case ':':
			nrm_log_error(
			        "Missing command line argument after: '%s'\n",
			        argv[optind - 1]);
			return -NRM_EINVAL;
		case '?':
			nrm_log_error("Wrong command line option: '%s'\n",
			              argv[optind - 1]);
			return -NRM_EINVAL;
		default:
			nrm_log_error("Logic error in option parser\n");
			return -NRM_EINVAL;
		}
	}
	if (args->upstream_uri == NULL)
		args->upstream_uri =
		        nrm_string_fromchar(NRM_DEFAULT_UPSTREAM_URI);

	return optind;
}

int nrm_tools_print_help(const nrm_tools_args_t *args)
{
	static const char *common_help[] = {
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
	const char *freq_help =
	        "--freq, -f <double>    : signal frequency (in Hz)\n";
	fprintf(stdout, "Usage: %s [options]\n\n", args->progname);
	for (int i = 0; common_help[i] != NULL; i++)
		fprintf(stdout, "%s", common_help[i]);
	if ((args->flags & NRM_TOOLS_ARGS_FLAG_FREQ) != 0) {
		fprintf(stdout, "%s", freq_help);
	}
	return 0;
}

int nrm_tools_print_version(const nrm_tools_args_t *args)
{
	fprintf(stdout, "%s: version %s\n", args->progname, nrm_version_string);
	return 0;
}
