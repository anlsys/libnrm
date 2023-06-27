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
#include "nrm-tools.h"

#include "internal/messages.h"
#include "internal/nrmi.h"
#include "internal/roles.h"

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
	nrm_tools_common_args_t args;
	err = nrm_tools_parse_common_args(argc, argv, &args);
	if (err < 0) {
		fprintf(stderr,
			"nrm-dummy-extra: errors during argument parsing\n");
		exit(EXIT_FAILURE);
	}

	/* remove the parsed part, but keep argv[0] around:
	 *  - err is the position of the next unparsed argv
	 *  - we need to reduce argc by the amount of argument parsed
	 *  - argv is updated to match
	 */
	argc = (argc - err) + 1;
	for (int i = 0; i < argc - 1; i++)
		argv[i+1] = argv[err+i];

	nrm_tools_extra_args_t extra_args;
	err = nrm_tools_parse_extra_args(argc, argv, &extra_args, 
					 NRM_TOOLS_EXTRA_ARG_FREQ);
	if (err < 0) {
		fprintf(stderr,
			"nrm-dummy-extra: error during extra arg parsing\n");
		exit(EXIT_FAILURE);
	}

	if (args.ask_help) {
		nrm_tools_print_common_help("nrm-dummy-extra");
		exit(EXIT_SUCCESS);
	}
	if (args.ask_version) {
		nrm_tools_print_common_version("nrm-dummy-extra");
		exit(EXIT_SUCCESS);
	}

	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "nrm-dummy-extra");
	nrm_log_setlevel(args.log_level);

	nrm_log_debug("frequency setting: %f\n", extra_args.freq);
	nrm_log_debug("after command line parsing: argc: %u argv[0]: %s\n",
	              argc, argv[0]);

	nrm_log_info("creating client\n");
	nrm_client_create(&client, args.upstream_uri, args.pub_port, args.rpc_port);

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
	while (true) {
		nrm_time_t time;
		nrm_time_gettime(&time);
		nrm_client_send_event(client, time, sensor, scope, counter++);

		/* sleep */
		double sleeptime = 1 / extra_args.freq;
		struct timespec req, rem;
		req.tv_sec = ceil(sleeptime);
		req.tv_nsec = sleeptime * 1e9 - ceil(sleeptime) * 1e9;
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
