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
#include <getopt.h>

#include "internal/messages.h"

static nrm_client_t *client;

struct client_cmd {
	const char *name;
	int (*fn)(int, char **);
};

int cmd_connect(nrm_tools_args_t *args, int argc, char **argv)
{
	/* optional number of retries */
	if (argc > 3)
		return EXIT_FAILURE;

	int err = 0;
	unsigned int retries = 5;
	if (argc == 2)
		err = nrm_parse_uint(argv[1], &retries);
	assert(err == 0);

	for (size_t i = 0; i < retries; i++) {
		err = nrm_client_create(&client, args->upstream_uri,
		                        args->pub_port, args->rpc_port);
		if (err == 0)
			return 0;
		sleep(1);
	}
	return EXIT_FAILURE;
}

int cmd_actuate(int argc, char **argv)
{
	/* no options at this time */
	if (argc < 3)
		return EXIT_FAILURE;

	char *name = argv[1];
	double value = strtod(argv[2], NULL);

	/* find actuator */
	int err;
	nrm_vector_t *results;
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_ACTUATOR, name,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(results, &len);

	assert(len == 1);
	nrm_actuator_t **a;
	nrm_vector_get_withtype(nrm_actuator_t *, results, 0, a);

	nrm_log_info("sending actuation\n");
	nrm_client_actuate(client, *a, value);
	nrm_actuator_destroy(a);
	nrm_vector_clear(results);
	nrm_vector_destroy(&results);
	return 0;
}

static void nrmc_string_getenv(char *envname, nrm_vector_t *vec)
{
	nrm_string_t ret;
	char *envstr = getenv(envname);
	if (envstr == NULL)
		return;

	ret = nrm_string_fromchar(envstr);
	nrm_vector_push_back(vec, &ret);
}

static void nrmc_string_setenv(char *envname, nrm_vector_t *vec)
{
	nrm_string_t val = nrm_string_vjoin(':', vec);
	nrm_log_info("%s=%s\n", envname, val);
	setenv(envname, val, 1);
	nrm_string_decref(val);
}

int cmd_run(int argc, char **argv)
{

	int err;
	nrm_vector_t *preloads, *ompt;
	static struct option cmd_run_long_options[] = {
	        {"preload", required_argument, NULL, 'd'},
	        {"ompt", required_argument, NULL, 't'},
	        {0, 0, 0, 0},
	};

	static const char *cmd_run_short_options = ":d:t:";

	nrm_vector_create(&preloads, sizeof(nrm_string_t));
	nrmc_string_getenv("LD_PRELOAD", preloads);
	nrm_vector_create(&ompt, sizeof(nrm_string_t));
	nrmc_string_getenv("OMP_TOOL_LIBRARIES", ompt);
	nrm_string_t path;

	optind = 1;

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
		case 'd':
			path = nrm_string_fromchar(optarg);
			nrm_vector_push_back(preloads, &path);
			nrm_log_debug("asked to preload: %s\n", path);
			break;
		case 't':
			path = nrm_string_fromchar(optarg);
			nrm_vector_push_back(ompt, &path);
			nrm_log_debug(
			        "asked to add to OMP_TOOL_LIBRARIES: %s\n",
			        path);
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

	nrmc_string_setenv("LD_PRELOAD", preloads);
	nrm_vector_foreach(preloads, iter)
	{
		nrm_string_t *s = nrm_vector_iterator_get(iter);
		nrm_string_decref(*s);
	}
	nrmc_string_setenv("OMP_TOOL_LIBRARIES", ompt);
	nrm_vector_foreach(ompt, iter)
	{
		nrm_string_t *s = nrm_vector_iterator_get(iter);
		nrm_string_decref(*s);
	}
	nrm_log_info("exec: argc: %u, argv[0]: %s\n", argc, argv[0]);
	err = execvp(argv[0], &argv[0]);
	return err;
}

