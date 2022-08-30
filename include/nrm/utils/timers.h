/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#ifndef NRM_TIMERS_H
#define NRM_TIMERS_H 1

/*******************************************************************************
 * High Resolution Timers
 * type and functions to save a timestamp and compute a difference.
 * Resolution should be in nanoseconds.
 ******************************************************************************/

/**
 * Define type used to internally save timestamps (in nanoseconds since epoch)
 **/
typedef struct timespec nrm_time_t;

/**
 * Save timestamps into timer
 **/
void nrm_time_gettime(nrm_time_t *now);

/**
 * Compute the time difference between two timestamps, as nanoseconds.
 **/
int64_t nrm_time_diff(const nrm_time_t *start, const nrm_time_t *end);

/**
 * Convert timestamp into nanoseconds since epoch, as an int64_t value
 **/
int64_t nrm_time_tons(const nrm_time_t *time);

/**
 * Convert a nanoseconds timestamp into an NRM time value
 **/
nrm_time_t nrm_time_fromns(int64_t time);

#endif /* NRM_TIMERS_H */
