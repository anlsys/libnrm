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

#include "nrm.h"

#include "internal/nrmi.h"
#include "internal/messages.h"
#include "internal/roles.h"

#include <getopt.h>
#include <czmq.h>

int nrmd_shim_monitor_read_callback(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)arg;
	(void)loop;
	nrm_msg_t *msg;
	int msg_type;

	msg = nrm_ctrlmsg_recv(socket, &msg_type);
	nrm_msg_print(stdout, msg);
	nrm_msg_destroy(&msg);
	return 0;
}

static int ask_help = 0;
static int ask_version = 0;

static struct option long_options[] = {
	{ "help", no_argument, &ask_help, 1 },
	{ "version", no_argument, &ask_version, 1},
	{ 0, 0, 0, 0},
};

static const char *short_options = ":hV";

static const char *help[] = {
	"Usage: nrmd-shim [options]\n\n",
	"Allowed options:\n",
	"--help, -h    : print this help message\n",
	"--version, -V : print program version\n",
	NULL
};

void print_help()
{
	for (int i = 0; help[i] != NULL; i++)
		fprintf(stdout, "%s", help[i]);
}

void print_version()
{
	fprintf(stdout,"nrmd-shim: version %s\n", nrm_version_string);
}

int main(int argc, char *argv[])
{
	int c;
	int option_index = 0;

	while(1) {
		c = getopt_long(argc, argv, short_options, long_options,
				&option_index);
		if (c == -1)
			break;
		switch(c) {
		case 0:
			break;
		case 'h':
			ask_help = 1;
			break;
		case 'V':
			ask_version = 1;
			break;
		case '?':
			fprintf(stderr,"nrmd-shim: invalid options: %s\n",
				argv[optind-1]);
			exit(EXIT_FAILURE);
		default:
			fprintf(stderr,"nrmd_shim: this should not happen\n");
			exit(EXIT_FAILURE);
		}
	}

	/* remove the parsed part */
	argc -= optind;
	argv = &(argv[optind]);

	if (ask_help) {
		print_help();
		exit(EXIT_SUCCESS);
	}
	if (ask_version) {
		print_version();
		exit(EXIT_SUCCESS);
	}

	if(argc == 0) {
		print_help();
		exit(EXIT_FAILURE);
	}

	assert(!strcmp(argv[0], "fromenv"));
	/* create a monitor:
	 *  - this is a broker that listen to incoming messages
	 *  - we have our own loop to listen to it, no on a different thread
	 *  though, as we only have this to take care of
	 */
	struct nrm_role_monitor_s *monitor =
		nrm_role_monitor_create_fromenv();
	assert(monitor != NULL);

	zloop_t *loop = zloop_new();
	assert(loop != NULL);

	zsock_t *pipe = (zsock_t *)nrm_role_monitor_broker(monitor);
	zloop_reader(loop, pipe,
		     nrmd_shim_monitor_read_callback, NULL);
	zloop_start(loop);	

	return 0;
}