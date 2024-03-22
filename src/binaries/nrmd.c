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
#include <sys/signalfd.h>

#include "internal/nrmi.h"

#include "internal/control.h"

struct nrm_daemon_s {
	nrm_state_t *state;
	nrm_server_t *server;
	nrm_eventbase_t *events;
	nrm_control_t *control;
	nrm_sensor_t *mysensor;
	nrm_scope_t *myscope;
	nrm_string_t mytopic;
	nrm_string_t eventtopic;
};

int signo;
struct nrm_daemon_s my_daemon;

int nrmd_event_callback(nrm_server_t *server,
                        nrm_string_t uuid,
                        nrm_scope_t *scope,
                        nrm_time_t time,
                        double value)
{
	nrm_eventbase_push_event(my_daemon.events, uuid, scope, time, value);
	nrm_server_publish(server, my_daemon.eventtopic, time, uuid, scope,
	                   value);
	return 0;
}

int nrmd_actuate_callback(nrm_server_t *server, nrm_actuator_t *a, double value)
{
	(void)server;
	(void)a;
	(void)value;
	/* we only allow actuation to happen here */
	return 0;
}

double nrmd_actuator_value(nrm_string_t uuid)
{
	nrm_actuator_t *a = NULL;
	nrm_hash_find(my_daemon.state->actuators, uuid, (void *)&a);
	if (a != NULL) {
		/* found the actuator */
		return a->value;
	}
	return 0.0;
}

int nrmd_control_tick(nrm_server_t *server)
{
	nrm_time_t now;
	nrm_time_gettime(&now);

	/* control loop: build vector of inputs */
	nrm_log_debug("control loop tick\n");
	nrm_vector_t *inputs;
	nrm_vector_t *outputs;
	nrm_control_getargs(my_daemon.control, &inputs, &outputs);

	nrm_vector_foreach(inputs, iterator)
	{
		nrm_control_input_t *in = nrm_vector_iterator_get(iterator);
		/* pull the scope from the server */
		nrm_scope_t *scope;
		nrm_hash_find(my_daemon.state->scopes, in->scope_uuid,
		              (void *)&scope);
		if (scope == NULL) {
			nrm_log_error("input scope not found");
			continue;
		}
		nrm_eventbase_pull_timeserie(my_daemon.events, in->sensor_uuid,
		                             scope, in->since, &in->timeserie);
	}

	nrm_vector_foreach(outputs, iterator)
	{
		nrm_control_output_t *out = nrm_vector_iterator_get(iterator);
		out->value = nrmd_actuator_value(out->actuator_uuid);
	}
	/* launch control: fill inputs and outputs first */
	nrm_log_debug("control action\n");
	nrm_control_action(my_daemon.control, inputs, outputs);

	/* update actuators */
	nrm_vector_foreach(outputs, iterator)
	{
		nrm_control_output_t *out = nrm_vector_iterator_get(iterator);
		nrm_server_actuate(server, out->actuator_uuid, out->value);
	}
	/* tick the event base */
	nrm_log_debug("eventbase tick\n");
	nrm_eventbase_tick(my_daemon.events, now);
	return 0;
}

int nrmd_timer_callback(nrm_server_t *server)
{
	nrm_log_info("global timer wakeup\n");

	/* create a ticking event */
	nrm_time_t now;
	nrm_time_gettime(&now);

	nrm_server_publish(server, my_daemon.mytopic, now,
	                   my_daemon.mysensor->uuid, my_daemon.myscope, 1.0);

	if (my_daemon.control == NULL)
		return 0;

	nrmd_control_tick(server);
	return 0;
}

int nrmd_tick_callback(nrm_server_t *server)
{
	nrm_log_info("tick wakeup\n");

	if (my_daemon.control == NULL)
		return 0;

	nrmd_control_tick(server);
	return 0;
}

int main(int argc, char *argv[])
{
	int err;
	nrm_tools_args_t args;
	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "nrmd");

	args.progname = "nrmd";
	args.flags = NRM_TOOLS_ARGS_FLAG_FREQ;
	err = nrm_tools_parse_args(argc, argv, &args);
	if (err < 0) {
		nrm_log_error("Errors during argument parsing\n");
		nrm_tools_print_help(&args);
		exit(EXIT_FAILURE);
	}

	/* remove the parsed part */
	argc -= err;
	argv = &(argv[err]);

	if (args.ask_help) {
		nrm_tools_print_help(&args);
		exit(EXIT_SUCCESS);
	}
	if (args.ask_version) {
		nrm_tools_print_version(&args);
		exit(EXIT_SUCCESS);
	}

	nrm_log_setlevel(args.log_level);

	/* init state */
	my_daemon.state = nrm_state_create();
	my_daemon.events = nrm_eventbase_create(5);
	nrm_scope_hwloc_scopes(&my_daemon.state->scopes);
	my_daemon.mysensor = nrm_sensor_create("daemon.tick");
	nrm_string_t global_scope = nrm_string_fromchar("nrm.hwloc.Machine.0");
	nrm_hash_find(my_daemon.state->scopes, global_scope,
	              (void *)&my_daemon.myscope);
	assert(my_daemon.myscope);
	nrm_string_decref(global_scope);
	my_daemon.mytopic = nrm_string_fromchar("daemon");
	my_daemon.eventtopic = nrm_string_fromchar("daemon.events.raw");

	/* configuration */
	if (argc == 0) {
		nrm_log_info("no configuration given, skipping control\n");
		goto start;
	}

	json_error_t jerror;
	FILE *config = fopen(argv[0], "r");
	assert(config != NULL);
	json_t *jconfig = json_loadf(config, 0, &jerror);
	assert(jconfig != NULL);
	json_t *control_config;
	err = json_unpack_ex(jconfig, &jerror, 0, "{s?:o}", "control",
	                     &control_config);
	if (!err && control_config) {
		nrm_control_create(&my_daemon.control, control_config);
	}

start:
	nrm_log_info("daemon initialized\n");

	/* start the server */
	err = nrm_server_create(&my_daemon.server, my_daemon.state,
	                        args.upstream_uri, args.pub_port,
	                        args.rpc_port);
	assert(err == 0);

	/* setting up the callbacks */
	nrm_server_user_callbacks_t callbacks = {
	        .event = nrmd_event_callback,
	        .actuate = nrmd_actuate_callback,
	        .signal = NULL,
	        .timer = nrmd_timer_callback,
	        .tick = nrmd_tick_callback,
	};
	nrm_server_setcallbacks(my_daemon.server, callbacks);

	/* add a periodic wake up to generate metrics */
	if (args.freq != 0.0) {
		nrm_time_t sleeptime = nrm_time_fromfreq(args.freq);
		nrm_server_settimer(my_daemon.server, sleeptime);
	}

	/* start the whole thing */
	nrm_server_start(my_daemon.server);

	nrm_log_info("exiting daemon\n");

	/* teardown NRM */
	nrm_string_decref(my_daemon.mytopic);
	nrm_string_decref(my_daemon.eventtopic);
	nrm_sensor_destroy(&my_daemon.mysensor);
	nrm_eventbase_destroy(&my_daemon.events);
	nrm_state_destroy(&my_daemon.state);
	nrm_server_destroy(&my_daemon.server);
	nrm_log_debug("NRM components destroyed\n");

	exit(EXIT_SUCCESS);
}
