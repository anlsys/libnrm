/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

/* Filename: nrm-nvml.c
 *
 * Description: Implements middleware between the NVIDIA Management Library
 * (NVML) and the NRM downstream interface. GPUs are detected via NVML. The
 * NVIDIA driver (providing libnvidia-ml.so) must be installed for this utility
 * to run.
 *
 * Supported events (pass via -e/--event, default is nrm.nvml.power):
 *   nrm.nvml.power              - board power draw, in Watts
 *   nrm.nvml.temperature       - GPU die temperature, in degrees Celsius
 *   nrm.nvml.gpu_utilization   - GPU compute utilization, in percent
 *   nrm.nvml.memory_utilization - GPU memory-controller utilization, in percent
 *   nrm.nvml.memory_used       - used device memory, in bytes
 *
 * With --enable-power-control, one continuous actuator per GPU is exposed
 * (nrm.nvml.gpu.power_limit.<idx>) to set the board power-management limit via
 * nvmlDeviceSetPowerManagementLimit. This typically requires root privileges.
 */

#include "nrm.h"
#include "nrm/tools.h"
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <nvml.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static nrm_client_t *client;
static nrm_vector_t *events;
static size_t num_events;
static unsigned int num_devices;
static pthread_mutex_t nvml_lock;

/* actuator state: one continuous actuator per (supported) device */
static int power_control;

struct nrm_nvml_actinfo_s {
	nrm_actuator_t *actuator;
	unsigned int device_idx;
};

typedef struct nrm_nvml_actinfo_s nrm_nvml_actinfo_t;

static nrm_vector_t *actuators; /* nrm_nvml_actinfo_t */

/* the kind of measurement an event carries: selects the NVML call used to
 * read it and the unit conversion applied before sending.
 */
enum nrm_nvml_kind_e {
	NRM_NVML_POWER = 0, /* mW -> W */
	NRM_NVML_TEMPERATURE, /* degrees C */
	NRM_NVML_GPU_UTIL, /* percent */
	NRM_NVML_MEMORY_UTIL, /* percent */
	NRM_NVML_MEMORY_USED, /* bytes */
	NRM_NVML_KIND_MAX
};

struct nrm_nvml_kindinfo_s {
	const char *name;
	enum nrm_nvml_kind_e kind;
};

/* the set of events understood by this tool */
static const struct nrm_nvml_kindinfo_s nrm_nvml_kinds[] = {
        {"nrm.nvml.power", NRM_NVML_POWER},
        {"nrm.nvml.temperature", NRM_NVML_TEMPERATURE},
        {"nrm.nvml.gpu_utilization", NRM_NVML_GPU_UTIL},
        {"nrm.nvml.memory_utilization", NRM_NVML_MEMORY_UTIL},
        {"nrm.nvml.memory_used", NRM_NVML_MEMORY_USED},
};
static const size_t nrm_nvml_num_kinds =
        sizeof(nrm_nvml_kinds) / sizeof(nrm_nvml_kinds[0]);

struct nrm_nvml_eventinfo_s {
	nrm_string_t name;
	enum nrm_nvml_kind_e kind;
	nrm_sensor_t *sensor;
	nrm_vector_t *scopes; /* nrm_scope_t *, one per device */
};

typedef struct nrm_nvml_eventinfo_s nrm_nvml_eventinfo_t;

/* device handles, indexed the same way as every event's scopes vector */
static nvmlDevice_t *devices;

/* look up the kind for a given event name, returns -1 if unknown */
static int nrm_nvml_kind_from_name(const char *name)
{
	for (size_t i = 0; i < nrm_nvml_num_kinds; i++)
		if (!strcmp(name, nrm_nvml_kinds[i].name))
			return (int)nrm_nvml_kinds[i].kind;
	return -1;
}

/* read a single measurement from one device, applying unit conversion. On
 * failure, logs the NVML error and returns non-zero (value left untouched).
 */
