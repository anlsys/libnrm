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
	nrm_eventbase_t *events;
	nrm_control_t *control;
	nrm_sensor_t *mysensor;
	nrm_scope_t *myscope;
	nrm_string_t mytopic;
};

int signo;
struct nrm_daemon_s my_daemon;

nrm_msg_t *nrmd_daemon_remove_actuator(nrm_msg_remove_t *msg)
{
	nrm_string_t uuid = nrm_string_fromchar(msg->uuid);
	nrm_actuator_t *actuator = NULL;
	nrm_hash_remove(&my_daemon.state->actuators, uuid, (void *)&actuator);
	nrm_actuator_destroy(&actuator);
	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_ACK);
	return ret;
}

nrm_msg_t *nrmd_daemon_remove_scope(nrm_msg_remove_t *msg)
{
	nrm_string_t uuid = nrm_string_fromchar(msg->uuid);
	nrm_scope_t *scope = NULL;
	nrm_hash_remove(&my_daemon.state->scopes, uuid, (void *)&scope);
	nrm_scope_destroy(scope);
	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_ACK);
	return ret;
}

nrm_msg_t *nrmd_daemon_remove_sensor(nrm_msg_remove_t *msg)
{
	nrm_string_t uuid = nrm_string_fromchar(msg->uuid);
	nrm_sensor_t *sensor = NULL;
	nrm_hash_remove(&my_daemon.state->sensors, uuid, (void *)&sensor);
	nrm_sensor_destroy(&sensor);
	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_ACK);
	return ret;
}

nrm_msg_t *nrmd_daemon_remove_slice(nrm_msg_remove_t *msg)
{
	nrm_string_t uuid = nrm_string_fromchar(msg->uuid);
	nrm_slice_t *slice = NULL;
	nrm_hash_remove(&my_daemon.state->slices, uuid, (void *)&slice);
	nrm_slice_destroy(&slice);
	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_ACK);
	return ret;
}

nrm_msg_t *nrmd_daemon_build_list_actuators()
{
	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_LIST);

	nrm_vector_t *actuators_vector = NULL;
	nrm_vector_create(&actuators_vector, sizeof(nrm_actuator_t));

	nrm_hash_foreach(my_daemon.state->actuators, iter)
	{
		nrm_actuator_t *actuator = nrm_hash_iterator_get(iter);
		nrm_vector_push_back(actuators_vector, actuator);
	}

	nrm_msg_set_list_actuators(ret, actuators_vector);
	nrm_vector_destroy(&actuators_vector);
	return ret;
}

nrm_msg_t *nrmd_daemon_build_list_scopes()
{
	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_LIST);

	nrm_vector_t *scopes_vector = NULL;
	nrm_vector_create(&scopes_vector, sizeof(nrm_scope_t));

	nrm_hash_foreach(my_daemon.state->scopes, iter)
	{
		nrm_scope_t *scope = nrm_hash_iterator_get(iter);
		nrm_vector_push_back(scopes_vector, scope);
	}

	nrm_msg_set_list_scopes(ret, scopes_vector);
	nrm_vector_destroy(&scopes_vector);
	return ret;
}

nrm_msg_t *nrmd_daemon_build_list_sensors()
{
	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_LIST);

	nrm_vector_t *sensors_vector = NULL;
	nrm_vector_create(&sensors_vector, sizeof(nrm_sensor_t));

	nrm_hash_foreach(my_daemon.state->sensors, iter)
	{
		nrm_sensor_t *sensor = nrm_hash_iterator_get(iter);
		nrm_vector_push_back(sensors_vector, sensor);
	}

	nrm_msg_set_list_sensors(ret, sensors_vector);
	nrm_vector_destroy(&sensors_vector);
	return ret;
}

nrm_msg_t *nrmd_daemon_build_list_slices()
{
	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_LIST);

	nrm_vector_t *slices_vector = NULL;
	nrm_vector_create(&slices_vector, sizeof(nrm_slice_t));

	nrm_hash_foreach(my_daemon.state->slices, iter)
	{
		nrm_slice_t *slice = nrm_hash_iterator_get(iter);
		nrm_vector_push_back(slices_vector, slice);
	}

	nrm_msg_set_list_slices(ret, slices_vector);
	nrm_vector_destroy(&slices_vector);
	return ret;
}

nrm_msg_t *nrmd_daemon_add_actuator(nrm_uuid_t *clientid,
                                    nrm_msg_actuator_t *actuator)
{
	nrm_actuator_t *newactuator = nrm_actuator_create_frommsg(actuator);
	newactuator->clientid =
	        nrm_uuid_create_fromchar(nrm_uuid_to_char(clientid));

	nrm_hash_add(&my_daemon.state->actuators, newactuator->uuid,
	             newactuator);

	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_ADD);
	nrm_msg_set_add_actuator(ret, newactuator);
	return ret;
}

nrm_msg_t *nrmd_daemon_add_scope(nrm_msg_scope_t *scope)
{
	nrm_scope_t *newscope = nrm_scope_create_frommsg(scope);

	nrm_hash_add(&my_daemon.state->scopes, newscope->uuid, newscope);

	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_ADD);
	nrm_msg_set_add_scope(ret, newscope);
	return ret;
}

