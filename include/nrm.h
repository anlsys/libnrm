/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

/**
 * Main NRM header, contains most of the nrm public API
 **/

#ifndef NRM_H
#define NRM_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "nrm/utils/alloc.h"
#include "nrm/utils/bitmaps.h"
#include "nrm/utils/error.h"
#include "nrm/utils/ringbuffer.h"
#include "nrm/utils/scopes.h"
#include "nrm/utils/strings.h"
#include "nrm/utils/timers.h"
#include "nrm/utils/uuids.h"
#include "nrm/utils/vectors.h"
#include "nrm/utils/version.h"

/*******************************************************************************
 * Library Init/Fini
 ******************************************************************************/

/** initializes the library, in particular using environment variables to figure
 * out if the library should emit messages at all and if a ratelimit is in
 * place.
 *
 * @param[inout] argc: a pointer to the number of arguments in *argv
 * @param[inout] argv: an array of command line options.
 *
 * If the library detects command line options it understands, it will consume
 * those arguments and modify the two variables accordingly. Other arguments
 * will be ignored. It is safe to give a program's main arguments to this
 * library.
 *
 * @return 0 if successful; an error code otherwise.
 **/
int nrm_init(int *argc, char **argv[]);

/**
 * Terminates the library.
 * @return 0 if successful; an error code otherwise.
 **/
int nrm_finalize(void);

/*******************************************************************************
 * Logging
 ******************************************************************************/

#define NRM_LOG_QUIET 0
#define NRM_LOG_ERROR 1
#define NRM_LOG_WARNING 2
#define NRM_LOG_NORMAL 3
#define NRM_LOG_INFO 4
#define NRM_LOG_DEBUG 5

int nrm_log_init(FILE *f, const char *nm);

void nrm_log_printf(int level,
                    const char *file,
                    unsigned int line,
                    const char *format,
                    ...);

int nrm_log_setlevel(int level);

#define nrm_log_error(...)                                                     \
	nrm_log_printf(NRM_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define nrm_log_warning(...)                                                   \
	nrm_log_printf(NRM_LOG_WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define nrm_log_normal(...)                                                    \
	nrm_log_printf(NRM_LOG_NORMAL, __FILE__, __LINE__, __VA_ARGS__)
#define nrm_log_info(...)                                                      \
	nrm_log_printf(NRM_LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define nrm_log_debug(...)                                                     \
	nrm_log_printf(NRM_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)

/*******************************************************************************
 * NRM Messages
 * Messages being transmitted between nrm components
 ******************************************************************************/

typedef struct _Nrm__Message nrm_msg_t;

void nrm_msg_destroy(nrm_msg_t **msg);

/*******************************************************************************
 * Slice: a resource arbitration unit
 ******************************************************************************/

struct nrm_slice_s {
	nrm_string_t name;
	nrm_uuid_t *uuid;
};

typedef struct nrm_slice_s nrm_slice_t;

nrm_slice_t *nrm_slice_create(char *name);

void nrm_slice_destroy(nrm_slice_t **);
void nrm_slice_fprintf(FILE *out, nrm_slice_t *);

/*******************************************************************************
 * Sensor: an emitter of events
 ******************************************************************************/

struct nrm_sensor_s {
	nrm_string_t name;
	nrm_uuid_t *uuid;
};

typedef struct nrm_sensor_s nrm_sensor_t;

nrm_sensor_t *nrm_sensor_create(char *name);

void nrm_sensor_destroy(nrm_sensor_t **);

/*******************************************************************************
 * State: the full state of a controller
 ******************************************************************************/

struct nrm_state_s {
	nrm_vector_t *slices;
	nrm_vector_t *sensors;
	nrm_vector_t *scopes;
};

typedef struct nrm_state_s nrm_state_t;

nrm_state_t *nrm_state_create(void);

/*******************************************************************************
 * EventBase: a timeseries in-memory database
 ******************************************************************************/

struct nrm_eventbase_s;

typedef struct nrm_eventbase_s nrm_eventbase_t;

nrm_eventbase_t *nrm_eventbase_create(size_t maxperiods);

int nrm_eventbase_push_event(
        nrm_eventbase_t *, nrm_uuid_t *, nrm_scope_t *, nrm_time_t, double);

void nrm_eventbase_destroy(nrm_eventbase_t **);

/*******************************************************************************
 * NRM Client object
 * Used by any program intending to communicate with a NRM daemon. Initiate most
 * RPCs, retrieve information about the state of the daemon, can register new
 * elements, send events, listen to state changes.
 ******************************************************************************/

typedef struct nrm_client_s nrm_client_t;

typedef int(nrm_client_event_listener_fn)(nrm_uuid_t *uuid,
                                          nrm_time_t time,
                                          nrm_scope_t *scope,
                                          double value);

int nrm_client_create(nrm_client_t **client,
                      const char *uri,
                      int pub_port,
                      int rpc_port);

int nrm_client_add_scope(const nrm_client_t *client, nrm_scope_t *scope);
int nrm_client_add_sensor(const nrm_client_t *client, nrm_sensor_t *sensor);
int nrm_client_add_slice(const nrm_client_t *client, nrm_slice_t *slice);
int nrm_client_find(const nrm_client_t *client,
                    int type,
                    char *name,
                    nrm_uuid_t *uuid,
                    nrm_vector_t **results);
int nrm_client_list_scopes(const nrm_client_t *client, nrm_vector_t **scopes);
int nrm_client_list_sensors(const nrm_client_t *client, nrm_vector_t **sensors);
int nrm_client_list_slices(const nrm_client_t *client, nrm_vector_t **slices);
int nrm_client_remove(const nrm_client_t *client, int type, nrm_uuid_t *uuid);
int nrm_client_send_event(const nrm_client_t *client,
                          nrm_time_t time,
                          nrm_sensor_t *sensor,
                          nrm_scope_t *scope,
                          double value);
int nrm_client_set_event_listener(nrm_client_t *client,
                                  nrm_client_event_listener_fn fn);
int nrm_client_start_event_listener(const nrm_client_t *client,
                                    nrm_string_t topic);

void nrm_client_destroy(nrm_client_t **client);

/*******************************************************************************
 * NRM Role API
 * A "role" is a set of features of NRM that a client of this library is using.
 * A typical role is a sensor, or an actuator, or something monitoring sensor
 * messages and so on.
 * Behind each role is an event loop and a helper thread, related to its actions
 * in the communication infrastructure of the NRM.
 ******************************************************************************/

typedef struct nrm_role_s nrm_role_t;
typedef int(nrm_role_sub_callback_fn)(nrm_msg_t *msg, void *arg);

nrm_role_t *nrm_role_monitor_create_fromenv();

nrm_role_t *nrm_role_sensor_create_fromenv(const char *sensor_name);

nrm_role_t *nrm_role_client_create_fromparams(const char *, int, int);

int nrm_role_send(const nrm_role_t *role, nrm_msg_t *msg, nrm_uuid_t *to);
nrm_msg_t *nrm_role_recv(const nrm_role_t *role, nrm_uuid_t **from);
int nrm_role_pub(const nrm_role_t *role, nrm_string_t topic, nrm_msg_t *msg);
int nrm_role_register_sub_cb(const nrm_role_t *role,
                             nrm_role_sub_callback_fn *fn,
                             void *arg);
int nrm_role_sub(const nrm_role_t *role, nrm_string_t topic);

void nrm_role_destroy(nrm_role_t **);

#ifdef __cplusplus
}
#endif

#endif