static int
nrm_nvml_read(nvmlDevice_t device, enum nrm_nvml_kind_e kind, double *value)
{
	nvmlReturn_t rc;
	switch (kind) {
	case NRM_NVML_POWER: {
		unsigned int power;
		rc = nvmlDeviceGetPowerUsage(device, &power);
		if (rc == NVML_SUCCESS)
			*value = (double)power / 1000.0; /* mW -> W */
		break;
	}
	case NRM_NVML_TEMPERATURE: {
		unsigned int temp;
		rc = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU,
		                              &temp);
		if (rc == NVML_SUCCESS)
			*value = (double)temp;
		break;
	}
	case NRM_NVML_GPU_UTIL: {
		nvmlUtilization_t util;
		rc = nvmlDeviceGetUtilizationRates(device, &util);
		if (rc == NVML_SUCCESS)
			*value = (double)util.gpu;
		break;
	}
	case NRM_NVML_MEMORY_UTIL: {
		nvmlUtilization_t util;
		rc = nvmlDeviceGetUtilizationRates(device, &util);
		if (rc == NVML_SUCCESS)
			*value = (double)util.memory;
		break;
	}
	case NRM_NVML_MEMORY_USED: {
		nvmlMemory_t mem;
		rc = nvmlDeviceGetMemoryInfo(device, &mem);
		if (rc == NVML_SUCCESS)
			*value = (double)mem.used;
		break;
	}
	default:
		nrm_log_error("Unknown NVML event kind %d\n", (int)kind);
		return -1;
	}

	if (rc != NVML_SUCCESS) {
		nrm_log_error("NVML read error: %s\n", nvmlErrorString(rc));
		return -1;
	}
	return 0;
}

/* de-duplicate a newly created scope against the ones the daemon already
 * knows about, following the same convention as nrm-geopm.
 */
static int nrm_nvml_find_scope(nrm_scope_t **scope, int *added)
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

/* build the sensor and per-device scopes for a single event name */
static int nrm_nvml_prepare_event(nrm_string_t *s)
{
	nrm_nvml_eventinfo_t event;

	int kind = nrm_nvml_kind_from_name(*s);
	if (kind < 0) {
		nrm_log_error("Unknown NVML event: %s\n", *s);
		return -1;
	}

	event.name = *s;
	nrm_string_incref(event.name);
	event.kind = (enum nrm_nvml_kind_e)kind;

	/* sensor info */
	event.sensor = nrm_sensor_create(event.name);
	nrm_client_add_sensor(client, event.sensor);

	/* one scope per GPU device */
	nrm_vector_create(&event.scopes, sizeof(nrm_scope_t *));
	for (unsigned int i = 0; i < num_devices; i++) {
		nrm_string_t scope_name =
		        nrm_string_fromprintf("%s.gpu.%u", event.name, i);
		int added_scope;
		nrm_scope_t *scope = nrm_scope_create(scope_name);
		nrm_scope_add(scope, NRM_SCOPE_TYPE_GPU, i);
		nrm_nvml_find_scope(&scope, &added_scope);
		nrm_vector_push_back(event.scopes, &scope);
		nrm_string_decref(scope_name);
	}
	nrm_vector_push_back(events, &event);
	nrm_log_debug("Created sensor %s from event %s\n", event.sensor->uuid,
	              event.name);
	return 0;
}

int nrm_nvml_timer_callback(nrm_reactor_t *reactor)
{
	(void)reactor;
	nrm_time_t time;
	nrm_time_gettime(&time);
	nrm_vector_foreach(events, iterator)
	{
		nrm_nvml_eventinfo_t *event = nrm_vector_iterator_get(iterator);
		pthread_mutex_lock(&nvml_lock);
		for (unsigned int i = 0; i < num_devices; i++) {
			double value = 0.0;
			nrm_scope_t **s;
			nrm_vector_get_withtype(nrm_scope_t *, event->scopes, i,
			                        s);
			if (nrm_nvml_read(devices[i], event->kind, &value)) {
				/* device may not support this metric; skip */
				continue;
			}
			nrm_log_debug("%s.%u - measurement: %f\n", event->name,
			              i, value);
			nrm_client_send_event(client, time, event->sensor, *s,
			                      value);
		}
		pthread_mutex_unlock(&nvml_lock);
	}
	return 0;
}

/* resolve an actuator uuid to its device index, or -1 if not found */
static int nrm_nvml_actuator_device(nrm_uuid_t *uuid)
{
	nrm_vector_foreach(actuators, iterator)
	{
		nrm_nvml_actinfo_t *ai = nrm_vector_iterator_get(iterator);
		nrm_string_t auuid = nrm_actuator_uuid(ai->actuator);
		if (!nrm_string_cmp(auuid, *uuid))
			return (int)ai->device_idx;
	}
	return -1;
}

int nrm_nvml_act_callback(nrm_uuid_t *uuid, double value)
{
	int idx = nrm_nvml_actuator_device(uuid);
	if (idx < 0) {
		nrm_log_error("Received actuation for unknown actuator\n");
		return -1;
	}
	/* NVML expects the limit in milliwatts */
	unsigned int limit = (unsigned int)(value * 1000.0);
	nrm_log_debug("Setting power limit of GPU %d to %f W (%u mW)\n", idx,
	              value, limit);
	pthread_mutex_lock(&nvml_lock);
	nvmlReturn_t rc =
	        nvmlDeviceSetPowerManagementLimit(devices[idx], limit);
	pthread_mutex_unlock(&nvml_lock);
	if (rc != NVML_SUCCESS) {
		nrm_log_error("Failed to set power limit on GPU %d: %s\n", idx,
		              nvmlErrorString(rc));
		return -1;
	}
	return 0;
}

