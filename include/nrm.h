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
#include <string.h>
#include <sys/signalfd.h>
#include <time.h>

// clang-format off
#include "nrm/utils/alloc.h"
#include "nrm/utils/bitmaps.h"
#include "nrm/utils/error.h"
#include "nrm/utils/parsers.h"
#include "nrm/utils/ringbuffer.h"
#include "nrm/utils/strings.h"
#include "nrm/utils/scopes.h"
#include "nrm/utils/timers.h"
#include "nrm/utils/uuids.h"
#include "nrm/utils/vectors.h"
#include "nrm/utils/hashes.h"
#include "nrm/utils/variables.h"
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
 * Set to one by the library if at least one nrm_log_init has
 * been called
 */
extern int nrm_log_initialized;

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
#define nrm_log_perror(...)                                                    \
	do {                                                                   \
		char *__nrm_errstr = strerror(errno);                          \
		nrm_log_printf(NRM_LOG_ERROR, __FILE__, __LINE__,              \
		               __VA_ARGS__);                                   \
		nrm_log_printf(NRM_LOG_ERROR, __FILE__, __LINE__,              \
		               "perror: %s\n", __nrm_errstr);                  \
	} while (0)

/*******************************************************************************
 * Actuator: something capable of actions on the system
 ******************************************************************************/

struct nrm_actuator_data_s;
struct nrm_actuator_ops_s;

struct nrm_actuator_s {
	struct nrm_actuator_data_s *data;
	struct nrm_actuator_ops_s *ops;
};

typedef struct nrm_actuator_s nrm_actuator_t;

nrm_actuator_t *nrm_actuator_discrete_create(const char *name);
int nrm_actuator_discrete_closest_choice(nrm_actuator_t *, double *);
int nrm_actuator_discrete_set_choices(nrm_actuator_t *, size_t, double *);
int nrm_actuator_discrete_list_choices(nrm_actuator_t *, nrm_vector_t **choices);

nrm_actuator_t *nrm_actuator_continuous_create(const char *name);
int nrm_actuator_continuous_set_limits(nrm_actuator_t *, double min, double max);

nrm_string_t nrm_actuator_uuid(nrm_actuator_t *);
nrm_uuid_t *nrm_actuator_clientid(nrm_actuator_t *);
double nrm_actuator_value(nrm_actuator_t *);

void nrm_actuator_fprintf(FILE *out, nrm_actuator_t *);
void nrm_actuator_destroy(nrm_actuator_t **);

int nrm_actuator_set_value(nrm_actuator_t *, double);
int nrm_actuator_validate_value(nrm_actuator_t *, double);

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

nrm_string_t nrm_slice_uuid(nrm_slice_t *);

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
	nrm_hash_t *scopes;
	nrm_hash_t *sensors;
	nrm_hash_t *slices;
};

typedef struct nrm_state_s nrm_state_t;

nrm_state_t *nrm_state_create(void);

int nrm_state_list_actuators(nrm_state_t *, nrm_vector_t *);
int nrm_state_list_scopes(nrm_state_t *, nrm_vector_t *);
int nrm_state_list_sensors(nrm_state_t *, nrm_vector_t *);
int nrm_state_list_slices(nrm_state_t *, nrm_vector_t *);

int nrm_state_add_actuator(nrm_state_t *, nrm_actuator_t *);
int nrm_state_add_scope(nrm_state_t *, nrm_scope_t *);
int nrm_state_add_sensor(nrm_state_t *, nrm_sensor_t *);
int nrm_state_add_slice(nrm_state_t *, nrm_slice_t *);

int nrm_state_remove_actuator(nrm_state_t *, const char *uuid);
int nrm_state_remove_scope(nrm_state_t *, const char *uuid);
int nrm_state_remove_sensor(nrm_state_t *, const char *uuid);
int nrm_state_remove_slice(nrm_state_t *, const char *uuid);

void nrm_state_destroy(nrm_state_t **);

/*******************************************************************************
 * Timeserie: a timeserie labeled by a scope and a sensor.
 ******************************************************************************/

struct nrm_event_s {
	nrm_time_t time;
	double value;
};
typedef struct nrm_event_s nrm_event_t;

struct nrm_timeserie_s;
typedef struct nrm_timeserie_s nrm_timeserie_t;

int nrm_timeserie_create(nrm_timeserie_t **, nrm_string_t, nrm_scope_t *t);
int nrm_timeserie_add_event(nrm_timeserie_t *, nrm_time_t, double);
int nrm_timeserie_add_events(nrm_timeserie_t *, nrm_vector_t *);
nrm_vector_t *nrm_timeserie_get_events(nrm_timeserie_t *);
void nrm_timeserie_destroy(nrm_timeserie_t **);

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

int nrm_eventbase_pull_timeserie(nrm_eventbase_t *,
                                 nrm_string_t,
                                 nrm_scope_t *,
                                 nrm_time_t since,
                                 nrm_timeserie_t **ts);

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

int nrm_client_actuate(nrm_client_t *client,
                       nrm_actuator_t *actuator,
                       double value);

int nrm_client_add_actuator(nrm_client_t *client, nrm_actuator_t *actuator);