nrm_msg_t *nrmd_daemon_add_sensor(const char *name)
{
	nrm_sensor_t *newsensor = nrm_sensor_create(name);

	nrm_hash_add(&my_daemon.state->sensors, newsensor->uuid, newsensor);

	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_ADD);
	nrm_msg_set_add_sensor(ret, newsensor);
	return ret;
}

nrm_msg_t *nrmd_daemon_add_slice(const char *name)
{
	nrm_slice_t *newslice = nrm_slice_create(name);

	nrm_hash_add(&my_daemon.state->slices, newslice->uuid, newslice);

	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_ADD);
	nrm_msg_set_add_slice(ret, newslice);
	return ret;
}

nrm_msg_t *nrmd_handle_add_request(nrm_uuid_t *clientid, nrm_msg_add_t *msg)
{
	nrm_msg_t *ret = NULL;
	switch (msg->type) {
	case NRM_MSG_TARGET_TYPE_ACTUATOR:
		nrm_log_info("adding an actuator\n");
		ret = nrmd_daemon_add_actuator(clientid, msg->actuator);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SLICE:
		nrm_log_info("adding a slice\n");
		ret = nrmd_daemon_add_slice(msg->slice->uuid);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		nrm_log_info("adding a sensor\n");
		ret = nrmd_daemon_add_sensor(msg->sensor->uuid);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SCOPE:
		nrm_log_info("adding a scope\n");
		ret = nrmd_daemon_add_scope(msg->scope);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	default:
		nrm_log_error("wrong add request type %u\n", msg->type);
		break;
	}
	return ret;
}

nrm_msg_t *nrmd_handle_list_request(nrm_msg_list_t *msg)
{
	nrm_msg_t *ret = NULL;
	switch (msg->type) {
	case NRM_MSG_TARGET_TYPE_ACTUATOR:
		nrm_log_info("building list of actuators\n");
		ret = nrmd_daemon_build_list_actuators();
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SLICE:
		nrm_log_info("building list of slices\n");
		ret = nrmd_daemon_build_list_slices();
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		nrm_log_info("building list of sensors\n");
		ret = nrmd_daemon_build_list_sensors();
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SCOPE:
		nrm_log_info("building list of scopes\n");
		ret = nrmd_daemon_build_list_scopes();
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	default:
		nrm_log_error("wrong list request type %u\n", msg->type);
		break;
	}
	return ret;
}

nrm_msg_t *nrmd_handle_remove_request(nrm_msg_remove_t *msg)
{
	nrm_msg_t *ret = NULL;
	switch (msg->type) {
	case NRM_MSG_TARGET_TYPE_ACTUATOR:
		nrm_log_info("removing an actuator\n");
		ret = nrmd_daemon_remove_actuator(msg);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SLICE:
		nrm_log_info("removing a slice\n");
		ret = nrmd_daemon_remove_slice(msg);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		nrm_log_info("removing a sensor\n");
		ret = nrmd_daemon_remove_sensor(msg);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SCOPE:
		nrm_log_info("removing a scope\n");
		ret = nrmd_daemon_remove_scope(msg);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	default:
		nrm_log_error("wrong list request type %u\n", msg->type);
		break;
	}
	return ret;
}

int nrmd_handle_event_request(nrm_msg_event_t *msg)
{
	nrm_string_t uuid = nrm_string_fromchar(msg->uuid);
	nrm_scope_t *scope = nrm_scope_create_frommsg(msg->scope);
	nrm_time_t time = nrm_time_fromns(msg->time);
	nrm_eventbase_push_event(my_daemon.events, uuid, scope, time,
	                         msg->value);
	return 0;
}

nrm_msg_t *nrmd_handle_actuate_request(nrm_role_t *role, nrm_msg_actuate_t *msg)
{
	nrm_string_t uuid = nrm_string_fromchar(msg->uuid);
	nrm_actuator_t *a = NULL;
	nrm_hash_find(my_daemon.state->actuators, uuid, (void *)&a);
	if (a != NULL) {
		/* found the actuator */
		nrm_log_debug("actuating %s: %f\n", a->uuid, msg->value);
		nrm_msg_t *action = nrm_msg_create();
		nrm_msg_fill(action, NRM_MSG_TYPE_ACTUATE);
		nrm_msg_set_actuate(action, a->uuid, msg->value);
		nrm_role_send(role, action, a->clientid);
	}

	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_ACK);
	return ret;
}

nrm_msg_t *nrmd_handle_exit_request(nrm_role_t *role)
{
	(void)role;
	/* just ack the message and exit the daemon */
	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_ACK);
	return ret;
}