/* create one continuous power-limit actuator per device, registering it with
 * the client. Returns 0 on success.
 */
static int nrm_nvml_prepare_actuators(void)
{
	nrm_vector_create(&actuators, sizeof(nrm_nvml_actinfo_t));
	for (unsigned int i = 0; i < num_devices; i++) {
		nvmlEnableState_t mode;
		nvmlReturn_t rc =
		        nvmlDeviceGetPowerManagementMode(devices[i], &mode);
		if (rc != NVML_SUCCESS || mode != NVML_FEATURE_ENABLED) {
			nrm_log_error(
			        "GPU %u does not support power management, "
			        "skipping actuator\n",
			        i);
			continue;
		}

		unsigned int minlimit, maxlimit;
		rc = nvmlDeviceGetPowerManagementLimitConstraints(
		        devices[i], &minlimit, &maxlimit);
		if (rc != NVML_SUCCESS) {
			nrm_log_error("GPU %u: unable to read power limit "
			              "constraints: %s\n",
			              i, nvmlErrorString(rc));
			continue;
		}

		nrm_string_t name =
		        nrm_string_fromprintf("nrm.nvml.gpu.power_limit.%u", i);
		nrm_actuator_t *actuator = nrm_actuator_continuous_create(name);
		nrm_string_decref(name);

		/* NVML reports constraints in milliwatts */
		nrm_actuator_continuous_set_limits(actuator,
		                                   (double)minlimit / 1000.0,
		                                   (double)maxlimit / 1000.0);
		nrm_actuator_set_value(actuator, (double)maxlimit / 1000.0);

		if (nrm_client_add_actuator(client, actuator)) {
			nrm_log_error(
			        "Unable to add power actuator for GPU %u\n", i);
			nrm_actuator_destroy(&actuator);
			continue;
		}
		nrm_nvml_actinfo_t ai = {.actuator = actuator, .device_idx = i};
		nrm_vector_push_back(actuators, &ai);
		nrm_log_debug(
		        "GPU %u: power actuator added, limits [%f, %f] W\n", i,
		        (double)minlimit / 1000.0, (double)maxlimit / 1000.0);
	}
	return 0;
}

/* scan argv for --enable-power-control, set the global flag, and remove it
 * from argv so the standard argument parser doesn't reject it. Returns the
 * (possibly reduced) argument count.
 */
static int nrm_nvml_extract_power_control(int argc, char **argv)
{
	int out = 0;
	for (int i = 0; i < argc; i++) {
		if (i > 0 && !strcmp(argv[i], "--enable-power-control")) {
			power_control = 1;
			continue;
		}
		argv[out++] = argv[i];
	}
	argv[out] = NULL;
	return out;
}

