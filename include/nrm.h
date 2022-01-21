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
 * Downstream API
 ******************************************************************************/

/** an opaque context required for a sensor to emit data back to the NRM. */
struct nrm_sensor_emitter_ctxt;

/** allocates an new empty context for a sensor emitter */
struct nrm_sensor_emitter_ctxt *nrm_sensor_emitter_create(void);

/** destroys a context for a sensor emitter. pointer is set to NULL upon
 * completion.
 */
int nrm_sensor_emitter_destroy(struct nrm_sensor_emitter_ctxt *);

/**
 * Establish a connection and set up basic information about the emitter.
 **/
int nrm_sensor_emitter_start(struct nrm_sensor_emitter_ctxt *ctxt,
			    const char *sensor_name);

/**
 * Disconnect from the NRM and shutdown the sensor
 **/
int nrm_sensor_emitter_exit(struct nrm_sensor_emitter_ctxt *ctxt);

/**
 * @param progress: cumulative value that represents the application progress
 * since the last progress report.
 */
int nrm_sensor_emitter_send(struct nrm_sensor_emitter_ctxt *ctxt,
			    unsigned long progress);

#ifdef __cplusplus
}
#endif

#endif
