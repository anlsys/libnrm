/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

/* Filename: perfwrapper.c
 *
 * Description: Implements middleware between papi and the NRM
 *              downstream interface.
 */

#include "config.h"

#include "nrm.h"
#include "nrm/tools.h"
#include <errno.h>
#include <math.h>
#include <papi.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int cmdpid;
static nrm_client_t *client;
static nrm_scope_t *scope;
static nrm_vector_t *sensors = NULL;
static long long *counters = NULL;
static size_t EventCodeCnt = 0;
static int EventSet = PAPI_NULL;

int find_allowed_scope(nrm_client_t *client, nrm_scope_t **scope)
{
	/* create a scope based on hwloc_allowed, althouth we know for a fact
	 * that the daemon should already have this scope.
	 */
	nrm_string_t name =
	        nrm_string_fromprintf("nrm.extra.papi.%u", getpid());
	nrm_scope_t *allowed = nrm_scope_create_hwloc_allowed(name);
	nrm_string_decref(name);

	nrm_vector_t *nrmd_scopes;
	nrm_client_list_scopes(client, &nrmd_scopes);

	size_t numscopes;
	int newscope = 0;
	nrm_vector_length(nrmd_scopes, &numscopes);
	for (size_t i = 0; i < numscopes; i++) {
		nrm_scope_t *s;
		nrm_vector_pop_back(nrmd_scopes, &s);
		if (!nrm_scope_cmp(s, allowed)) {
			nrm_scope_destroy(allowed);
			allowed = s;
			newscope = 1;
			continue;
		}
		nrm_scope_destroy(s);
	}
	if (!newscope) {
		nrm_log_error("Could not find an existing scope to match\n");
		return -NRM_EINVAL;
	}
	nrm_vector_destroy(&nrmd_scopes);
	*scope = allowed;
	return 0;
}

int nrm_papiwrapper_timer_callback(nrm_reactor_t *reactor)
{
	(void)reactor;
	int err;
	nrm_time_t time;

	/* sample and report */
	err = PAPI_read(EventSet, counters);
	if (err != PAPI_OK) {
		nrm_log_error("PAPI event read error: %s\n",
				PAPI_strerror(err));
		return -1;
	}
	nrm_log_debug("PAPI counters read.\n");

	nrm_time_gettime(&time);
	nrm_log_debug("NRM time obtained.\n");

	for (size_t i = 0; i < EventCodeCnt; i++) {
		nrm_sensor_t **sensor;
		nrm_vector_get_withtype(nrm_sensor_t *, sensors, i,
				sensor);
		if (nrm_client_send_event(client, time, *sensor, scope,
					counters[i]) != 0) {
			nrm_log_error(
					"Sending event to the daemon error\n");
			return -1;
		}
	}
	nrm_log_debug("NRM values sent.\n");
	return 0;
}

int nrm_papiwrapper_signal_callback(nrm_reactor_t *reactor,
		struct signalfd_siginfo fdsi)
{
	(void)reactor;
	nrm_log_debug("Received signal\n");

	/* exit immediately on SIGINT/SIGTERM */
	if (fdsi.ssi_signo != SIGCHLD)
		return -1;

	nrm_log_debug("Signal info: %d %d\n", fdsi.ssi_signo, fdsi.ssi_pid);
	/* ignore status updates on wrong pid */
	if ((int)fdsi.ssi_pid != cmdpid)
		return 0;

	/* ignore stopped/restarted cmd */
	if (fdsi.ssi_code == CLD_STOPPED || fdsi.ssi_code == CLD_CONTINUED)
		return 0;

	/* we're here if the child command exited, so end the program by
	 * exiting the reactor
	 */
	return -1;
}

