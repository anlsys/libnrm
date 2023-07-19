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
#include "nrm/tools.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static nrm_client_t *client;

int nrm_dummy_extra_callback(nrm_uuid_t *uuid, double value)
{
	(void)uuid;
	nrm_log_debug("action %f\n", value);
	return 0;
}

int main(int argc, char *argv[])
{
	int err;
	nrm_tools_args_t args;

	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "nrm-dummy-extra");

	args.progname = "nrm-dummy-extra";
	args.flags = NRM_TOOLS_ARGS_FLAG_FREQ;

	err = nrm_tools_parse_args(argc, argv, &args);
	if (err < 0) {
		nrm_log_error("errors during argument parsing\n");
		nrm_tools_print_help(&args);
		exit(EXIT_FAILURE);
	}

	if (args.ask_help) {
		nrm_tools_print_help(&args);
		exit(EXIT_SUCCESS);
	}
	if (args.ask_version) {
		nrm_tools_print_version(&args);
		exit(EXIT_SUCCESS);
	}

	nrm_log_setlevel(args.log_level);

	nrm_log_debug("frequency setting: %f\n", args.freq);
	nrm_log_debug("after command line parsing: argc: %u argv[0]: %s\n",
	              argc, argv[0]);

	nrm_log_info("creating client\n");
	err = nrm_client_create(&client, args.upstream_uri, args.pub_port,
	                        args.rpc_port);
	if (err) {
		nrm_log_error("error during client creation: %d\n", err);
		return EXIT_FAILURE;
	}

	nrm_sensor_t *sensor;
	nrm_scope_t *scope;
	nrm_actuator_t *actuator;

	nrm_log_info("creating dummy sensor\n");
	sensor = nrm_sensor_create("nrm-dummy-extra-sensor");
	err = nrm_client_add_sensor(client, sensor);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	nrm_log_info("creating dummy scope\n");
	scope = nrm_scope_create("nrm.scope.package.0");
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
	while (1) {
		nrm_time_t time;
		nrm_time_gettime(&time);
		nrm_client_send_event(client, time, sensor, scope, counter++);

		/* sleep */
		struct timespec req, rem;
		req = nrm_time_fromfreq(args.freq);
		/* possible signal interrupt here */
		err = nanosleep(&req, &rem);
		if (err == -1 && errno == EINTR) {
			nrm_log_error("interupted during sleep, exiting\n");
			break;
		}
	}
	nrm_client_destroy(&client);
	nrm_finalize();
	return 0;
}
