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
#include <getopt.h>
#include <sys/signalfd.h>

#include "internal/nrmi.h"

#include "internal/control.h"
#include "internal/messages.h"
#include "internal/roles.h"

struct nrm_daemon_s {
	nrm_state_t *state;
	nrm_server_t *server;
	nrm_eventbase_t *events;
	nrm_control_t *control;
	nrm_sensor_t *mysensor;
	nrm_scope_t *myscope;
	nrm_string_t mytopic;
};

int signo;
struct nrm_daemon_s my_daemon;

int nrmd_event_callback(nrm_server_t *server, nrm_string_t uuid,
			nrm_scope_t *scope, nrm_time_t time, double value)
{
	(void)server;
	/* TODO also publish the raw events */
	nrm_eventbase_push_event(my_daemon.events, uuid, scope, time, value);
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

int nrmd_timer_callback(nrm_server_t *server)
{
	nrm_log_info("global timer wakeup\n");

	/* create a ticking event */
	nrm_time_t now;
	nrm_time_gettime(&now);

	nrm_server_publish(server, my_daemon.mytopic, now, my_daemon.mysensor,
			   my_daemon.myscope, 1.0);

	/* tick the event base */
	nrm_log_debug("eventbase tick\n");
	nrm_eventbase_tick(my_daemon.events, now);

	/* control loop: build vector of inputs */
	nrm_log_debug("control loop tick\n");
	if (my_daemon.control == NULL)
		return 0;

	nrm_vector_t *inputs;
	nrm_vector_t *outputs;
	nrm_control_getargs(my_daemon.control, &inputs, &outputs);

	nrm_vector_foreach(inputs, iterator)
	{
		nrm_control_input_t *in = nrm_vector_iterator_get(iterator);
		nrm_eventbase_last_value(my_daemon.events, in->sensor_uuid,
		                         in->scope_uuid, &in->value);
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
	return 0;
}

static int ask_help = 0;
static int ask_version = 0;

static struct option long_options[] = {
        {"help", no_argument, &ask_help, 1},
        {"version", no_argument, &ask_version, 1},
        {0, 0, 0, 0},
};

static const char *short_options = ":hV";

static const char *help[] = {"Usage: nrmd-shim [options]\n\n",
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
	fprintf(stdout, "nrmd: version %s\n", nrm_version_string);
}

int main(int argc, char *argv[])
{
	int c, sfd, retval;
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
		case 'V':
			ask_version = 1;
			break;
		case '?':
			fprintf(stderr, "nrmd-shim: invalid options: %s\n",
			        argv[optind - 1]);
			exit(EXIT_FAILURE);
		default:
			fprintf(stderr, "nrmd_shim: this should not happen\n");
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
	nrm_log_init(stderr, "nrmd");
	nrm_log_setlevel(NRM_LOG_DEBUG);

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

	/* configuration */
	if (argc == 0) {
		nrm_log_info("no configuration given, skipping control\n");
		goto start;
	}

	json_error_t jerror;
	int err;
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
	        NRM_DEFAULT_UPSTREAM_URI, NRM_DEFAULT_UPSTREAM_PUB_PORT,
	        NRM_DEFAULT_UPSTREAM_RPC_PORT);
	assert(err == 0);

	/* setting up the callbacks */
	nrm_server_user_callbacks_t callbacks = {
		.event = nrmd_event_callback,
		.actuate = nrmd_actuate_callback,
		.signal = NULL,
		.timer = nrmd_timer_callback,
	};
	nrm_server_setcallbacks(my_daemon.server, callbacks);

	/* add a periodic wake up to generate metrics */
	nrm_server_settimer(my_daemon.server, 1000);

	/* start the whole thing */
	nrm_server_start(my_daemon.server);

	/* teardown NRM */
	nrm_string_decref(my_daemon.mytopic);
	nrm_sensor_destroy(&my_daemon.mysensor);
	nrm_eventbase_destroy(&my_daemon.events);
	nrm_state_destroy(&my_daemon.state);
	nrm_server_destroy(&my_daemon.server);
	nrm_log_debug("NRM components destroyed\n");

	exit(EXIT_SUCCESS);
}
