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
 * and the NRM downstream interface. Resources detected via hwloc and GEOPM. The
 * `geopmd` service must be running. The `msr` module must be enabled via
 * `modprobe msr` if its installed. This utility may also need root privileges
 * to detect many basic signals like CPU_POWER.
 */

#include <limits.h>
#include <stddef.h>
#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <geopm_pio.h>
#include <geopm_topo.h>
#include <getopt.h>
#include <hwloc.h>
#include <limits.h>
#include <math.h>
#include <nrm.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int log_level = NRM_LOG_DEBUG;
volatile sig_atomic_t stop = 0;

static nrm_client_t *client;
static nrm_scope_t *scope;
static nrm_sensor_t *sensor;

static char *upstream_uri = "tcp://127.0.0.1";
static int pub_port = 2345;
static int rpc_port = 3456;

char *usage =
        "usage: nrm-power [options] \n"
        "     options:\n"
        "            -s	 --signals           Single GEOPM Signal name. Default: both CPU_POWER and DRAM_POWER\n"
        "                                       Use `sudo geopmread` to determine valid signal names         \n"
        "            -v, --verbose           Produce verbose output. Log messages will be displayed to stderr\n"
        "            -h, --help              Displays this help message\n";

// handler for interrupt?
void interrupt(int signum)
{
	nrm_log_debug("Interrupt caught. Exiting loop.\n");
	stop = 1;
}

int get_cpu_idx(hwloc_topology_t topology, int cpu)
{
	hwloc_obj_t pu;
	pu = hwloc_get_pu_obj_by_os_index(topology, cpu);
	return pu->logical_index;
}

int nrm_extra_create_name_ssu(const char *pattern,
                              const char *extra,
                              unsigned int idx,
                              char **name)
{
	char *buf;
	size_t bufsize;
	bufsize = snprintf(NULL, 0, "%s.%s.%u", pattern, extra, idx);
	bufsize++;
	buf = calloc(1, bufsize);
	if (!buf)
		return -NRM_ENOMEM;
	snprintf(buf, bufsize, "%s.%s.%u", pattern, extra, idx);
	*name = buf;
	return 0;
}

int nrm_extra_find_scope(nrm_client_t *client, nrm_scope_t **scope, int *added)
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
		assert(nrm_client_add_scope(client, *scope) == 0);
	}
	nrm_vector_destroy(&nrmd_scopes);
	*added = !newscope;
	return 0;
}

struct signal_info_s {
	nrm_string_t signal_name;
	char *domain_name;
	int domain_type;
	int num_domains;
};

typedef struct signal_info_s signal_info_t;

