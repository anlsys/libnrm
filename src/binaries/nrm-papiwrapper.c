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

#include <errno.h>
#include <getopt.h>
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


#define UPSTREAM_URI "tcp://127.0.0.1"
#define PUB_PORT 2345
#define RPC_PORT 3456

int find_allowed_scope(nrm_client_t *client, nrm_scope_t **scope)

int main(int argc, char **argv)
{
	int c, err;
	double freq = 1;
	char(*EventCodeStrs)[PAPI_MAX_STR_LEN] = NULL;
	int EventCodeCnt = 0;
	int ret = EXIT_FAILURE;

	nrm_client_t *client;
	nrm_scope_t *scope;
	nrm_sensor_t **sensors = NULL;
	int custom_scope = 0;
	long long *counters = NULL;

	int log_level = NRM_LOG_ERROR;

	const char *usage =
	        "Usage: nrm-perfwrapper [options] [command]\n"
	        "     options:\n"
	        "            -e, --event             PAPI preset event name. Default: PAPI_TOT_INS\n"
	        "            -f, --frequency         Frequency in hz to poll. Default: 10.0\n"
	        "            -v, --verbose           Produce verbose output. Log messages will be displayed to stderr\n"
	        "            -h, --help              Displays this help message\n";

	while (1) {
		static struct option long_options[] = {
		        {"verbose", no_argument, 0, 'v'},
		        {"frequency", required_argument, 0, 'f'},
		        {"help", no_argument, 0, 'h'},
		        {"event", required_argument, 0, 'e'},
		        {0, 0, 0, 0}};

		int option_index = 0;
		c = getopt_long(argc, argv, "+vf:e:h", long_options,
		                &option_index);

		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'v':
			log_level = NRM_LOG_DEBUG;
			break;
		case 'f':
			errno = 0;
			freq = strtod(optarg, NULL);
			if (errno != 0 || freq <= 0) {
				fprintf(stderr,
				        "Error parsing the frequency\n");
				goto cleanup_preinit;
			}
			break;
		case 'e':
			if ((EventCodeStrs = realloc(
			             EventCodeStrs,
			             (EventCodeCnt + 1) *
			                     sizeof(*EventCodeStrs))) == NULL) {
				fprintf(stderr, "realloc failed\n");
				goto cleanup_preinit;
			}
			strcpy(EventCodeStrs[EventCodeCnt++], optarg);
			break;
		case 'h':
			fprintf(stderr, "%s", usage);
			goto cleanup_preinit;
		case '?':
		default:
			fprintf(stderr, "Wrong option argument\n");
			fprintf(stderr, "%s", usage);
			goto cleanup_preinit;
		}
	}

	if (EventCodeCnt == 0) {
		EventCodeCnt = 1;
		if ((EventCodeStrs = malloc(sizeof(*EventCodeStrs))) == NULL) {
			fprintf(stderr, "malloc failed\n");
			goto cleanup_preinit;
		}
		strcpy(EventCodeStrs[0], "PAPI_TOT_INS");
	}

	if (nrm_init(NULL, NULL) != 0) {
		fprintf(stderr, "nrm_init failed\n");
		goto cleanup_preinit;
	}
	if (nrm_log_init(stderr, "nrm.extra.perf") != 0) {
		fprintf(stderr, "nrm_log_init failed\n");
		goto cleanup_postinit;
	}

	nrm_log_setlevel(log_level);
	nrm_log_debug("NRM logging initialized.\n");

	// create client
	if (nrm_client_create(&client, UPSTREAM_URI, PUB_PORT, RPC_PORT) != 0 ||
	    client == NULL) {
		nrm_log_error("Client creation failed\n");
		goto cleanup_postinit;
	}

	nrm_log_debug("NRM client initialized.\n");

	nrm_log_debug("verbose=%d; freq=%f\n", log_level, freq);
	nrm_log_debug("Events:\n");
	for (int i = 0; i < EventCodeCnt; i++)
		nrm_log_debug("[%d]=%s\n", i, EventCodeStrs[i]);

	if (optind >= argc) {
		nrm_log_error("Expected command after options.\n");
		goto cleanup_client;
	}

	if (nrm_extra_find_allowed_scope(client, "nrm.extra.perf", &scope,
	                                 &custom_scope) != 0) {
		nrm_log_error("Finding scope failed\n");
		goto cleanup_client;
	}
	nrm_log_debug("NRM scope initialized.\n");

	/* create our sensors and add them to the daemon */
	if ((sensors = calloc(EventCodeCnt, sizeof(*sensors))) == NULL) {
		nrm_log_error("Allocating sensors failed\n");
		goto cleanup_scope;
	}
	for (int i = 0; i < EventCodeCnt; i++) {
		nrm_string_t sensor_name =
			nrm_string_fromprintf("nrm.extra.perf.%s.%u",
					      getpid(), EventCodeStrs[i]);
		if ((sensors[i] = nrm_sensor_create(sensor_name)) == NULL) {
			nrm_log_error("Sensor creation failed\n");
			nrm_string_decref(sensor_name);
			goto cleanup_sensor;
		}
		free(sensor_name);
		if (nrm_client_add_sensor(client, sensors[i]) != 0) {
			nrm_string_decref(sensor_name);
			nrm_log_error("Adding sensor failed\n");
			goto cleanup_sensor;
		}
		nrm_string_decref(sensor_name);
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
	for (int i = 0; i < EventCodeCnt; i++) {
		int EventCode;
		err = PAPI_event_name_to_code(EventCodeStrs[i], &EventCode);
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
		        EventCodeStrs[i], EventCode);
	}

	int pid = fork();
	if (pid < 0) {
		nrm_log_error("perfwrapper fork error\n");
		goto cleanup;
	} else if (pid == 0) {
		/* child, needs to exec the cmd */
		if (cmpinfo->attach_must_ptrace)
			if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
				nrm_log_error(
				        "ptrace(PTRACE_TRACEME) failed\n");
				_exit(EXIT_FAILURE);
			}

		err = execvp(argv[optind], &argv[optind]);
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
		long long sleeptime = 1e9 / freq;
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

		for (int i = 0; i < EventCodeCnt; i++) {
			if (nrm_client_send_event(client, time, sensors[i],
			                          scope, counters[i]) != 0) {
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
	for (int i = 0; i < EventCodeCnt; i++)
		nrm_client_send_event(client, time, sensors[i], scope,
		                      counters[i]);

cleanup:
	free(counters);
cleanup_sensor:
	for (int i = 0; i < EventCodeCnt; i++)
		if (sensors[i])
			nrm_sensor_destroy(&sensors[i]);
	free(sensors);
cleanup_scope:
	/* if we had to add the scope to the daemon, make sure to clean up after
	 * ourselves
	 */
	if (custom_scope)
		nrm_client_remove_scope(client, scope);
	nrm_scope_destroy(scope);
cleanup_client:
	nrm_client_destroy(&client);
cleanup_postinit:
	nrm_finalize();
cleanup_preinit:
	free(EventCodeStrs);
	exit(ret);
}
