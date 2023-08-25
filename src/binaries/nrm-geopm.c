/*******************************************************************************
 * Copyright 2023 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

/* Filename: nrm-geopm.c
 *
 * Description: Implements middleware between GEOPM (https://geopm.github.io/)
 * and the NRM downstream interface. Resources detected via GEOPM. The
 * `geopmd` service must be running. The `msr` module must be enabled via
 * `modprobe msr` if its installed. This utility may also need root privileges
 * to detect many basic signals like CPU_POWER.
 */

#include "nrm.h"
#include "nrm/tools.h"
#include <assert.h>
#include <errno.h>
#include <geopm_pio.h>
#include <geopm_topo.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static nrm_client_t *client;
static nrm_vector_t *events;
static size_t num_events;

struct nrm_geopm_eventinfo_s {
	nrm_string_t name;
	nrm_string_t control;
	int domain_type;
	int num_domains;
	nrm_sensor_t *sensor;
	nrm_actuator_t *actuator;
	nrm_vector_t *scopes;
};

typedef struct nrm_geopm_eventinfo_s nrm_geopm_eventinfo_t;

int nrm_geopm_cpu_act_callback(nrm_uuid_t *uuid, double value)
{

	nrm_vector_foreach(events, iterator)
	{
		nrm_geopm_eventinfo_t *event =
		        nrm_vector_iterator_get(iterator);
		if (event->domain_type == GEOPM_DOMAIN_PACKAGE) {
			for (int i = 0; i < event->num_domains; i++) {
				int err = geopm_pio_write_control(
				        event->control, event->domain_type, i,
				        value);
				if (err) {
					nrm_log_error(
					        "Unable to write_control to package within callback\n");
					return EXIT_FAILURE;
				}
			}
		}
	}
}

int nrm_geopm_set_actuator_info(nrm_geopm_eventinfo_t event)
{
	if (event.domain_type == GEOPM_DOMAIN_PACKAGE) {
		event.control = nrm_string_fromchar("CPU_POWER_LIMIT_CONTROL");
		double choices[2] = {236.0, 240.0};
		nrm_actuator_set_choices(event.actuator, 2, choices);
		nrm_actuator_set_value(event.actuator, 236.0);
		err = nrm_client_add_actuator(client, event.actuator);
		if (err) {
			nrm_log_error(
			        "Unable to add PACKAGE actuator to client\n");
			return EXIT_FAILURE;
		}
		nrm_client_set_actuate_listener(client,
		                                nrm_geopm_cpu_act_callback);
		nrm_client_start_actuate_listener(client);
	}
	// can do a GPU condition/setup later here
}

int nrm_geopm_domain_to_scope(nrm_scope_t *scope,
                              int domain_type,
                              int domain_idx)
{
	if (domain_type == GEOPM_DOMAIN_PACKAGE) {
		/* GEOPM uses it's own internal numbering, so ask it to convert
		 * to OS indexes
		 */
		int num_pus = geopm_topo_num_domain_nested(
		        GEOPM_DOMAIN_CPU, GEOPM_DOMAIN_PACKAGE);
		int pus[num_pus];
		geopm_topo_domain_nested(GEOPM_DOMAIN_CPU, GEOPM_DOMAIN_PACKAGE,
		                         domain_idx, num_pus, pus);

		for (int i = 0; i < num_pus; i++) {
			nrm_scope_add(scope, NRM_SCOPE_TYPE_CPU, pus[i]);
		}
	} else if (domain_type == GEOPM_DOMAIN_GPU) {
		nrm_scope_add(scope, NRM_SCOPE_TYPE_GPU, domain_idx);
	}
	return 0;
}

int nrm_geopm_find_scope(nrm_scope_t **scope, int *added)
{
	nrm_vector_t *nrmd_scopes;
	int newscope = 0;
	size_t numscopes;
	nrm_client_list_scopes(client, &nrmd_scopes);
	nrm_vector_length(nrmd_scopes, &numscopes);
	for (size_t i = 0; i < numscopes; i++) {
		nrm_scope_t *s;
		nrm_vector_pop_back(nrmd_scopes, &s);
		if (!nrm_scope_cmp(s, *scope)) {
			nrm_scope_destroy(*scope);
			*scope = s;
			newscope = 1;
			continue;
		}
		nrm_scope_destroy(s);
	}
	if (!newscope) {
		nrm_log_debug(
		        "allowed scope not found in nrmd, adding a new one\n");
	}
	nrm_vector_destroy(&nrmd_scopes);
	*added = !newscope;
	return 0;
}