int main(int argc, char **argv)
{
	int j, char_opt, err;
	double freq = 1;
	size_t i;
	nrm_vector_t *signal_args = NULL;
	nrm_vector_t *signal_info_list = NULL;

	assert(nrm_vector_create(&signal_args, sizeof(nrm_string_t)) ==
	       NRM_SUCCESS);
	assert(nrm_vector_create(&signal_info_list, sizeof(signal_info_t)) ==
	       NRM_SUCCESS);

	nrm_init(NULL, NULL);
	assert(nrm_log_init(stderr, "nrm.extra.geopm") == 0);

	nrm_log_setlevel(log_level);
	nrm_log_debug("NRM logging initialized.\n");

	// register callback handler for interrupt
	signal(SIGINT, interrupt);

	// TODO: fix "-v" not being parsed as verbose
	while (1) {
		static struct option long_options[] = {
		        {"signals", required_argument, 0, 's'},
		        {"verbose", no_argument, &log_level, 1},
		        {"help", no_argument, 0, 'h'},
		        {"frequency", required_argument, 0, 'f'},
		        {0, 0, 0, 0}};

		int option_index = 0;
		char_opt = getopt_long(argc, argv, "s:vhf:", long_options,
		                       &option_index);

		if (char_opt == -1)
			break;
		switch (char_opt) {
		case 0:
			break;
		case 's':
			nrm_log_debug("Parsed signal %s\n", optarg);
			nrm_string_t cast_optarg = nrm_string_fromchar(optarg);
			nrm_vector_push_back(signal_args, &cast_optarg);
			break;
		case 'v':
			log_level = NRM_LOG_DEBUG;
			break;
		case 'f':
			freq = strtod(optarg, NULL);
			break;
		case 'h':
			fprintf(stderr, "%s", usage);
			exit(EXIT_SUCCESS);
		case '?':
		default:
			fprintf(stderr, "Wrong option argument\n");
			fprintf(stderr, "%s", usage);
			exit(EXIT_FAILURE);
		}
	}

	nrm_client_create(&client, upstream_uri, pub_port, rpc_port);
	nrm_log_debug("NRM client initialized.\n");
	assert(client != NULL);

	// create sensor
	const char *name = "nrm.sensor.power-geopm";
	sensor = nrm_sensor_create(name);

	// client add sensor
	assert(nrm_client_add_sensor(client, sensor) == 0);

	int nsignals = geopm_pio_num_signal_name();
	assert(nsignals > 0); // just check that we can obtain signals
	nrm_log_debug("GEOPM detects %d possible signals\n", nsignals);

	size_t n_signals = 1;
	assert(nrm_vector_length(signal_args, &n_signals) == NRM_SUCCESS);

	nrm_string_t CPU_ENERGY = nrm_string_fromchar("CPU_ENERGY");
	nrm_string_t DRAM_ENERGY = nrm_string_fromchar("DRAM_ENERGY");
	if (n_signals == 0) {
		nrm_vector_push_back(signal_args, &CPU_ENERGY);
		nrm_vector_push_back(signal_args, &DRAM_ENERGY);
		nrm_log_debug(
		        "Measuring CPU_ENERGY and DRAM_ENERGY by default\n");
		n_signals = 2;
	}

	// this loop will obtain our signal information
	int domain_type;
	nrm_string_t *signal_name;
	char domain_name[NAME_MAX + 1];
	for (i = 0; i < n_signals; i++) {
		void *p;
		nrm_vector_get(signal_args, i, &p);
		signal_name = (nrm_string_t *)p;
		nrm_log_debug("Retrieved %s for signal_info construction\n",
		              *signal_name);

		signal_info_t *ret = calloc(1, sizeof(signal_info_t));
		ret->signal_name = *signal_name;

		domain_type = geopm_pio_signal_domain_type(*signal_name);
		if (domain_type < 0) {
			nrm_log_error(
			        "Unable to parse domain. Either the signal name is incorrect, or you must sudo-run this utility.\n"); // GEOPM_DOMAIN_INVALID = -1
			exit(EXIT_FAILURE);
		}

		ret->domain_type = domain_type;
		ret->num_domains = geopm_topo_num_domain(domain_type);
		assert(ret->num_domains >= 0);

		err = geopm_topo_domain_name(domain_type, NAME_MAX,
		                             domain_name);
		assert(err == 0);
		nrm_log_debug("We get signal: %s. Main screen turn on.\n",
		              domain_name);
		ret->domain_name = domain_name;

		nrm_vector_push_back(signal_info_list, ret);
	}

	hwloc_topology_t topology;
	hwloc_obj_t socket;
	hwloc_cpuset_t cpus;

	assert(hwloc_topology_init(&topology) == 0);
	assert(hwloc_topology_load(topology) == 0);

	nrm_scope_t *custom_scopes[n_signals], *scopes[n_signals];
	signal_info_t *signal_info;

	char *scope_name;
	int added, n_scopes = 0, n_numa_scopes = 0, n_cpu_scopes = 0,
	           n_custom_scopes = 0, n_gpu_scopes = 0, cpu_idx, cpu;

	// signals like CPU_POWER belong to "package"
	// creating scopes for each detected "package", "gpu", and
	// "memory"/

	for (i = 0; i < n_signals; i++) {
		void *p;
		nrm_vector_get(signal_info_list, i, &p);
		signal_info = (signal_info_t *)p;

		for (j = 0; j < signal_info->num_domains; j++) {
			err = nrm_extra_create_name_ssu(
			        "nrm.geopm", signal_info->domain_name, j,
			        &scope_name);
			scope = nrm_scope_create(scope_name);
			if (strcmp(signal_info->domain_name, "package")) {

				socket = hwloc_get_obj_by_type(
				        topology, HWLOC_OBJ_SOCKET, j);
				cpus = socket->cpuset;
				hwloc_bitmap_foreach_begin(cpu, cpus)
				        cpu_idx = get_cpu_idx(topology, cpu);
				nrm_scope_add(scope, NRM_SCOPE_TYPE_CPU,
				              cpu_idx);
				hwloc_bitmap_foreach_end();
				n_cpu_scopes++;

			} else if (strcmp(signal_info->domain_name, "gpu")) {
				nrm_scope_add(scope, NRM_SCOPE_TYPE_GPU, j);

			} else if (strcmp(signal_info->domain_name, "memory")) {
				nrm_scope_add(scope, NRM_SCOPE_TYPE_NUMA, j);
			}

			scopes[n_scopes] = scope;
			n_scopes++;
			nrm_extra_find_scope(client, &scope, &added);
			if (added) {
				custom_scopes[n_custom_scopes] = scope;
				n_custom_scopes++;
			}
		}
	}

	nrm_log_debug(
	        "%d NRM scopes initialized (%d NUMA, %d CPU, %d GPU, %d custom)\n",
	        n_scopes, n_numa_scopes, n_cpu_scopes, n_gpu_scopes,
	        n_custom_scopes);

	nrm_time_t before_time, after_time;

	double sleeptime = 1 / freq;
	int scope_idx;

	while (true) {

		nrm_time_gettime(&before_time);

		/* sleep for a frequency */
		struct timespec req, rem;
		req.tv_sec = ceil(sleeptime);
		req.tv_nsec = sleeptime * 1e9 - ceil(sleeptime) * 1e9;

		err = nanosleep(&req, &rem);
		if (err == -1 && errno == EINTR) {
			nrm_log_error("interrupted during sleep, exiting\n");
			break;
		}

		nrm_time_gettime(&after_time);

		scope_idx = 0;

		for (i = 0; i < n_signals; i++) {
			void *p;
			nrm_vector_get(signal_info_list, i, &p);
			signal_info = (signal_info_t *)p;

			for (j = 0; j < signal_info->num_domains; j++) {
				double value = 0;
				err = geopm_pio_read_signal(
				        signal_info->signal_name,
				        signal_info->domain_type, j, &value);
				nrm_log_debug(
				        "%s.%d:%s - energy measurement: %f\n",
				        signal_info->domain_name, j,
				        signal_info->signal_name, value);
				nrm_client_send_event(client, after_time,
				                      sensor, scopes[scope_idx],
				                      value);
				scope_idx++;
			}
		}

		if (err == -1 || errno == EINTR) {
			nrm_log_error("Interrupted. Exiting\n");
			break;
		}
	}

	/* final send here */
	for (i = 0; i < n_signals; i++) {
		void *p;
		nrm_vector_get(signal_info_list, i, &p);
		signal_info = (signal_info_t *)p;

		for (j = 0; j < signal_info->num_domains; j++) {
			double value = 0;
			err = geopm_pio_read_signal(signal_info->signal_name,
			                            signal_info->domain_type, j,
			                            &value);
			nrm_client_send_event(client, after_time, sensor,
			                      scopes[scope_idx], value);
			scope_idx++;
		}
	}

	for (j = 0; j < n_custom_scopes; j++) {
		nrm_client_remove_scope(client, custom_scopes[j]);
	}
	for (j = 0; j < n_scopes; j++) {
		nrm_scope_destroy(scopes[j]);
	}

	nrm_log_debug("NRM scopes deleted.\n");

	nrm_sensor_destroy(&sensor);
	nrm_client_destroy(&client);

	nrm_string_decref(CPU_ENERGY);
	nrm_string_decref(DRAM_ENERGY);

	nrm_vector_destroy(&signal_args);
	nrm_vector_destroy(&signal_info_list);

	nrm_finalize();
	hwloc_bitmap_free(cpus);
	hwloc_topology_destroy(topology);

	exit(EXIT_SUCCESS);
}