/**
 * Adds an NRM scope to an NRM client.
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_add_scope(nrm_client_t *client, nrm_scope_t *scope);

/**
 * Adds an NRM sensor to an NRM client.
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_add_sensor(nrm_client_t *client, nrm_sensor_t *sensor);

/**
 * Adds an NRM slice to an NRM client.
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_add_slice(nrm_client_t *client, nrm_slice_t *slice);

/**
 * Find matching NRM objects within a client
 * @param client: NRM client
 * @param type: An NRM scope, sensor, or slice type
 * @param uuid: name/uuid of the object to find
 * @param results: NRM vector for containing results
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_find(nrm_client_t *client,
                    int type,
                    const char *uuid,
                    nrm_vector_t **results);

int nrm_client_list_actuators(nrm_client_t *client, nrm_vector_t **actuators);

/**
 * Lists an NRM client's registered scopes into a vector
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_list_scopes(nrm_client_t *client, nrm_vector_t **scopes);

/**
 * Lists an NRM client's registered sensors into a vector
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_list_sensors(nrm_client_t *client, nrm_vector_t **sensors);

/**
 * Lists an NRM client's registered slices into a vector
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_list_slices(nrm_client_t *client, nrm_vector_t **slices);

/**
 * Removes an NRM actuator from a daemon
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_remove_actuator(nrm_client_t *client, nrm_actuator_t *actuator);

/**
 * Removes an NRM slice from a daemon
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_remove_slice(nrm_client_t *client, nrm_slice_t *slice);

/**
 * Removes an NRM sensor from a daemon
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_remove_sensor(nrm_client_t *client, nrm_sensor_t *sensor);

/**
 * Removes an NRM scope from a daemon
 * @return 0 if successful, an error code otherwise
 */
int nrm_client_remove_scope(nrm_client_t *client, nrm_scope_t *scope);

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
int nrm_client_send_event(nrm_client_t *client,
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
int nrm_client_send_exit(nrm_client_t *client);

/**
 * Asks the daemon to tick
 *
 * @param client: NRM client object
 * @return 0 if successful, an error code otherwise
 *
 */
int nrm_client_send_tick(nrm_client_t *client);

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

/** User-level callbacks on server events */
struct nrm_server_user_callbacks_s {
	/* receiving a sensor event */
	int (*event)(nrm_server_t *,
	             nrm_string_t,
	             nrm_scope_t *,
	             nrm_time_t,
	             double value);
	/* receiving a request to actuate */
	int (*actuate)(nrm_server_t *, nrm_actuator_t *, double value);
	/* receiving a POSIX signal */
	int (*signal)(nrm_server_t *, int);
	/* timer trigger */
	int (*timer)(nrm_server_t *);
	/* receive a request to tick */
	int (*tick)(nrm_server_t *);
};

typedef struct nrm_server_user_callbacks_s nrm_server_user_callbacks_t;

/**
 * Creates a new NRM server.
 *
 * Uses a state to keep track of server objects that clients can add/remove
 * from the system.
 *
 * @param server: pointer to a variable that will contain the server handle
 * @param server: pointer to a valid state handle
 * @param uri: address for listening to clients
 * @param pub_port: port for publishing server events
 * @param rpc_port: port for listening to requests
 * @return 0 if successful, an error code otherwise
 *
 */
int nrm_server_create(nrm_server_t **server,
                      nrm_state_t *state,
                      const char *uri,
                      int pub_port,
                      int rpc_port);

int nrm_server_setcallbacks(nrm_server_t *server,
                            nrm_server_user_callbacks_t callbacks);

int nrm_server_settimer(nrm_server_t *server, nrm_time_t sleeptime);

int nrm_server_start(nrm_server_t *server);

int nrm_server_publish(nrm_server_t *server,
                       nrm_string_t topic,
                       nrm_time_t now,
                       nrm_string_t sensor_uuid,
                       nrm_scope_t *scope,
                       double value);

int nrm_server_actuate(nrm_server_t *server, nrm_string_t uuid, double value);

/**
 * Destroys an NRM server. Closes connections.
 */
void nrm_server_destroy(nrm_server_t **server);

/*******************************************************************************
 * NRM Reactor object
 * Used by any program that wants to become an idle loop only waking up on
 * signals or timers.
 ******************************************************************************/

typedef struct nrm_reactor_s nrm_reactor_t;

/** User-level callbacks on reactor events */
struct nrm_reactor_user_callbacks_s {
	/* receiving a POSIX signal */
	int (*signal)(nrm_reactor_t *, struct signalfd_siginfo);
	/* timer trigger */
	int (*timer)(nrm_reactor_t *);
};

typedef struct nrm_reactor_user_callbacks_s nrm_reactor_user_callbacks_t;

/**
 * Creates a new NRM reactor.
 *
 * Uses a state to keep track of reactor objects that clients can add/remove
 * from the system.
 *
 * @param reactor: pointer to a valid state handle
 * @return 0 if successful, an error code otherwise
 *
 */
int nrm_reactor_create(nrm_reactor_t **reactor, sigset_t *mask);

int nrm_reactor_setcallbacks(nrm_reactor_t *reactor,
                             nrm_reactor_user_callbacks_t callbacks);

int nrm_reactor_settimer(nrm_reactor_t *reactor, nrm_time_t sleeptime);

int nrm_reactor_start(nrm_reactor_t *reactor);

/**
 * Destroys an NRM reactor. Closes connections.
 */
void nrm_reactor_destroy(nrm_reactor_t **reactor);

#ifdef __cplusplus
}
#endif

#endif
