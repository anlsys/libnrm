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

#include "internal/messages.h"
#include "internal/nrmi.h"
#include "internal/roles.h"

static int ask_help = 0;
static int ask_version = 0;
static char *upstream_uri = NRM_DEFAULT_UPSTREAM_URI;
static int pub_port = NRM_DEFAULT_UPSTREAM_PUB_PORT;
static int rpc_port = NRM_DEFAULT_UPSTREAM_RPC_PORT;
static nrm_client_t *client;

static struct option long_options[] = {
        {"help", no_argument, &ask_help, 1},
        {"version", no_argument, &ask_version, 1},
        {"uri", required_argument, NULL, 'u'},
        {"rpc-port", required_argument, NULL, 'r'},
        {"pub-port", required_argument, NULL, 'p'},
        {0, 0, 0, 0},
};

static const char *short_options = "+hVu:r:p:";

static const char *help[] = {"Usage: nrmc [options]\n\n", "Allowed options:\n",
                             "--help, -h    : print this help message\n",
                             "--version, -V : print program version\n", NULL};

void print_help()
{
	for (int i = 0; help[i] != NULL; i++)
		fprintf(stdout, "%s", help[i]);
}

void print_version()
{
	fprintf(stdout, "nrmc: version %s\n", nrm_version_string);
}

struct client_cmd {
	const char *name;
	int (*fn)(int, char **);
};

int cmd_run(int argc, char **argv)
{

	int err;
	char *manifest_name = NULL;
	static struct option cmd_run_long_options[] = {
	        {"manifest", required_argument, NULL, 'm'},
	        {0, 0, 0, 0},
	};

	static const char *cmd_run_short_options = ":m:";

	int c;
	int option_index = 0;
	while (1) {
		c = getopt_long(argc, argv, cmd_run_short_options,
		                cmd_run_long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'm':
			manifest_name = optarg;
			break;
		case '?':
			return EXIT_FAILURE;
		default:
			return EXIT_FAILURE;
		}
	}
	/* remove the parsed part */
	argc -= optind;
	argv = &(argv[optind]);

	/* ensure we have something here, otherwise there's no command to launch
	 */
	if (argc < 1)
		return EXIT_FAILURE;

	nrm_log_info("exec: argc: %u, argv[0]: %s\n", argc, argv[0]);
	err = execvp(argv[0], &argv[0]);
	return err;
}

