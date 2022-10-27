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

// clang-format off
#include "nrm/utils/alloc.h"
#include "nrm/utils/bitmaps.h"
#include "nrm/utils/error.h"
#include "nrm/utils/ringbuffer.h"
#include "nrm/utils/strings.h"
#include "nrm/utils/scopes.h"
#include "nrm/utils/timers.h"
#include "nrm/utils/uuids.h"
#include "nrm/utils/vectors.h"
#include "nrm/utils/hashes.h"
#include "nrm/utils/version.h"
// clang-format on
//
/*******************************************************************************
 * Library Init/Fini
 ******************************************************************************/

/** initializes the library, in particular using environment variables to figure
 * out if the library should emit messages at all and if a ratelimit is in
 * place.
 *
 * @param[in,out] argc: a pointer to the number of arguments in *argv
 * @param[in,out] argv: an array of command line options.
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
 * Terminates the library. Do this before an instrumented program exits.
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

/**
 * Initializes NRM logging
 *
 * @param f: file descriptor
 * @param nm: logging source namespace
 * @return 0 if successful, an error code otherwise
 */
int nrm_log_init(FILE *f, const char *nm);

/**
 * Prints an NRM log message at a log level, labeled with source file and line
 * number.
 *
 * @param level: log level constant
 * @param file: source file label typically ``__FILE__``
 * @param line: source line number label. typically ``__LINE__``
 * @param format: printf formatted string
 */
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
 * Actuator: something capable of actions on the system
 ******************************************************************************/

struct nrm_actuator_s {
	nrm_string_t uuid;
	nrm_uuid_t *clientid;
	double value;
	nrm_vector_t *choices;
};

typedef struct nrm_actuator_s nrm_actuator_t;

nrm_actuator_t *nrm_actuator_create(const char *name);
int nrm_actuator_set_value(nrm_actuator_t *, double);
int nrm_actuator_set_choices(nrm_actuator_t *, size_t, double *);

void nrm_actuator_destroy(nrm_actuator_t **);
void nrm_actuator_fprintf(FILE *out, nrm_actuator_t *);

/*******************************************************************************
 * Extra scope API
 ******************************************************************************/

int nrm_scope_hwloc_scopes(nrm_hash_t **scopes);
nrm_scope_t *nrm_scope_create_hwloc_allowed(const char *name);

/*******************************************************************************
 * Slice: a resource arbitration unit
 ******************************************************************************/

struct nrm_slice_s {
	nrm_string_t uuid;
};

typedef struct nrm_slice_s nrm_slice_t;

/**
 * Creates a new NRM slice
 *
 * @param name: char pointer to a name describing the slice
 * @return: a new NRM slice structure
 */
nrm_slice_t *nrm_slice_create(const char *name);

/**
 * Removes an NRM slice. Do this for each slice before an instrumented program
 * exits.
 */
void nrm_slice_destroy(nrm_slice_t **);

/**
 * Prints an NRM slice's contents to the specified output file.
 */
void nrm_slice_fprintf(FILE *out, nrm_slice_t *);

/*******************************************************************************
 * Sensor: an emitter of events
 ******************************************************************************/

struct nrm_sensor_s {
	nrm_string_t uuid;
};

typedef struct nrm_sensor_s nrm_sensor_t;
/**
 * Creates a new NRM sensor
 *
 * @param name: char pointer to a name describing the sensor
 * @return: a new NRM sensor structure
 */
nrm_sensor_t *nrm_sensor_create(const char *name);

nrm_string_t nrm_sensor_uuid(nrm_sensor_t *sensor);

/**
 * Removes an NRM sensor. Do this for each sensor before an instrumented program
 * exits.
 */
void nrm_sensor_destroy(nrm_sensor_t **);

/*******************************************************************************
 * State: the full state of a controller
 ******************************************************************************/

struct nrm_state_s {
	nrm_hash_t *actuators;
	nrm_hash_t *slices;
	nrm_hash_t *sensors;
	nrm_hash_t *scopes;
};

typedef struct nrm_state_s nrm_state_t;

nrm_state_t *nrm_state_create(void);

void nrm_state_destroy(nrm_state_t **);

/*******************************************************************************
 * EventBase: a timeseries in-memory database
 ******************************************************************************/

struct nrm_eventbase_s;

typedef struct nrm_eventbase_s nrm_eventbase_t;

nrm_eventbase_t *nrm_eventbase_create(size_t maxperiods);

size_t nrm_eventbase_get_maxperiods(nrm_eventbase_t *);

int nrm_eventbase_push_event(
        nrm_eventbase_t *, nrm_string_t, nrm_scope_t *, nrm_time_t, double);

int nrm_eventbase_tick(nrm_eventbase_t *, nrm_time_t);

int nrm_eventbase_last_value(nrm_eventbase_t *,
                             nrm_string_t,
                             nrm_string_t,
                             double *);

void nrm_eventbase_destroy(nrm_eventbase_t **);

/*******************************************************************************
 * NRM Client object
 * Used by any program intending to communicate with a NRM daemon. Initiate most
 * RPCs, retrieve information about the state of the daemon, can register new
 * elements, send events, listen to state changes.
 ******************************************************************************/