int main(int argc, char **argv)
{
	int err;
	nrm_tools_args_t args;

	nrm_init(NULL, NULL);
	assert(nrm_log_init(stderr, "nrm-nvml") == 0);

	int ret = EXIT_FAILURE;
	int nvml_initialized = 0;

	/* handle our tool-specific flag before the generic parser sees it */
	argc = nrm_nvml_extract_power_control(argc, argv);

	args.progname = "nrm-nvml";
	args.flags = NRM_TOOLS_ARGS_FLAG_FREQ | NRM_TOOLS_ARGS_FLAG_EVENT;

	err = nrm_tools_parse_args(argc, argv, &args);
	if (err < 0) {
		nrm_log_error("errors during argument parsing\n");
		nrm_tools_print_help(&args);
		exit(EXIT_FAILURE);
	}

	if (args.ask_help) {
		nrm_tools_print_help(&args);
		fprintf(stdout,
		        "\nnrm-nvml specific options:\n"
		        "  --enable-power-control    expose per-GPU power-limit "
		        "actuators (needs privileges)\n"
		        "\nsupported events (-e/--event):\n"
		        "  nrm.nvml.power\n"
		        "  nrm.nvml.temperature\n"
		        "  nrm.nvml.gpu_utilization\n"
		        "  nrm.nvml.memory_utilization\n"
		        "  nrm.nvml.memory_used\n");
		exit(EXIT_SUCCESS);
	}
	if (args.ask_version) {
		nrm_tools_print_version(&args);
		exit(EXIT_SUCCESS);
	}

	nrm_log_setlevel(args.log_level);

	nrm_vector_length(args.events, &num_events);
	if (num_events == 0) {
		nrm_string_t s = nrm_string_fromchar("nrm.nvml.power");
		nrm_vector_push_back(args.events, &s);
		nrm_log_debug("Measuring nrm.nvml.power by default\n");
	}

	/* initialize NVML after arg parsing, so --help/--version work on
	 * machines without an NVIDIA driver (e.g. CI).
	 */
	nvmlReturn_t nvrc = nvmlInit_v2();
	if (nvrc != NVML_SUCCESS) {
		nrm_log_error("Failed to initialize NVML: %s\n",
		              nvmlErrorString(nvrc));
		goto cleanup_postinit;
	}
	nvml_initialized = 1;

	nvrc = nvmlDeviceGetCount_v2(&num_devices);
	if (nvrc != NVML_SUCCESS) {
		nrm_log_error("Failed to query GPU count: %s\n",
		              nvmlErrorString(nvrc));
		goto cleanup_postinit;
	}
	if (num_devices == 0) {
		nrm_log_error("No NVIDIA GPUs detected\n");
		goto cleanup_postinit;
	}
	nrm_log_debug("NVML detects %u GPU(s)\n", num_devices);

	devices = calloc(num_devices, sizeof(*devices));
	if (devices == NULL) {
		nrm_log_error("Out of memory allocating device handles\n");
		goto cleanup_postinit;
	}
	for (unsigned int i = 0; i < num_devices; i++) {
		nvrc = nvmlDeviceGetHandleByIndex_v2(i, &devices[i]);
		if (nvrc != NVML_SUCCESS) {
			nrm_log_error("Failed to get handle for GPU %u: %s\n",
			              i, nvmlErrorString(nvrc));
			goto cleanup_devices;
		}
	}

	if (nrm_client_create(&client, args.upstream_uri, args.pub_port,
	                      args.rpc_port) != 0 ||
	    client == NULL) {
		nrm_log_error("Client creation failed\n");
		goto cleanup_devices;
	}
	nrm_log_debug("NRM client initialized.\n");

	pthread_mutex_init(&nvml_lock, NULL);

	nrm_vector_create(&events, sizeof(nrm_nvml_eventinfo_t));
	nrm_vector_foreach(args.events, iterator)
	{
		nrm_string_t *s = nrm_vector_iterator_get(iterator);
		if (nrm_nvml_prepare_event(s)) {
			nrm_log_error("Failed to prepare event %s\n", *s);
			goto cleanup;
		}
	}

	if (power_control) {
		nrm_nvml_prepare_actuators();
		nrm_client_set_actuate_listener(client, nrm_nvml_act_callback);
		nrm_client_start_actuate_listener(client);
		nrm_log_debug("Power-control actuators enabled\n");
	}

	nrm_reactor_t *reactor;
	err = nrm_reactor_create(&reactor, NULL);
	if (err) {
		nrm_log_error("error during reactor creation\n");
		goto cleanup;
	}

	nrm_reactor_user_callbacks_t callbacks = {
	        .signal = NULL,
	        .timer = nrm_nvml_timer_callback,
	};
	nrm_reactor_setcallbacks(reactor, callbacks);
	nrm_time_t sleeptime = nrm_time_fromfreq(args.freq);
	nrm_reactor_settimer(reactor, sleeptime);
	nrm_reactor_start(reactor);
	ret = EXIT_SUCCESS;

	nrm_reactor_destroy(&reactor);
cleanup:
	if (power_control && actuators != NULL) {
		nrm_vector_foreach(actuators, iterator)
		{
			nrm_nvml_actinfo_t *ai =
			        nrm_vector_iterator_get(iterator);
			nrm_client_remove_actuator(client, ai->actuator);
			nrm_actuator_destroy(&ai->actuator);
		}
		nrm_vector_destroy(&actuators);
	}
	if (events != NULL) {
		nrm_vector_foreach(events, iterator)
		{
			nrm_nvml_eventinfo_t *e =
			        nrm_vector_iterator_get(iterator);
			nrm_client_remove_sensor(client, e->sensor);
			nrm_sensor_destroy(&e->sensor);
			for (unsigned int i = 0; i < num_devices; i++) {
				nrm_scope_t **s;
				nrm_vector_get_withtype(nrm_scope_t *,
				                        e->scopes, i, s);
				nrm_scope_destroy(*s);
			}
			nrm_vector_destroy(&e->scopes);
			nrm_string_decref(e->name);
		}
		nrm_vector_destroy(&events);
	}
	nrm_client_destroy(&client);
	pthread_mutex_destroy(&nvml_lock);
cleanup_devices:
	free(devices);
cleanup_postinit:
	if (nvml_initialized)
		nvmlShutdown();
	nrm_tools_args_destroy(&args);
	nrm_finalize();
	return ret;
}
