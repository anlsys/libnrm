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

#include "nrm.h"

#include "internal/messages.h"
#include "internal/nrmi.h"
#include "internal/roles.h"

static int ask_help = 0;
static int ask_version = 0;
static char *upstream_uri = NRM_DEFAULT_UPSTREAM_URI;
static int pub_port = NRM_DEFAULT_UPSTREAM_PUB_PORT;
static int rpc_port = NRM_DEFAULT_UPSTREAM_RPC_PORT;
static nrm_client_t *client;

static struct option long_options[] = {
        {"help", no_argument, &ask_help, 1},
        {"version", no_argument, &ask_version, 1},
        {"uri", required_argument, NULL, 'u'},
        {"rpc-port", required_argument, NULL, 'r'},
        {"pub-port", required_argument, NULL, 'p'},
        {0, 0, 0, 0},
};

static const char *short_options = "+hVu:r:p:";

static const char *help[] = {"Usage: nrm-dummy-extra [options]\n\n",
                             "Allowed options:\n",
                             "--help, -h    : print this help message\n",
                             "--version, -V : print program version\n", NULL};

void print_help()
{
	for (int i = 0; help[i] != NULL; i++)
		fprintf(stdout, "%s", help[i]);
}

void print_version()
{
	fprintf(stdout, "nrm-dummy-extra: version %s\n", nrm_version_string);
}

int nrm_dummy_extra_callback(nrm_uuid_t *uuid, double value)
{
	nrm_log_debug("action %f\n", value);
	return 0;
}

int main(int argc, char *argv[])
{
	int c;
	int option_index = 0;

	while (1) {
		c = getopt_long(argc, argv, short_options, long_options,
		                &option_index);
		if (c == -1)
			break;
		switch (c) {
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
			fprintf(stderr, "nrm-dummy-extra: missing argument\n");
			exit(EXIT_FAILURE);
		case '?':
			fprintf(stderr, "nrm-dummy-extra: invalid option: %s\n",
			        argv[optind - 1]);
			exit(EXIT_FAILURE);
		default:
			fprintf(stderr,
			        "nrm-dummy-extra: this should not happen\n");
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

	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "nrm-dummy-extra");
	nrm_log_setlevel(NRM_LOG_DEBUG);

	nrm_log_debug("after command line parsing: argc: %u argv[0]: %s\n",
	              argc, argv[0]);

	nrm_log_info("creating client\n");
	nrm_client_create(&client, upstream_uri, pub_port, rpc_port);

	nrm_sensor_t *sensor;
	nrm_scope_t *scope;
	nrm_actuator_t *actuator;
	int err;

	nrm_log_info("creating dummy sensor\n");
	sensor = nrm_sensor_create("nrm-dummy-extra-sensor");
	err = nrm_client_add_sensor(client, sensor);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	nrm_log_info("creating dummy scope\n");
	scope = nrm_scope_create();
	nrm_scope_threadprivate(scope);
	err = nrm_client_add_scope(client, scope);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	nrm_log_info("creating dummy actuator\n");
	actuator = nrm_actuator_create("nrm-dummy-extra-actuator");
	double choices[2] = {0.0, 1.0};
	nrm_actuator_set_choices(actuator, 2, choices);
	nrm_actuator_set_value(actuator, 0.0);
	err = nrm_client_add_actuator(client, actuator);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	nrm_log_info("starting dummy actuate callback\n");
	nrm_client_set_actuate_listener(client, nrm_dummy_extra_callback);
	nrm_client_start_actuate_listener(client);

	uint64_t counter = 0;
	while (true) {
		nrm_time_t time;
		nrm_time_gettime(&time);
		nrm_client_send_event(client, time, sensor, scope, counter++);
		sleep(1);
	}

	nrm_client_destroy(&client);
	nrm_finalize();
	return err;
}
