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

static int ask_help = 0;
static int ask_version = 0;
static char *upstream_uri = NRM_DEFAULT_UPSTREAM_URI;
static int pub_port = NRM_DEFAULT_UPSTREAM_PUB_PORT;
static int rpc_port = NRM_DEFAULT_UPSTREAM_RPC_PORT;

static struct option long_options[] = {
	{ "help", no_argument, &ask_help, 1 },
	{ "version", no_argument, &ask_version, 1},
	{ "uri", required_argument, NULL, 'u'},
	{ "rpc-port", required_argument, NULL, 'r'},
	{ "pub-port", required_argument, NULL, 'p'},
	{ 0, 0, 0, 0},
};

static const char *short_options = "+hVu:r:p:";

static const char *help[] = {
	"Usage: nrmc [options]\n\n",
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
	fprintf(stdout,"nrmc: version %s\n", nrm_version_string);
}

struct client_cmd {
	const char *name;
	int (*fn)(int, char **);
};

int cmd_run(int argc, char **argv) {

	char *manifest_name = NULL;
	char *slice_name = NULL;
	static struct option cmd_run_long_options[] = {
		{ "manifest", required_argument, NULL, 'm' },
		{ "slice", required_argument, NULL, 's' },
		{ 0, 0, 0, 0},
	};

	static const char *cmd_run_short_options = ":m:s:";


	int c;
	int option_index = 0;
	while(1) {
		c = getopt_long(argc, argv, cmd_run_short_options,
				cmd_run_long_options,
				&option_index);
		if (c == -1)
			break;
		switch(c) {
		case 0:
			break;
		case 'm':
			manifest_name = optarg;
			break;
		case 's':
			slice_name = optarg;
			break;
		case '?':
			return EXIT_FAILURE;
		default:
			return EXIT_FAILURE;
		}
	}
	/* remove the parsed part */
	argc -= optind;
	argv = &(argv[optind]);

	/* ensure we have something here, otherwise there's no command to launch
	 */
	if (argc < 0)
		return EXIT_FAILURE;

	fprintf(stderr, "%s, %s, %d, %s\n", manifest_name, slice_name,
		argc, argv[0]);

	/* create a client, send the request for a new slice, wait for an answer
	 */
	/*
	nrm_role_t *client = nrm_role_client_create_fromparams(upstream_uri, pub_port,
							rpc_port);
	nrm_role_client_send_new_slice(slice_name);
	nrm_role_client_recv_new_slice(&slice_uuid);

	err = execvp(argv[0], &argv[0]);
	return err;
	*/
	return 0;
}

int cmd_add_slice(int argc, char **argv) {


}

int cmd_list_slices(int argc, char **argv) {


	/* no options at this time */
	(void)argc;
	(void)argv;

	nrm_log_info("creating client\n");

	nrm_role_t *client = nrm_role_client_create_fromparams(upstream_uri,
							       pub_port,
							       rpc_port);
	nrm_log_info("sending request\n");
	/* craft the message we want to send */
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_LIST);
	nrm_msg_set_list_slices(msg, NULL);
	nrm_role_send(client, msg, NULL);

	/* wait for the answer */
	nrm_log_info("receiving reply\n");
	msg = nrm_role_recv(client, NULL);
	nrm_log_printmsg(LOG_DEBUG, msg);
	nrm_role_destroy(&client);
	return 0;
}

static struct client_cmd commands[] = {
	{ "list-slices", cmd_list_slices },
	{ "run", cmd_run },
	{ 0, 0 },
};

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
		case 'p':
			errno = 0;
			pub_port = strtol(optarg, NULL, 0);
			assert(errno == 0);
			break;
		case 'r':
			errno = 0;
			rpc_port = strtol(optarg, NULL, 0);
			assert(errno == 0);
			break;
		case 'u':
			upstream_uri = optarg;
			break;
		case 'V':
			ask_version = 1;
			break;
		case ':':
			fprintf(stderr,"nrmc: missing argument\n");
			exit(EXIT_FAILURE);
		case '?':
			fprintf(stderr,"nrmc: invalid option: %s\n",
				argv[optind-1]);
			exit(EXIT_FAILURE);
		default:
			fprintf(stderr,"nrmc: this should not happen\n");
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

	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "nrmc");
	nrm_log_setlevel(NRM_LOG_DEBUG);

	int err = 0;
	for(int i = 0; commands[i].name != NULL; i++) {
		if(!strcmp(argv[0], commands[i].name)) {
		   err = commands[i].fn(argc, argv);
		   goto end;
		}
	}
	fprintf(stderr, "wrong command: %s\n", argv[0]);
	err = EXIT_FAILURE;
end:	
	nrm_finalize();
	return err;
}