int main(int argc, char **argv)
{
	int err;
	nrm_tools_args_t args;

	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "nrm-papiwrapper");

	int ret = EXIT_FAILURE;

	args.progname = "nrm-papiwrapper";
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

	if (err >= argc) {
		nrm_log_error("Expected command after options.\n");
		exit(EXIT_FAILURE);
	}

	/* remove the parsed part */
	argc -= err;
	argv = &(argv[err]);

	nrm_log_setlevel(args.log_level);

	nrm_vector_length(args.events, &EventCodeCnt);
	if (EventCodeCnt == 0) {
		EventCodeCnt = 1;
		nrm_string_t def = nrm_string_fromchar("PAPI_TOT_INS");
		nrm_vector_push_back(args.events, &def);
	}

	// create client
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

	if (find_allowed_scope(client, &scope) != 0) {
		nrm_log_error("Finding scope failed\n");
		goto cleanup_client;
	}
	nrm_log_debug("NRM scope initialized.\n");

	/* create our sensors and add them to the daemon */
	nrm_vector_create(&sensors, sizeof(nrm_sensor_t *));
	if (sensors == NULL) {
		nrm_log_error("Allocating sensors failed\n");
		goto cleanup_scope;
	}
	for (size_t i = 0; i < EventCodeCnt; i++) {
		nrm_string_t *eventName;
		nrm_string_t sensor_name;

		nrm_vector_get_withtype(nrm_string_t, args.events, i,
		                        eventName);
		sensor_name = nrm_string_fromprintf("nrm.extra.perf.%s.%u",
		                                    *eventName, getpid());
		nrm_sensor_t *sensor = nrm_sensor_create(sensor_name);
		nrm_string_decref(sensor_name);
		if (sensor == NULL) {
			nrm_log_error("Sensor creation failed\n");
			goto cleanup_sensor;
		}
		if (nrm_client_add_sensor(client, sensor) != 0) {
			nrm_log_error("Adding sensor failed\n");
			goto cleanup_sensor;
		}
		nrm_vector_push_back(sensors, &sensor);
	}

	// initialize PAPI
	int papi_retval;
	papi_retval = PAPI_library_init(PAPI_VER_CURRENT);

	if (papi_retval != PAPI_VER_CURRENT) {
		nrm_log_error("PAPI library init error: %s\n",
		              PAPI_strerror(papi_retval));
		goto cleanup;
	}

	const PAPI_component_info_t *cmpinfo;
	if ((cmpinfo = PAPI_get_component_info(0)) == NULL) {
		nrm_log_error("PAPI_get_component_info failed\n");
		goto cleanup;
	}
	nrm_log_debug("PAPI initialized.\n");

	/* set up PAPI interface */

	err = PAPI_create_eventset(&EventSet);
	if (err != PAPI_OK) {
		nrm_log_error("PAPI eventset creation error: %s\n",
		              PAPI_strerror(err));
		goto cleanup;
	}
	for (size_t i = 0; i < EventCodeCnt; i++) {
		int EventCode;
		nrm_string_t *EventName;
		nrm_vector_get_withtype(nrm_string_t, args.events, i,
		                        EventName);
		err = PAPI_event_name_to_code(*EventName, &EventCode);
		if (err != PAPI_OK) {
			nrm_log_error("PAPI event_name translation error: %s\n",
			              PAPI_strerror(err));
			goto cleanup;
		}
		err = PAPI_add_event(EventSet, EventCode);
		if (err != PAPI_OK) {
			nrm_log_error("PAPI eventset append error: %s\n",
			              PAPI_strerror(err));
			goto cleanup;
		}

		nrm_log_debug(
		        "PAPI code string %s converted to PAPI code %i, and registered.\n",
		        *EventName, EventCode);
	}

	cmdpid = fork();
	if (cmdpid < 0) {
		nrm_log_error("fork error\n");
		goto cleanup;
	} else if (cmdpid == 0) {
		/* child, needs to exec the cmd */
		if (cmpinfo->attach_must_ptrace)
			if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
				nrm_log_error(
				        "ptrace(PTRACE_TRACEME) failed\n");
				_exit(EXIT_FAILURE);
			}

		err = execvp(argv[0], &argv[0]);
		nrm_log_error("Error executing command: %s\n", strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* parent process */
	if (cmpinfo->attach_must_ptrace) {
		int status;

		/* Wait for the child process to stop on execve(2).  */
		if (waitpid(cmdpid, &status, 0) == -1) {
			nrm_log_error("waitpid error: %s\n", strerror(errno));
			goto cleanup;
		}

		if (WIFSTOPPED(status) == 0) {
			nrm_log_error("Unexpected waitpid status: %d\n",
			              status);
			goto cleanup;
		}
	}

	/* Need to attach counters to the child */
	err = PAPI_attach(EventSet, cmdpid);
	if (err != PAPI_OK) {
		nrm_log_error("PAPI eventset attach error: %s\n",
		              PAPI_strerror(err));
		goto cleanup;
	}
	nrm_log_debug("PAPI attached to process with pid %i\n", cmdpid);

	err = PAPI_start(EventSet);
	if (err != PAPI_OK) {
		nrm_log_error("PAPI start error: %s\n", PAPI_strerror(err));
		goto cleanup;
	}
	nrm_log_debug("PAPI started. Initializing event read/send to NRM\n");

	nrm_time_t time;
	if ((counters = malloc(EventCodeCnt * sizeof(*counters))) == NULL) {
		nrm_log_error("Allocating counters failed\n");
		goto cleanup;
	}

	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGINT);
	sigaddset(&sigmask, SIGTERM);
	sigaddset(&sigmask, SIGCHLD);
	nrm_reactor_t *reactor;
	err = nrm_reactor_create(&reactor, &sigmask);
	if (err) {
		nrm_log_error("error during reactor creation\n");
		goto cleanup;
	}

	nrm_reactor_user_callbacks_t callbacks = {
	        .signal = nrm_papiwrapper_signal_callback,
	        .timer = nrm_papiwrapper_timer_callback,
	};
	nrm_reactor_setcallbacks(reactor, callbacks);

	nrm_time_t sleeptime = nrm_time_fromfreq(args.freq);
	nrm_reactor_settimer(reactor, sleeptime);

	if (cmpinfo->attach_must_ptrace) {
		/* Let the child continue with execve(2).  */
		if (ptrace(PTRACE_CONT, cmdpid, NULL, NULL) == -1) {
			nrm_log_error("ptrace(PTRACE_TRACEME) failed\n");
			goto cleanup;
		}
	}

	nrm_reactor_start(reactor);
	ret = EXIT_SUCCESS;
	nrm_log_debug("Finalizing PAPI-event read/send to NRM.\n");

	/* final send here */
	PAPI_stop(EventSet, counters);
	nrm_time_gettime(&time);
	for (size_t i = 0; i < EventCodeCnt; i++) {
		nrm_sensor_t **sensor;
		nrm_vector_get_withtype(nrm_sensor_t *, sensors, i, sensor);
		nrm_client_send_event(client, time, *sensor, scope,
		                      counters[i]);
	}

cleanup:
	free(counters);
cleanup_sensor:
	for (size_t i = 0; i < EventCodeCnt; i++) {
		nrm_sensor_t **sensor;
		nrm_vector_get_withtype(nrm_sensor_t *, sensors, i, sensor);
		if (*sensor)
			nrm_sensor_destroy(sensor);
	}
	nrm_vector_destroy(&sensors);
cleanup_scope:
	nrm_scope_destroy(scope);
cleanup_client:
	nrm_client_destroy(&client);
cleanup_postinit:
	nrm_tools_args_destroy(&args);
	nrm_finalize();
	return ret;
}