int cmd_add_actuator(int argc, char **argv)
{

	/* no matter the arguments, only one extra parameter */
	if (argc != 2)
		return EXIT_FAILURE;

	int err;
	char *name = argv[1];
	nrm_actuator_t *actuator = nrm_actuator_create(name);

	err = nrm_client_add_actuator(client, actuator);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}
	json_t *json = nrm_actuator_to_json(actuator);
	json_dumpf(json, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_add_scope(int argc, char **argv)
{

	/* no matter the arguments, only one extra parameter */
	if (argc != 2)
		return EXIT_FAILURE;

	int err;
	nrm_scope_t *scope;
	json_t *param;
	json_error_t json_error;
	param = json_loads(argv[1], 0, &json_error);
	scope = nrm_scope_create();
	err = nrm_scope_from_json(scope, param);
	if (err)
		return EXIT_FAILURE;

	err = nrm_client_add_scope(client, scope);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}
	json_t *json = nrm_scope_to_json(scope);
	json_dumpf(json, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_add_slice(int argc, char **argv)
{

	if (argc < 2)
		return EXIT_FAILURE;

	int err;
	char *name = argv[1];
	nrm_slice_t *slice = nrm_slice_create(name);

	err = nrm_client_add_slice(client, slice);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}
	json_t *json = nrm_slice_to_json(slice);
	json_dumpf(json, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_add_sensor(int argc, char **argv)
{

	if (argc < 2)
		return EXIT_FAILURE;

	int err;
	char *name = argv[1];
	nrm_sensor_t *sensor = nrm_sensor_create(name);

	err = nrm_client_add_sensor(client, sensor);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}
	json_t *json = nrm_sensor_to_json(sensor);
	json_dumpf(json, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_find_actuator(int argc, char **argv)
{

	int err;
	static int ask_uuid = 0;
	static struct option cmd_run_long_options[] = {
	        {"uuid", no_argument, &ask_uuid, 1},
	        {0, 0, 0, 0},
	};

	static const char *cmd_run_short_options = ":u";

	int c;
	int option_index = 0;
	while (1) {
		c = getopt_long(argc, argv, cmd_run_short_options,
		                cmd_run_long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'u':
			ask_uuid = 1;
			break;
		case '?':
			return EXIT_FAILURE;
		default:
			return EXIT_FAILURE;
		}
	}
	/* remove the parsed part */
	argc -= optind;
	argv = &(argv[optind]);

	if (argc < 1)
		return EXIT_FAILURE;

	nrm_vector_t *results;
	char *name = NULL;
	nrm_uuid_t *uuid = NULL;
	if (ask_uuid)
		uuid = nrm_uuid_create_fromchar(argv[0]);
	else
		name = argv[0];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_ACTUATOR, name, uuid,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(results, &len);

	json_t *array = json_array();
	for (size_t i = 0; i < len; i++) {
		nrm_actuator_t *a;
		void *p;
		nrm_vector_get(results, i, &p);
		a = (nrm_actuator_t *)p;
		json_t *json = nrm_actuator_to_json(a);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_find_scope(int argc, char **argv)
{

	int err;
	static int ask_uuid = 0;
	static struct option cmd_run_long_options[] = {
	        {"uuid", no_argument, &ask_uuid, 1},
	        {0, 0, 0, 0},
	};

	static const char *cmd_run_short_options = ":u";

	int c;
	int option_index = 0;
	while (1) {
		c = getopt_long(argc, argv, cmd_run_short_options,
		                cmd_run_long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'u':
			ask_uuid = 1;
			break;
		case '?':
			return EXIT_FAILURE;
		default:
			return EXIT_FAILURE;
		}
	}
	/* remove the parsed part */
	argc -= optind;
	argv = &(argv[optind]);

	if (argc < 1)
		return EXIT_FAILURE;

	nrm_vector_t *results;
	char *name = NULL;
	nrm_uuid_t *uuid = NULL;
	if (ask_uuid)
		uuid = nrm_uuid_create_fromchar(argv[0]);
	else
		name = argv[0];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_SCOPE, name, uuid,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(results, &len);

	json_t *array = json_array();
	for (size_t i = 0; i < len; i++) {
		nrm_scope_t *s;
		void *p;
		nrm_vector_get(results, i, &p);
		s = (nrm_scope_t *)p;
		json_t *json = nrm_scope_to_json(s);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_find_sensor(int argc, char **argv)
{

	int err;
	static int ask_uuid = 0;
	static struct option cmd_run_long_options[] = {
	        {"uuid", no_argument, &ask_uuid, 1},
	        {0, 0, 0, 0},
	};

	static const char *cmd_run_short_options = ":u";

	int c;
	int option_index = 0;
	while (1) {
		c = getopt_long(argc, argv, cmd_run_short_options,
		                cmd_run_long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'u':
			ask_uuid = 1;
			break;
		case '?':
			return EXIT_FAILURE;
		default:
			return EXIT_FAILURE;
		}
	}
	/* remove the parsed part */
	argc -= optind;
	argv = &(argv[optind]);

	if (argc < 1)
		return EXIT_FAILURE;

	nrm_vector_t *results;
	char *name = NULL;
	nrm_uuid_t *uuid = NULL;
	if (ask_uuid)
		uuid = nrm_uuid_create_fromchar(argv[0]);
	else
		name = argv[0];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_SENSOR, name, uuid,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(results, &len);

	json_t *array = json_array();
	for (size_t i = 0; i < len; i++) {
		nrm_sensor_t *s;
		void *p;
		nrm_vector_get(results, i, &p);
		s = (nrm_sensor_t *)p;
		json_t *json = nrm_sensor_to_json(s);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_find_slice(int argc, char **argv)
{

	int err;
	static int ask_uuid = 0;
	static struct option cmd_run_long_options[] = {
	        {"uuid", no_argument, &ask_uuid, 1},
	        {0, 0, 0, 0},
	};

	static const char *cmd_run_short_options = ":u";

	int c;
	int option_index = 0;
	while (1) {
		c = getopt_long(argc, argv, cmd_run_short_options,
		                cmd_run_long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'u':
			ask_uuid = 1;
			break;
		case '?':
			return EXIT_FAILURE;
		default:
			return EXIT_FAILURE;
		}
	}
	/* remove the parsed part */
	argc -= optind;
	argv = &(argv[optind]);

	if (argc < 1)
		return EXIT_FAILURE;

	nrm_vector_t *results;
	char *name = NULL;
	nrm_uuid_t *uuid = NULL;
	if (ask_uuid)
		uuid = nrm_uuid_create_fromchar(argv[0]);
	else
		name = argv[0];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_SLICE, name, uuid,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(results, &len);

	json_t *array = json_array();
	for (size_t i = 0; i < len; i++) {
		nrm_slice_t *s;
		void *p;
		nrm_vector_get(results, i, &p);
		s = (nrm_slice_t *)p;
		json_t *json = nrm_slice_to_json(s);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int client_listen_callback(nrm_uuid_t uuid,
                           nrm_time_t time,
                           nrm_scope_t scope,
                           double value)
{
	nrm_log_debug("event\n");
	return 0;
}

int cmd_listen(int argc, char **argv)
{
	/* no options at this time */
	if (argc < 1)
		return EXIT_FAILURE;

	nrm_string_t topic = nrm_string_fromchar(argv[0]);
	nrm_log_debug("listening to topic: %s\n", topic);

	nrm_client_set_event_listener(client, client_listen_callback);
	nrm_client_start_event_listener(client, topic);

	/* don't want to push a message queue into the user API, so do it this
	 * way. The callback will take care of printing events.
	 */
	while (1)
		;
	return 0;
}

int cmd_list_actuators(int argc, char **argv)
{
	/* no options at this time */
	(void)argc;
	(void)argv;

	int err;
	nrm_vector_t *actuators;

	err = nrm_client_list_actuators(client, &actuators);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(actuators, &len);

	json_t *array = json_array();
	for (size_t i = 0; i < len; i++) {
		nrm_actuator_t *r;
		void *p;
		nrm_vector_get(actuators, i, &p);
		r = (nrm_actuator_t *)p;
		json_t *json = nrm_actuator_to_json(r);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_list_scopes(int argc, char **argv)
{
	/* no options at this time */
	(void)argc;
	(void)argv;

	int err;
	nrm_vector_t *scopes;

	err = nrm_client_list_scopes(client, &scopes);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(scopes, &len);

	json_t *array = json_array();
	for (size_t i = 0; i < len; i++) {
		nrm_scope_t *s;
		void *p;
		nrm_vector_get(scopes, i, &p);
		s = (nrm_scope_t *)p;
		json_t *json = nrm_scope_to_json(s);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_list_sensors(int argc, char **argv)
{

	/* no options at this time */
	(void)argc;
	(void)argv;

	int err;
	nrm_vector_t *sensors;

	err = nrm_client_list_sensors(client, &sensors);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(sensors, &len);

	json_t *array = json_array();
	for (size_t i = 0; i < len; i++) {
		nrm_sensor_t *s;
		void *p;
		nrm_vector_get(sensors, i, &p);
		s = (nrm_sensor_t *)p;
		json_t *json = nrm_sensor_to_json(s);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_list_slices(int argc, char **argv)
{

	/* no options at this time */
	(void)argc;
	(void)argv;

	int err;
	nrm_vector_t *slices;

	err = nrm_client_list_slices(client, &slices);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(slices, &len);

	json_t *array = json_array();
	for (size_t i = 0; i < len; i++) {
		nrm_slice_t *s;
		void *p;
		nrm_vector_get(slices, i, &p);
		s = (nrm_slice_t *)p;
		json_t *json = nrm_slice_to_json(s);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_remove_actuator(int argc, char **argv)
{
	int err;
	static int ask_uuid = 0;
	static int ask_all = 0;
	static struct option cmd_run_long_options[] = {
	        {"uuid", no_argument, &ask_uuid, 1},
	        {"all", no_argument, &ask_uuid, 1},
	        {0, 0, 0, 0},
	};

	static const char *cmd_run_short_options = ":ua";

	int c;
	int option_index = 0;
	while (1) {
		c = getopt_long(argc, argv, cmd_run_short_options,
		                cmd_run_long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'u':
			ask_uuid = 1;
			break;
		case 'a':
			ask_all = 1;
			break;
		case '?':
			return EXIT_FAILURE;
		default:
			return EXIT_FAILURE;
		}
	}
	/* remove the parsed part */
	argc -= optind;
	argv = &(argv[optind]);

	if (argc < 1)
		return EXIT_FAILURE;

	nrm_vector_t *results;
	char *name = NULL;
	nrm_uuid_t *uuid = NULL;
	if (ask_uuid)
		uuid = nrm_uuid_create_fromchar(argv[0]);
	else
		name = argv[0];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_ACTUATOR, name, uuid,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(results, &len);
	/* either remove all, or just one (if we found one) */
	len = ask_all ? len : len > 0 ? 1 : 0;

	json_t *array = json_array();
	for (size_t i = 0; i < len; i++) {
		nrm_actuator_t *r;
		void *p;
		nrm_vector_get(results, i, &p);
		r = (nrm_actuator_t *)p;
		json_t *json = nrm_actuator_to_json(r);
		nrm_client_remove(client, NRM_MSG_TARGET_TYPE_ACTUATOR, r->uuid);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_remove_scope(int argc, char **argv)
{
	int err;
	static int ask_uuid = 0;
	static int ask_all = 0;
	static struct option cmd_run_long_options[] = {
	        {"uuid", no_argument, &ask_uuid, 1},
	        {"all", no_argument, &ask_uuid, 1},
	        {0, 0, 0, 0},
	};

	static const char *cmd_run_short_options = ":ua";

	int c;
	int option_index = 0;
	while (1) {
		c = getopt_long(argc, argv, cmd_run_short_options,
		                cmd_run_long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'u':
			ask_uuid = 1;
			break;
		case 'a':
			ask_all = 1;
			break;
		case '?':
			return EXIT_FAILURE;
		default:
			return EXIT_FAILURE;
		}
	}
	/* remove the parsed part */
	argc -= optind;
	argv = &(argv[optind]);

	if (argc < 1)
		return EXIT_FAILURE;

	nrm_vector_t *results;
	char *name = NULL;
	nrm_uuid_t *uuid = NULL;
	if (ask_uuid)
		uuid = nrm_uuid_create_fromchar(argv[0]);
	else
		name = argv[0];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_SCOPE, name, uuid,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(results, &len);
	/* either remove all, or just one (if we found one) */
	len = ask_all ? len : len > 0 ? 1 : 0;

	json_t *array = json_array();
	for (size_t i = 0; i < len; i++) {
		nrm_scope_t *s;
		void *p;
		nrm_vector_get(results, i, &p);
		s = (nrm_scope_t *)p;
		json_t *json = nrm_scope_to_json(s);
		nrm_client_remove(client, NRM_MSG_TARGET_TYPE_SCOPE, s->uuid);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_remove_sensor(int argc, char **argv)
{
	int err;
	static int ask_uuid = 0;
	static int ask_all = 0;
	static struct option cmd_run_long_options[] = {
	        {"uuid", no_argument, &ask_uuid, 1},
	        {"all", no_argument, &ask_uuid, 1},
	        {0, 0, 0, 0},
	};

	static const char *cmd_run_short_options = ":ua";

	int c;
	int option_index = 0;
	while (1) {
		c = getopt_long(argc, argv, cmd_run_short_options,
		                cmd_run_long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'u':
			ask_uuid = 1;
			break;
		case 'a':
			ask_all = 1;
			break;
		case '?':
			return EXIT_FAILURE;
		default:
			return EXIT_FAILURE;
		}
	}
	/* remove the parsed part */
	argc -= optind;
	argv = &(argv[optind]);

	if (argc < 1)
		return EXIT_FAILURE;

	nrm_vector_t *results;
	char *name = NULL;
	nrm_uuid_t *uuid = NULL;
	if (ask_uuid)
		uuid = nrm_uuid_create_fromchar(argv[0]);
	else
		name = argv[0];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_SENSOR, name, uuid,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(results, &len);
	/* either remove all, or just one (if we found one) */
	len = ask_all ? len : len > 0 ? 1 : 0;

	json_t *array = json_array();
	for (size_t i = 0; i < len; i++) {
		nrm_sensor_t *s;
		void *p;
		nrm_vector_get(results, i, &p);
		s = (nrm_sensor_t *)p;
		json_t *json = nrm_sensor_to_json(s);
		nrm_client_remove(client, NRM_MSG_TARGET_TYPE_SENSOR, s->uuid);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_remove_slice(int argc, char **argv)
{
	int err;
	static int ask_uuid = 0;
	static int ask_all = 0;
	static struct option cmd_run_long_options[] = {
	        {"uuid", no_argument, &ask_uuid, 1},
	        {"all", no_argument, &ask_uuid, 1},
	        {0, 0, 0, 0},
	};

	static const char *cmd_run_short_options = ":ua";

	int c;
	int option_index = 0;
	while (1) {
		c = getopt_long(argc, argv, cmd_run_short_options,
		                cmd_run_long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'u':
			ask_uuid = 1;
			break;
		case 'a':
			ask_all = 1;
			break;
		case '?':
			return EXIT_FAILURE;
		default:
			return EXIT_FAILURE;
		}
	}
	/* remove the parsed part */
	argc -= optind;
	argv = &(argv[optind]);

	if (argc < 1)
		return EXIT_FAILURE;

	nrm_vector_t *results;
	char *name = NULL;
	nrm_uuid_t *uuid = NULL;
	if (ask_uuid)
		uuid = nrm_uuid_create_fromchar(argv[0]);
	else
		name = argv[0];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_SLICE, name, uuid,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(results, &len);
	/* either remove all, or just one (if we found one) */
	len = ask_all ? len : len > 0 ? 1 : 0;

	json_t *array = json_array();
	for (size_t i = 0; i < len; i++) {
		nrm_slice_t *s;
		void *p;
		nrm_vector_get(results, i, &p);
		s = (nrm_slice_t *)p;
		json_t *json = nrm_slice_to_json(s);
		nrm_client_remove(client, NRM_MSG_TARGET_TYPE_SLICE, s->uuid);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_send_event(int argc, char **argv)
{
	/* no options at this time */
	if (argc < 2)
		return EXIT_FAILURE;

	char *name = argv[1];

	/* find sensor */
	int err;
	nrm_vector_t *results;
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_SENSOR, name, NULL,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(results, &len);

	assert(len == 1);
	nrm_sensor_t *s;
	void *p;
	nrm_vector_get(results, 0, &p);
	s = (nrm_sensor_t *)p;

	nrm_log_info("sending event\n");
	nrm_scope_t *scope = nrm_scope_create();
	nrm_scope_threadprivate(scope);
	nrm_time_t time;
	nrm_time_gettime(&time);
	nrm_client_send_event(client, time, s, scope, rand());

	return 0;
}

static struct client_cmd commands[] = {
        {"add-actuator", cmd_add_actuator},
        {"add-scope", cmd_add_scope},
        {"add-slice", cmd_add_slice},
        {"add-sensor", cmd_add_sensor},
        {"find-actuator", cmd_find_actuator},
        {"find-scope", cmd_find_scope},
        {"find-slice", cmd_find_slice},
        {"find-sensor", cmd_find_sensor},
        {"listen", cmd_listen},
        {"list-actuators", cmd_list_actuators},
        {"list-scopes", cmd_list_scopes},
        {"list-slices", cmd_list_slices},
        {"list-sensors", cmd_list_sensors},
        {"send-event", cmd_send_event},
        {"remove-actuator", cmd_remove_actuator},
        {"remove-scope", cmd_remove_scope},
        {"remove-slice", cmd_remove_slice},
        {"remove-sensor", cmd_remove_sensor},
        {"run", cmd_run},
        {0, 0},
};

int main(int argc, char *argv[])
{
	int c;
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
		case 'p':
			errno = 0;
			pub_port = strtol(optarg, NULL, 0);
			assert(errno == 0);
			break;
		case 'r':
			errno = 0;
			rpc_port = strtol(optarg, NULL, 0);
			assert(errno == 0);
			break;
		case 'u':
			upstream_uri = optarg;
			break;
		case 'V':
			ask_version = 1;
			break;
		case ':':
			fprintf(stderr, "nrmc: missing argument\n");
			exit(EXIT_FAILURE);
		case '?':
			fprintf(stderr, "nrmc: invalid option: %s\n",
			        argv[optind - 1]);
			exit(EXIT_FAILURE);
		default:
			fprintf(stderr, "nrmc: this should not happen\n");
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

	if (argc == 0) {
		print_help();
		exit(EXIT_FAILURE);
	}

	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "nrmc");
	nrm_log_setlevel(NRM_LOG_DEBUG);

	nrm_log_debug("after command line parsing: argc: %u argv[0]: %s\n",
	              argc, argv[0]);

	nrm_log_info("creating client\n");
	nrm_client_create(&client, upstream_uri, pub_port, rpc_port);

	int err = 0;
	for (int i = 0; commands[i].name != NULL; i++) {
		if (!strcmp(argv[0], commands[i].name)) {
			err = commands[i].fn(argc, argv);
			goto end;
		}
	}
	fprintf(stderr, "wrong command: %s\n", argv[0]);
	err = EXIT_FAILURE;
end:
	nrm_client_destroy(&client);
	nrm_finalize();
	return err;
}
