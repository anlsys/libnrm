/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#ifndef NRM_ERROR_H
#define NRM_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup nrm_error "NRM Error Management"
 * @brief NRM Error Codes and Functions
 *
 * Error codes and error helper functions.
 * As is quite common in C code, error code values are defined in positive,
 * but are returned in negative.
 * @{
 **/

/**
 * Variable set by nrm function calls. nrm_errno should be checked after an nrm
 * function call returning an error and prior to any other nrm call that may
 * overwrite it.
 **/
extern int nrm_errno;

/**
 * Get a string description of an nrm error.
 * @param err: the nrm error number.
 * Returns a static string describing the error.
 **/
const char *nrm_strerror(const int err);

/**
 * Print error on standard error output.
 * "msg": A message to prepend to error message.
 **/
void nrm_perror(const char *msg);

/** Generic value for success **/
#define NRM_SUCCESS 0

/**
 * Generic value for failure
 * Usually when this is the returned value,
 * the function will detail another way to
 * inspect error.
 **/
#define NRM_FAILURE 1

/**
 * Not enough memory was available
 * for fulfilling NRM function call.
 **/
#define NRM_ENOMEM 2

/**
 * One of the argument provided to
 * NRM function call was invalid.
 **/
#define NRM_EINVAL 3

/**
 * One of the arguments provided
 * to NRM function call has out of bound
 * value.
 **/
#define NRM_EDOM 4

/**
 * Invoked NRM abstraction function is actually
 * not implemented for this particular version of
 * NRM abstraction.
 **/
#define NRM_ENOTSUP 5

/**
 * Invoked NRM abstraction function has failed
 * because the resource it works on was busy.
 **/
#define NRM_EBUSY 6

/**
 * Invoked NRM abstraction function has failed
 * because the user does not have required permissions.
 **/
#define NRM_EPERM 7

/**
 * Value not found in the collection
 **/
#define NRM_ENOTFOUND 8

/**
 * Max allowed value for errors.
 **/
#define NRM_ERROR_MAX 9

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // NRM_ERROR_H
