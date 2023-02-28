/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#ifndef LIBNRM_INTERNAL_ROLES_H
#define LIBNRM_INTERNAL_ROLES_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "internal/messages.h"
#include "internal/nrmi.h"

/*******************************************************************************
 * Global definitions
 ******************************************************************************/

struct nrm_role_data;

struct nrm_role_ops {
	int (*send)(const struct nrm_role_data *data,
	            nrm_msg_t *msg,
	            nrm_uuid_t *to);
	nrm_msg_t *(*recv)(const struct nrm_role_data *data, nrm_uuid_t **from);
	int (*pub)(const struct nrm_role_data *data,
	           nrm_string_t topic,
	           nrm_msg_t *msg);
	int (*register_sub_cb)(const struct nrm_role_data *data,
	                       nrm_role_sub_callback_fn *fn,
	                       void *arg);
	int (*sub)(const struct nrm_role_data *data, nrm_string_t topic);
	int (*register_cmd_cb)(const struct nrm_role_data *data,
	                       nrm_role_cmd_callback_fn *fn,
	                       void *arg);
	void (*destroy)(nrm_role_t **role);
};

struct nrm_role_s {
	struct nrm_role_ops *ops;
	struct nrm_role_data *data;
};

int nrm_role_monitor_register_recvcallback(nrm_role_t *role,
                                           zloop_t *loop,
                                           zloop_reader_fn *fn,
                                           void *arg);

int nrm_role_controller_register_recvcallback(nrm_role_t *role,
                                              zloop_t *loop,
                                              zloop_reader_fn *fn,
                                              void *arg);
/*******************************************************************************
 * Monitor:
 * monitors sensor data, recv a message each time a sensor sends something
 ******************************************************************************/

nrm_role_t *nrm_role_monitor_create_fromenv();

extern struct nrm_role_ops nrm_role_monitor_ops;

/*******************************************************************************
 * Sensor:
 * gather information about something, sends data out
 ******************************************************************************/

nrm_role_t *nrm_role_sensor_create_fromenv();

extern struct nrm_role_ops nrm_role_sensor_ops;

/*******************************************************************************
 * Controller:
 * control the system, responds to client requests, publishes information on the
 * system
 ******************************************************************************/

nrm_role_t *nrm_role_controller_create_fromparams(const char *, int, int);

extern struct nrm_role_ops nrm_role_controller_ops;

/*******************************************************************************
 * Client:
 * client of the controller, send requests to it.
 ******************************************************************************/

nrm_role_t *nrm_role_client_create_fromparams(const char *, int, int);

extern struct nrm_role_ops nrm_role_client_ops;

#ifdef __cplusplus
}
#endif

#endif
