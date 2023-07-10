/*******************************************************************************
 * Copyright 2021 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the nrm-extra project.
 * For more info, see https://github.com/anlsys/nrm-extra
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

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

int main(int argc, char **argv)
{
	int err;
	nrm_tools_args_t args;

	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "nrm-papiwrapper");

	int ret = EXIT_FAILURE;
	nrm_client_t *client;
	nrm_scope_t *scope;
	nrm_vector_t *sensors = NULL;
	long long *counters = NULL;
	size_t EventCodeCnt = 0;

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
	int EventSet = PAPI_NULL;

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
		        EventName, EventCode);
	}

	int pid = fork();
	if (pid < 0) {
		nrm_log_error("fork error\n");
		goto cleanup;
	} else if (pid == 0) {
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
		if (waitpid(pid, &status, 0) == -1) {
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
	err = PAPI_attach(EventSet, pid);
	if (err != PAPI_OK) {
		nrm_log_error("PAPI eventset attach error: %s\n",
		              PAPI_strerror(err));
		goto cleanup;
	}
	nrm_log_debug("PAPI attached to process with pid %i\n", pid);

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

	if (cmpinfo->attach_must_ptrace) {
		/* Let the child continue with execve(2).  */
		if (ptrace(PTRACE_CONT, pid, NULL, NULL) == -1) {
			nrm_log_error("ptrace(PTRACE_TRACEME) failed\n");
			goto cleanup;
		}
	}

	do {
		/* sleep for a frequency */
		long long sleeptime = 1e9 / args.freq;
		struct timespec req, rem;
		req.tv_sec = sleeptime / 1000000000;
		req.tv_nsec = sleeptime % 1000000000;
		/* deal with signal interrupts */
		do {
			err = nanosleep(&req, &rem);
			req = rem;
		} while (err == -1 && errno == EINTR);

		/* sample and report */
		err = PAPI_read(EventSet, counters);
		if (err != PAPI_OK) {
			nrm_log_error("PAPI event read error: %s\n",
			              PAPI_strerror(err));
			goto cleanup;
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
				goto cleanup;
			}
		}
		nrm_log_debug("NRM values sent.\n");

		/* loop until child exits */
		int status;
		err = waitpid(pid, &status, WNOHANG);
		if (err == -1) {
			nrm_log_error("waitpid error: %s\n", strerror(errno));
			goto cleanup;
		}
	} while (err != pid);
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