int nrmd_shim_controller_read_callback(zloop_t *loop,
                                       zsock_t *socket,
                                       void *arg)
{
	(void)loop;
	(void)socket;
	int err = 0;
	nrm_log_info("entering callback\n");
	nrm_role_t *self = (nrm_role_t *)arg;
	nrm_msg_t *msg, *ret;
	nrm_uuid_t *uuid;
	nrm_log_debug("receiving\n");
	msg = nrm_role_recv(self, &uuid);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	switch (msg->type) {
	case NRM_MSG_TYPE_ACTUATE:
		ret = nrmd_handle_actuate_request(self, msg->actuate);
		nrm_role_send(self, ret, uuid);
		break;
	case NRM_MSG_TYPE_LIST:
		ret = nrmd_handle_list_request(msg->list);
		nrm_role_send(self, ret, uuid);
		break;
	case NRM_MSG_TYPE_ADD:
		ret = nrmd_handle_add_request(uuid, msg->add);
		nrm_role_send(self, ret, uuid);
		break;
	case NRM_MSG_TYPE_EVENT:
		nrmd_handle_event_request(msg->event);
		break;
	case NRM_MSG_TYPE_REMOVE:
		ret = nrmd_handle_remove_request(msg->remove);
		nrm_role_send(self, ret, uuid);
		break;
	case NRM_MSG_TYPE_EXIT:
		ret = nrmd_handle_exit_request(self);
		nrm_role_send(self, ret, uuid);
		/* that will trigger exit */
		err = -1;
		break;
	default:
		nrm_log_error("message type not handled\n");
		break;
	}
	nrm_msg_destroy_received(&msg);
	return err;
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

int nrmd_actuate(nrm_role_t *role, nrm_string_t uuid, double value)
{
	nrm_actuator_t *a = NULL;
	nrm_hash_find(my_daemon.state->actuators, uuid, (void *)&a);
	if (a != NULL) {
		/* found the actuator */
		nrm_log_debug("actuating %s: %f\n", a->uuid, value);
		nrm_msg_t *action = nrm_msg_create();
		nrm_msg_fill(action, NRM_MSG_TYPE_ACTUATE);
		nrm_msg_set_actuate(action, a->uuid, value);
		nrm_role_send(role, action, a->clientid);
	}
	return 0;
}

int nrmd_timer_callback(zloop_t *loop, int timerid, void *arg)
{
	(void)loop;
	(void)timerid;
	nrm_log_info("global timer wakeup\n");
	nrm_role_t *self = (nrm_role_t *)arg;

	/* create a ticking event */
	nrm_time_t now;
	nrm_time_gettime(&now);

	nrm_log_debug("crafting message\n");
	nrm_msg_t *msg = nrm_msg_create();
	nrm_msg_fill(msg, NRM_MSG_TYPE_EVENT);
	nrm_msg_set_event(msg, now, my_daemon.mysensor->uuid, my_daemon.myscope,
	                  1.0);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_log_debug("sending event\n");
	nrm_role_pub(self, my_daemon.mytopic, msg);

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
		nrmd_actuate(self, out->actuator_uuid, out->value);
	}
	return 0;
}

/* most logic from https://gist.github.com/mhaberler/8426050 */
static int
nrmd_signal_callback(zloop_t *loop, zmq_pollitem_t *poller, void *arg)
{
	(void)loop;
	(void)arg;
	struct signalfd_siginfo fdsi;
	ssize_t s = read(poller->fd, &fdsi, sizeof(struct signalfd_siginfo));
	assert(s == sizeof(struct signalfd_siginfo));
	signo = fdsi.ssi_signo;
	nrm_log_debug("Caught SIGINT\n");
	return -1;
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
	fprintf(stdout, "nrmd-shim: version %s\n", nrm_version_string);
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
	/* networking */
	nrm_role_t *controller = nrm_role_controller_create_fromparams(
	        NRM_DEFAULT_UPSTREAM_URI, NRM_DEFAULT_UPSTREAM_PUB_PORT,
	        NRM_DEFAULT_UPSTREAM_RPC_PORT);
	assert(controller != NULL);

	zloop_t *loop = zloop_new();
	assert(loop != NULL);

	/* prepare set of signal handlers for zloop */
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGINT);
	// sigaddset(&sigmask, SIGKILL); // SIGKILL is ignored by signalfd

	sfd = signalfd(-1, &sigmask, 0);
	assert(sfd != -1);

	zmq_pollitem_t signal_poller = {0, sfd, ZMQ_POLLIN, 0};

	nrm_role_controller_register_recvcallback(
	        controller, loop, nrmd_shim_controller_read_callback,
	        controller);

	/* add a periodic wake up to generate metrics */
	zloop_timer(loop, 1000, 0, nrmd_timer_callback, controller);
	/* register signal handler callback */
	zloop_poller(loop, &signal_poller, nrmd_signal_callback, NULL);

	/* start loop, returns when context terminated,
	process interrupted, or event handler returns -1 */
	retval = zloop_start(loop);

	/* teardown zloop */
	zloop_destroy(&loop);
	assert(loop == NULL);
	nrm_log_debug("Loop destroyed\n");

	/* teardown NRM */
	nrm_string_decref(my_daemon.mytopic);
	nrm_sensor_destroy(&my_daemon.mysensor);
	nrm_eventbase_destroy(&my_daemon.events);
	nrm_state_destroy(&my_daemon.state);
	nrm_role_destroy(&controller);
	nrm_log_debug("NRM components destroyed\n");

	exit(EXIT_SUCCESS);
}
