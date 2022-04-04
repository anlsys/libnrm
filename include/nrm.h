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
 * NRM Messages
 * Messages being transmitted between nrm components
 ******************************************************************************/

typedef struct nrm_msg_s nrm_msg_t;

nrm_msg_t *nrm_msg_new(int type, ...);

#define nrm_typecheck(var, type) \
	do { \
	type __dummy; \
	__typeof__(var) __dummy2; \
	(void)(&__dummy == &__dummy2); } while(0)

#define nrm_msg_new_progress(t, p, s) ({ \
	nrm_typecheck(t, nrm_time_t); \
	nrm_typecheck(p, unsigned long); \
	nrm_typecheck(s, nrm_scope_t *); \
	nrm_msg_new((int)NRM_MSG_TYPE_SENSOR_PROGRESS, t, p, s); })

#define nrm_msg_new_pause(t) ({ \
	nrm_typecheck(t, nrm_time_t); \
	nrm_msg_new((int)NRM_MSG_TYPE_SENSOR_PAUSE, t); })


nrm_msg_t *nrm_msg_new_req_list(int target);
nrm_msg_t *nrm_msg_new_rep_list(int target, nrm_vector_t *items);

void nrm_msg_fprintf(FILE *out, nrm_msg_t *msg);

void nrm_msg_destroy(nrm_msg_t **msg);

/*******************************************************************************
 * Slice: a resource arbitration unit
 ******************************************************************************/

struct nrm_slice_s {
	char *name;
	nrm_uuid_t *uuid;
};

typedef struct nrm_slice_s nrm_slice_t;

nrm_slice_t *nrm_slice_create(char *name);

void nrm_slice_destroy(nrm_slice_t **);

void nrm_slice_fprintf(FILE *out, nrm_slice_t *);

/*******************************************************************************
 * State: the full state of a controller 
 ******************************************************************************/

struct nrm_state_s {
	nrm_vector_t *slices;
};

typedef struct nrm_state_s nrm_state_t;

/*******************************************************************************
 * NRM Role API
 * A "role" is a set of features of NRM that a client of this library is using.
 * A typical role is a sensor, or an actuator, or something monitoring sensor
 * messages and so on.
 * Behind each role is an event loop and a helper thread, related to its actions
 * in the communication infrastructure of the NRM.
 ******************************************************************************/

typedef struct nrm_role_s nrm_role_t;

nrm_role_t *nrm_role_monitor_create_fromenv();

nrm_role_t *nrm_role_sensor_create_fromenv(const char *sensor_name);

nrm_role_t *nrm_role_client_create_fromparams(const char *, int, int);

int nrm_role_send(const nrm_role_t *role, nrm_msg_t *msg, nrm_uuid_t *to);

nrm_msg_t *nrm_role_recv(const nrm_role_t *role, nrm_uuid_t **from);

void nrm_role_destroy(nrm_role_t **);

#ifdef __cplusplus
}
#endif

#endif