typedef struct nrm_client_s nrm_client_t;

typedef int(nrm_client_event_listener_fn)(nrm_string_t sensor_uuid,
                                          nrm_time_t time,
                                          nrm_scope_t *scope,
                                          double value);
typedef int(nrm_client_actuate_listener_fn)(nrm_uuid_t *uuid, double value);

/**
 * Creates a new NRM Client.
 *
 * @param client: pointer to a variable that contains the created client handle
 * @param uri: address for connecting to `nrmd`
 * @param pub_port:
 * @param rpc_port:
 * @return 0 if successful, an error code otherwise
 *
 */
int nrm_client_create(nrm_client_t **client,
                      const char *uri,
                      int pub_port,
                      int rpc_port);

int nrm_client_actuate(const nrm_client_t *client,
                       nrm_actuator_t *actuator,
                       double value);

int nrm_client_add_actuator(const nrm_client_t *client,
                            nrm_actuator_t *actuator);

/**
 * Adds an NRM scope to an NRM client.
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_add_scope(const nrm_client_t *client, nrm_scope_t *scope);

/**
 * Adds an NRM sensor to an NRM client.
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_add_sensor(const nrm_client_t *client, nrm_sensor_t *sensor);

/**
 * Adds an NRM slice to an NRM client.
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_add_slice(const nrm_client_t *client, nrm_slice_t *slice);

/**
 * Find matching NRM objects within a client
 * @param client: NRM client
 * @param type: An NRM scope, sensor, or slice type
 * @param uuid: name/uuid of the object to find
 * @param results: NRM vector for containing results
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_find(const nrm_client_t *client,
                    int type,
                    const char *uuid,
                    nrm_vector_t **results);

int nrm_client_list_actuators(const nrm_client_t *client,
                              nrm_vector_t **actuators);

/**
 * Lists an NRM client's registered scopes into a vector
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_list_scopes(const nrm_client_t *client, nrm_vector_t **scopes);

/**
 * Lists an NRM client's registered sensors into a vector
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_list_sensors(const nrm_client_t *client, nrm_vector_t **sensors);

/**
 * Lists an NRM client's registered slices into a vector
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_list_slices(const nrm_client_t *client, nrm_vector_t **slices);

/**
 * Removes an NRM actuator from a daemon
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_remove_actuator(const nrm_client_t *client,
                               nrm_actuator_t *actuator);

/**
 * Removes an NRM slice from a daemon
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_remove_slice(const nrm_client_t *client, nrm_slice_t *slice);

/**
 * Removes an NRM sensor from a daemon
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_remove_sensor(const nrm_client_t *client, nrm_sensor_t *sensor);

/**
 * Removes an NRM scope from a daemon
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_remove_scope(const nrm_client_t *client, nrm_scope_t *scope);

/**
 * Sends a measurement to the NRM daemon
 *
 * @param client: NRM client object
 * @param time: a time value retrieved via `nrm_time_gettime(&time)`
 * @param sensor: NRM sensor object
 * @param scope: NRM scope object
 * @param value: a measurement to send to the NRM daemon`
 * @return 0 if successful, an error code otherwise
 *
 */
int nrm_client_send_event(const nrm_client_t *client,
                          nrm_time_t time,
                          nrm_sensor_t *sensor,
                          nrm_scope_t *scope,
                          double value);

/**
 * Asks the daemon to exit
 *
 * @param client: NRM client object
 * @return 0 if successful, an error code otherwise
 *
 */
int nrm_client_send_exit(const nrm_client_t *client);

/**
 * Set a callback function for client events
 * @param client: NRM client object
 * @param fn: function reference
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_set_event_listener(nrm_client_t *client,
                                  nrm_client_event_listener_fn fn);

/**
 * Start a callback function for client events
 * @param client: NRM client object
 * @param topic: NRM string label
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_start_event_listener(const nrm_client_t *client,
                                    nrm_string_t topic);

int nrm_client_set_actuate_listener(nrm_client_t *client,
                                    nrm_client_actuate_listener_fn fn);
int nrm_client_start_actuate_listener(const nrm_client_t *client);

/**
 * Removes an NRM client. Do this for each client before an instrumented program
 * exits.
 */
void nrm_client_destroy(nrm_client_t **client);

/*******************************************************************************
 * NRM Server object
 * Used by any program that wants to act as a control loop: it can receive
 * requests from clients and send actions back.
 ******************************************************************************/

typedef struct nrm_server_s nrm_server_t;

typedef int(nrm_server_callback_fn)(void *arg);

/**
 * Creates a new NRM server.
 *
 * @param server: pointer to a variable that will contain the server handle
 * @param uri: address for listening to clients
 * @param pub_port: port for publishing server events
 * @param rpc_port: port for listening to requests
 * @return 0 if successful, an error code otherwise
 *
 */
int nrm_server_create(nrm_server_t **server,
                      const char *uri,
                      int pub_port,
                      int rpc_port);


/**
 * Destroys an NRM server. Closes connections.
 */
void nrm_server_destroy(nrm_client_t **client);

#ifdef __cplusplus
}
#endif

#endif
