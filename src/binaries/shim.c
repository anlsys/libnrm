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

#include "internal/nrmi.h"
#include "internal/messages.h"
#include "internal/roles.h"

#include <getopt.h>

struct nrm_daemon_s {
	nrm_state_t *state;
};

struct nrm_daemon_s my_daemon;

nrm_msg_t *nrmd_daemon_build_list_sensors()
{
	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_LIST);
	nrm_msg_set_list_sensors(ret, my_daemon.state->sensors);
	return ret;
}

nrm_msg_t *nrmd_daemon_build_list_slices()
{
	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_LIST);
	nrm_msg_set_list_slices(ret, my_daemon.state->slices);
	return ret;
}

nrm_msg_t *nrmd_daemon_add_sensor(const char *name)
{
	nrm_sensor_t *newsensor = nrm_sensor_create((char *)name);
	newsensor->uuid = nrm_uuid_create();
	nrm_vector_push_back(my_daemon.state->sensors, newsensor);

	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_ADD);
	nrm_msg_set_add_sensor(ret, (char *)newsensor->name, newsensor->uuid);
	return ret;
}

nrm_msg_t *nrmd_daemon_add_slice(const char *name)
{
	nrm_slice_t *newslice = nrm_slice_create((char *)name);
	newslice->uuid = nrm_uuid_create();
	nrm_vector_push_back(my_daemon.state->slices, newslice);

	nrm_msg_t *ret = nrm_msg_create();
	nrm_msg_fill(ret, NRM_MSG_TYPE_ADD);
	nrm_msg_set_add_slice(ret, (char *)newslice->name, newslice->uuid);
	return ret;
}

nrm_msg_t *nrmd_handle_add_request(nrm_msg_add_t *msg)
{
	nrm_msg_t *ret;
	switch(msg->type) {
	case NRM_MSG_TARGET_TYPE_SLICE:
		nrm_log_info("adding a slice\n");
		ret = nrmd_daemon_add_slice(msg->slice->name);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	case NRM_MSG_TARGET_TYPE_SENSOR:
		nrm_log_info("adding a sensor\n");
		ret = nrmd_daemon_add_sensor(msg->sensor->name);
		nrm_log_printmsg(NRM_LOG_DEBUG, ret);
		break;
	}
	return ret;
}

nrm_msg_t *nrmd_handle_list_request(nrm_msg_list_t *msg)
{
	nrm_msg_t *ret;
	switch(msg->type) {
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
	}
	return ret;
}

int nrmd_shim_monitor_read_callback(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)arg;
	(void)loop;
	nrm_msg_t *msg;
	int msg_type;

	msg = nrm_ctrlmsg_recv(socket, &msg_type, NULL);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	nrm_msg_destroy(&msg);
	return 0;
}

int nrmd_shim_controller_read_callback(zloop_t *loop, zsock_t *socket, void *arg)
{
	(void)loop;
	(void)socket;
	nrm_log_info("entering callback\n");
	nrm_role_t *self = (nrm_role_t *)arg;
	nrm_msg_t *msg, *ret;
	nrm_uuid_t *uuid;
	nrm_log_debug("receiving\n");
	msg = nrm_role_recv(self, &uuid);
	nrm_log_printmsg(NRM_LOG_DEBUG, msg);
	switch(msg->type) {
	case NRM_MSG_TYPE_LIST:
		ret = nrmd_handle_list_request(msg->list);
		nrm_role_send(self, ret, uuid);
		break;
	case NRM_MSG_TYPE_ADD:
		ret = nrmd_handle_add_request(msg->add);
		nrm_role_send(self, ret, uuid);
		break;
	default:
		nrm_log_error("message type not handled\n");
		break;
	}
	nrm_msg_destroy(&msg);
	return 0;
}

static int ask_help = 0;
static int ask_version = 0;

static struct option long_options[] = {
	{ "help", no_argument, &ask_help, 1 },
	{ "version", no_argument, &ask_version, 1},
	{ 0, 0, 0, 0},
};

static const char *short_options = ":hV";

static const char *help[] = {
	"Usage: nrmd-shim [options]\n\n",
	"Allowed options:\n",
	"--help, -h    : print this help message\n",
	"--version, -V : print program version\n",
	NULL
};

void print_help()
{
	for (int i = 0; help[i] != NULL; i++)
		fprintf(stdout, "%s", help[i]);
}

void print_version()
{
	fprintf(stdout,"nrmd-shim: version %s\n", nrm_version_string);
}

int main(int argc, char *argv[])
{
	int c;
	int option_index = 0;

	while(1) {
		c = getopt_long(argc, argv, short_options, long_options,
				&option_index);
		if (c == -1)
			break;
		switch(c) {
		case 0:
			break;
		case 'h':
			ask_help = 1;
			break;
		case 'V':
			ask_version = 1;
			break;
		case '?':
			fprintf(stderr,"nrmd-shim: invalid options: %s\n",
				argv[optind-1]);
			exit(EXIT_FAILURE);
		default:
			fprintf(stderr,"nrmd_shim: this should not happen\n");
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

	if(argc == 0) {
		print_help();
		exit(EXIT_FAILURE);
	}

	assert(!strcmp(argv[0], "fromenv"));
	/* create a monitor:
	 *  - this is a broker that listen to incoming messages
	 *  - we have our own loop to listen to it, no on a different thread
	 *  though, as we only have this to take care of
	 */

	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "nrmd");
	nrm_log_setlevel(NRM_LOG_DEBUG);


	/* init state */
	my_daemon.state = nrm_state_create();


	nrm_role_t *monitor = nrm_role_monitor_create_fromenv();
	assert(monitor != NULL);

	nrm_role_t *controller =
		nrm_role_controller_create_fromparams(NRM_DEFAULT_UPSTREAM_URI,
						      NRM_DEFAULT_UPSTREAM_PUB_PORT,
						      NRM_DEFAULT_UPSTREAM_RPC_PORT);
	assert(controller != NULL);

	zloop_t *loop = zloop_new();
	assert(loop != NULL);

	nrm_role_monitor_register_recvcallback(monitor, loop,
					       nrmd_shim_monitor_read_callback,
					       NULL);
	nrm_role_controller_register_recvcallback(controller, loop,
					nrmd_shim_controller_read_callback,
					controller);
	zloop_start(loop);	

	return 0;
}