int cmd_add_scope(int argc, char **argv)
{

	/* no matter the arguments, only one extra parameter */
	if (argc != 3)
		return EXIT_FAILURE;

	int err;
	nrm_scope_t *scope;
	json_t *param;
	json_error_t json_error;
	param = json_loads(argv[2], 0, &json_error);
	scope = nrm_scope_create(argv[1]);
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

	if (argc < 2)
		return EXIT_FAILURE;

	int err;
	nrm_vector_t *results;
	char *name = NULL;
	name = argv[1];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_ACTUATOR, name,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	json_t *array = json_array();
	nrm_vector_foreach(results, iterator)
	{
		nrm_actuator_t **a = nrm_vector_iterator_get(iterator);
		json_t *json = nrm_actuator_to_json(*a);
		json_array_append_new(array, json);
		nrm_actuator_destroy(a);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	nrm_vector_clear(results);
	nrm_vector_destroy(&results);
	json_decref(array);
	return 0;
}

int cmd_find_scope(int argc, char **argv)
{

	if (argc < 2)
		return EXIT_FAILURE;

	int err;
	nrm_vector_t *results;
	char *name = NULL;
	name = argv[1];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_SCOPE, name,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	json_t *array = json_array();
	nrm_vector_foreach(results, iterator)
	{
		nrm_scope_t **s = nrm_vector_iterator_get(iterator);
		json_t *json = nrm_scope_to_json(*s);
		json_array_append_new(array, json);
		nrm_scope_destroy(*s);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	nrm_vector_clear(results);
	nrm_vector_destroy(&results);
	json_decref(array);
	return 0;
}

int cmd_find_sensor(int argc, char **argv)
{

	if (argc < 2)
		return EXIT_FAILURE;

	int err;
	nrm_vector_t *results;
	char *name = NULL;
	name = argv[1];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_SENSOR, name,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	json_t *array = json_array();
	nrm_vector_foreach(results, iterator)
	{
		nrm_sensor_t **s = nrm_vector_iterator_get(iterator);
		json_t *json = nrm_sensor_to_json(*s);
		json_array_append_new(array, json);
		nrm_sensor_destroy(s);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	nrm_vector_clear(results);
	nrm_vector_destroy(&results);
	json_decref(array);
	return 0;
}

int cmd_find_slice(int argc, char **argv)
{

	if (argc < 2)
		return EXIT_FAILURE;

	int err;
	nrm_vector_t *results;
	char *name = NULL;
	name = argv[1];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_SLICE, name,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	json_t *array = json_array();
	nrm_vector_foreach(results, iterator)
	{
		nrm_slice_t **s = nrm_vector_iterator_get(iterator);
		json_t *json = nrm_slice_to_json(*s);
		json_array_append_new(array, json);
		nrm_slice_destroy(s);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	nrm_vector_clear(results);
	nrm_vector_destroy(&results);
	json_decref(array);
	return 0;
}

int client_listen_callback(nrm_string_t sensor_uuid,
                           nrm_time_t time,
                           nrm_scope_t *scope,
                           double value)
{
	int64_t nstime = nrm_time_tons(&time);
	fprintf(stdout, "event: %" PRId64 " %s %s %f\n", nstime, sensor_uuid,
	        scope->uuid, value);
	return 0;
}

int cmd_listen(int argc, char **argv)
{
	/* optionally a topic */
	if (argc > 2)
		return EXIT_FAILURE;

	nrm_string_t topic;
	if (argc == 2)
		topic = nrm_string_fromchar(argv[1]);
	else
		topic = nrm_string_fromchar("");
	nrm_log_debug("listening to topic: %s\n", topic);

	nrm_client_set_event_listener(client, client_listen_callback);
	nrm_client_start_event_listener(client, topic);

	nrm_reactor_t *reactor;
	nrm_reactor_create(&reactor, NULL);

	/* idle loop */
	nrm_reactor_start(reactor);

	/* cleanup, only get there if signal or error */
	nrm_reactor_destroy(&reactor);
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

	json_t *array = json_array();
	nrm_vector_foreach(actuators, iterator)
	{
		nrm_actuator_t **r = nrm_vector_iterator_get(iterator);
		json_t *json = nrm_actuator_to_json(*r);
		json_array_append_new(array, json);
		nrm_actuator_destroy(r);
	}
	nrm_vector_destroy(&actuators);
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	json_decref(array);
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

	json_t *array = json_array();
	nrm_vector_foreach(scopes, iterator)
	{
		nrm_scope_t **s = nrm_vector_iterator_get(iterator);
		json_t *json = nrm_scope_to_json(*s);
		json_array_append_new(array, json);
		nrm_scope_destroy(*s);
	}
	nrm_vector_destroy(&scopes);
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	json_decref(array);
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

	json_t *array = json_array();
	nrm_vector_foreach(sensors, iterator)
	{
		nrm_sensor_t **s = nrm_vector_iterator_get(iterator);
		json_t *json = nrm_sensor_to_json(*s);
		json_array_append_new(array, json);
		nrm_sensor_destroy(s);
	}
	nrm_vector_destroy(&sensors);
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	json_decref(array);
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

	json_t *array = json_array();
	nrm_vector_foreach(slices, iterator)
	{
		nrm_slice_t **s = nrm_vector_iterator_get(iterator);
		json_t *json = nrm_slice_to_json(*s);
		json_array_append_new(array, json);
		nrm_slice_destroy(s);
	}
	nrm_vector_destroy(&slices);
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	json_decref(array);
	return 0;
}

int cmd_remove_scope(int argc, char **argv)
{
	int err;
	static int ask_all = 0;
	static struct option cmd_run_long_options[] = {
	        {"all", no_argument, &ask_all, 1},
	        {0, 0, 0, 0},
	};

	static const char *cmd_run_short_options = ":a";

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
	name = argv[0];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_SCOPE, name,
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
		nrm_vector_get_withtype(nrm_scope_t, results, i, s);
		json_t *json = nrm_scope_to_json(s);
		nrm_client_remove_scope(client, s);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_remove_sensor(int argc, char **argv)
{
	int err;
	static int ask_all = 0;
	static struct option cmd_run_long_options[] = {
	        {"all", no_argument, &ask_all, 1},
	        {0, 0, 0, 0},
	};

	static const char *cmd_run_short_options = ":a";

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
	name = argv[0];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_SENSOR, name,
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
		nrm_vector_get_withtype(nrm_sensor_t, results, i, s);
		json_t *json = nrm_sensor_to_json(s);
		nrm_client_remove_sensor(client, s);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_remove_slice(int argc, char **argv)
{
	int err;
	static int ask_all = 0;
	static struct option cmd_run_long_options[] = {
	        {"all", no_argument, &ask_all, 1},
	        {0, 0, 0, 0},
	};

	static const char *cmd_run_short_options = ":a";

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
	name = argv[1];
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_SLICE, name,
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
		nrm_vector_get_withtype(nrm_slice_t, results, i, s);
		json_t *json = nrm_slice_to_json(s);
		nrm_client_remove_slice(client, s);
		json_array_append_new(array, json);
	}
	json_dumpf(array, stdout, JSON_SORT_KEYS);
	return 0;
}

int cmd_send_event(int argc, char **argv)
{
	/* no options at this time */
	if (argc < 3)
		return EXIT_FAILURE;

	char *name = argv[1];
	char *scope_name = argv[2];

	/* find sensor */
	int err;
	nrm_vector_t *results;
	err = nrm_client_find(client, NRM_MSG_TARGET_TYPE_SENSOR, name,
	                      &results);
	if (err) {
		nrm_log_error("error during client request\n");
		return EXIT_FAILURE;
	}

	size_t len;
	nrm_vector_length(results, &len);

	assert(len == 1);
	nrm_sensor_t **s;
	nrm_sensor_t *sensor;
	nrm_vector_get_withtype(nrm_sensor_t *, results, 0, s);
	sensor = *s;

	nrm_log_info("sending event\n");
	nrm_scope_t *scope = nrm_scope_create(scope_name);
	nrm_scope_threadprivate(scope);
	nrm_time_t time;
	nrm_time_gettime(&time);
	nrm_client_send_event(client, time, sensor, scope, rand());

	nrm_vector_foreach(results, iterator)
	{
		nrm_vector_iterator_get(iterator);
		nrm_sensor_destroy(s);
	}
	nrm_vector_destroy(&results);
	nrm_scope_destroy(scope);
	return 0;
}

int cmd_exit(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	nrm_client_send_exit(client);
	return 0;
}

int cmd_tick(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	nrm_client_send_tick(client);
	return 0;
}

static struct client_cmd commands[] = {
        {"actuate", cmd_actuate},
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
        {"remove-scope", cmd_remove_scope},
        {"remove-slice", cmd_remove_slice},
        {"remove-sensor", cmd_remove_sensor},
        {"run", cmd_run},
        {"exit", cmd_exit},
        {"tick", cmd_tick},
        {0, 0},
};

void nrmc_print_help(nrm_tools_args_t *args)
{
	nrm_tools_print_help(args);

	fprintf(stdout, "\nAvailable Commands:\n");
	for (size_t i = 0; commands[i].name != NULL; i++)
		fprintf(stdout, "%s\n", commands[i].name);
}

int main(int argc, char *argv[])
{
	int err;
	nrm_tools_args_t args;
	nrm_log_init(stderr, "nrmc");
	nrm_init(NULL, NULL);

	args.progname = "nrmc";
	args.flags = 0;

	err = nrm_tools_parse_args(argc, argv, &args);
	if (err < 0) {
		nrm_log_error("errors during argument parsing\n");
		nrmc_print_help(&args);
		exit(EXIT_FAILURE);
	}

	/* remove the parsed part */
	argc -= err;
	argv = &(argv[err]);

	if (args.ask_help) {
		nrmc_print_help(&args);
		exit(EXIT_SUCCESS);
	}
	if (args.ask_version) {
		nrm_tools_print_version(&args);
		exit(EXIT_SUCCESS);
	}

	if (argc == 0) {
		nrmc_print_help(&args);
		exit(EXIT_FAILURE);
	}

	nrm_log_setlevel(args.log_level);
	nrm_log_debug("after command line parsing: argc: %u argv[0]: %s\n",
	              argc, argv[0]);

	if (!strcmp(argv[0], "connect")) {
		err = cmd_connect(&args, argc, argv);
		goto end;
	}

	nrm_log_info("creating client\n");
	err = nrm_client_create(&client, args.upstream_uri, args.pub_port,
	                        args.rpc_port);
	if (err) {
		nrm_log_error("error during client creation: %d\n", err);
		goto cleanup;
	}

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
cleanup:
	nrm_finalize();
	return err;
}
