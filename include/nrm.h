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
#include <stdio.h>
#include <time.h>

#include "nrm/utils/bitmaps.h"
#include "nrm/utils/scopes.h"
#include "nrm/utils/timers.h"
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

enum nrm_msg_type_e {
	NRM_MSG_TYPE_SENSOR_PROGRESS = 0,
	NRM_MSG_TYPE_SENSOR_PAUSE = 1,
};

struct nrm_msg_sspg_s {
	unsigned long progress;
	nrm_scope_t *scope;
	const char *name;
	const char *cmdid;
};

struct nrm_msg_sspa_s {
	const char *name;
	const char *cmdid;
};

struct nrm_msg_s {
	int type;	
	nrm_time_t timestamp;
	union {
		struct nrm_msg_sspg_s sspg;
		struct nrm_msg_sspa_s sspa;
	} u;
};

typedef struct nrm_msg_s nrm_msg_t;

nrm_msg_t *nrm_msg_new_progress(nrm_time_t timestamp, unsigned long progress,
				nrm_scope_t *scope);

nrm_msg_t *nrm_msg_new_pause(nrm_time_t timestamp);

void nrm_msg_fprintf(FILE *out, nrm_msg_t *msg);

void nrm_msg_destroy(nrm_msg_t **msg);

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

nrm_role_t *nrm_role_sensor_create_fromenv();

int nrm_role_send(const nrm_role_t *role, nrm_msg_t *msg);

nrm_msg_t *nrm_role_recv(const nrm_role_t *role);

void nrm_role_destroy(nrm_role_t **);

#ifdef __cplusplus
}
#endif

#endif