void nrm_geopm_prepare_event(nrm_string_t *s)
{
	int err;
	nrm_geopm_eventinfo_t event;
	event.name = *s;
	nrm_string_incref(event.name);

	/* geopm info */
	event.domain_type = geopm_pio_signal_domain_type(event.name);
	nrm_log_debug("Parsed %s as domain type %d\n", event.name,
	              event.domain_type);
	assert(event.domain_type > 0);
	event.num_domains = geopm_topo_num_domain(event.domain_type);
	nrm_log_debug("Domain type %d as %d domains\n", event.domain_type,
	              event.num_domains);
	assert(event.num_domains > 0);

	char domain_name[NAME_MAX + 1];
	err = geopm_topo_domain_name(event.domain_type, NAME_MAX, domain_name);
	assert(err == 0);
	nrm_log_debug("Parsed %s as domain %s\n", event.name, domain_name);

	/* sensor info */
	nrm_string_t sensor_name =
	        nrm_string_fromprintf("nrm.geopm.%s", event.name);
	event.sensor = nrm_sensor_create(sensor_name);
	nrm_client_add_sensor(client, event.sensor);

	/* actuator info */
	nrm_string_t act_name =
	        nrm_string_fromprintf("nrm.geopm.%s", domain_name);
	event.actuator = nrm_actuator_create(act_name);
	err = nrm_geopm_set_actuator_info(event);
	assert(err == 0);

	/* scope info */
	nrm_vector_create(&event.scopes, sizeof(nrm_scope_t *));
	for (int i = 0; i < event.num_domains; i++) {
		nrm_string_t scope_name = nrm_string_fromprintf(
		        "nrm.geopm.%s.%s.%d", event.name, domain_name, i);
		int added_scope;
		nrm_scope_t *scope = nrm_scope_create(scope_name);
		nrm_geopm_domain_to_scope(scope, event.domain_type, i);
		nrm_geopm_find_scope(&scope, &added_scope);
		nrm_vector_push_back(event.scopes, &scope);
	}
	nrm_vector_push_back(events, &event);
	nrm_log_debug("Created sensor %s from event %s\n", event.sensor->uuid,
	              event.name);
}

int nrm_geopm_timer_callback(nrm_reactor_t *reactor)
{
	(void)reactor;
	nrm_time_t time;
	nrm_time_gettime(&time);
	nrm_vector_foreach(events, iterator)
	{
		nrm_geopm_eventinfo_t *event =
		        nrm_vector_iterator_get(iterator);
		for (int i = 0; i < event->num_domains; i++) {
			double value = 0.0;
			nrm_scope_t **s;
			nrm_vector_get_withtype(nrm_scope_t *, event->scopes, i,
			                        s);
			geopm_pio_read_signal(event->name, event->domain_type,
			                      i, &value);
			nrm_log_debug("%s.%d - energy measurement: %f\n",
			              event->name, i, value);
			nrm_client_send_event(client, time, event->sensor, *s,
			                      value);
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	int err;
	nrm_tools_args_t args;

	nrm_init(NULL, NULL);
	assert(nrm_log_init(stderr, "nrm-geopm") == 0);

	int ret = EXIT_FAILURE;

	args.progname = "nrm-geopm";
	args.flags = NRM_TOOLS_ARGS_FLAG_FREQ | NRM_TOOLS_ARGS_FLAG_EVENT;

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

	nrm_vector_length(args.events, &num_events);
	if (num_events == 0) {
		nrm_string_t s = nrm_string_fromchar("CPU_ENERGY");
		nrm_vector_push_back(args.events, &s);
		nrm_log_debug("Measuring CPU_ENERGY by default\n");
	}

	if (nrm_client_create(&client, args.upstream_uri, args.pub_port,
	                      args.rpc_port) != 0 ||
	    client == NULL) {
		nrm_log_error("Client creation failed\n");
		goto cleanup_postinit;
	}

	nrm_log_debug("NRM client initialized.\n");
	nrm_log_debug("Events:\n");
	nrm_vector_foreach(args.events, iterator)
	{
		nrm_string_t *s = nrm_vector_iterator_get(iterator);
		nrm_log_debug("%s\n", *s);
	}

	int nsignals = geopm_pio_num_signal_name();
	assert(nsignals > 0); // just check that we can obtain signals
	nrm_log_debug("GEOPM detects %d possible signals\n", nsignals);

	nrm_vector_create(&events, sizeof(nrm_geopm_eventinfo_t));
	nrm_vector_foreach(args.events, iterator)
	{
		nrm_string_t *s = nrm_vector_iterator_get(iterator);
		nrm_geopm_prepare_event(s);
	}

	nrm_reactor_t *reactor;
	err = nrm_reactor_create(&reactor, NULL);
	if (err) {
		nrm_log_error("error during reactor creation\n");
		goto cleanup;
	}

	nrm_reactor_user_callbacks_t callbacks = {
	        .signal = NULL,
	        .timer = nrm_geopm_timer_callback,
	};
	nrm_reactor_setcallbacks(reactor, callbacks);
	nrm_time_t sleeptime = nrm_time_fromfreq(args.freq);
	nrm_reactor_settimer(reactor, sleeptime);
	nrm_reactor_start(reactor);
	ret = EXIT_SUCCESS;

	nrm_vector_foreach(events, iterator)
	{
		nrm_geopm_eventinfo_t *e = nrm_vector_iterator_get(iterator);
		nrm_client_remove_sensor(client, e->sensor);
		nrm_sensor_destroy(&e->sensor);
		for (int i = 0; i < e->num_domains; i++) {
			nrm_scope_t **s;
			nrm_vector_get_withtype(nrm_scope_t *, e->scopes, i, s);
			nrm_scope_destroy(*s);
		}
	}
	nrm_vector_destroy(&events);
cleanup:
	nrm_reactor_destroy(&reactor);

	nrm_client_destroy(&client);
cleanup_postinit:
	nrm_tools_args_destroy(&args);
	nrm_finalize();
	return ret;
}
