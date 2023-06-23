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

/* parse common command-line arguments, consuming them */
int nrm_tools_parse_common_args(int *argc, char **argv[],
				nrm_tools_common_args_t *args)
{
	static const char *shortopts = "+vh:u:r:p:";
	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"log-level", required_argument, 0, 'l'},
		{"quiet", no_argument, 0, 'q'},
		{"verbose", no_argument, 0, 'v'},
		{"uri", required_argument, 0, 'u'},
		{"rpc", required_argument, 0, 'r'},
		{"pub", required_argument, 0, 'p'},
		{0, 0, 0, 0}
	};
	while (1) {
		int option_index = 0;
		int c = getopt_long(*argc, *argv, shortopts, long_options,
		                &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'f':
			errno = 0;
			args->freq = strtod(optarg, NULL);
			if (errno != 0) {
				fprintf(stderr,
					"Error during conversion to double: %d\n",
				        errno);
				return 1;
			}
			if (args->freq <= 0) {
				fprintf(stderr,
					"Wrong frequency value: %f\n",
					args->freq);
				return 1;
			}
			break;
		case 'h':
			args->help = 1;
			break;
		case 'p':
			errno = 0;
			args->pub_port = (int) strtoul(optarg, NULL, 10);
			if (errno != 0) {
				fprintf(stderr,
					"Error during conversion to int: %d\n",
					errno);
				return 1;
			}
			if (args->pub_port <= 0) {
				fprintf(stderr,
					"Wrong port value: %d\n",
					args->pub_port);
				return 1;
			}
			break;
		case 'r':
			errno = 0;
			args->rpc_port = (int) strtoul(optarg, NULL, 10);
			if (errno != 0) {
				fprintf(stderr,
					"Error during conversion to int: %d\n",
					errno);
				return 1;
			}
			if (args->rpc_port <= 0) {
				fprintf(stderr,
					"Wrong port value: %d\n",
					args->rpc_port);
				return 1;
			}
			break;
		case 'u':
			args->upstream_uri = strdup(optarg);
			break;
		case 'v':
			args->log_level = NRM_LOG_DEBUG;
			break;
		case '?':
		default:
			fprintf(stderr, "Wrong option argument\n");
			return 1;
		}
	}

	return 0;
}
