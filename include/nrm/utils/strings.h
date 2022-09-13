/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#ifndef NRM_STRINGS_H
#define NRM_STRINGS_H 1

typedef char *nrm_string_t;

nrm_string_t nrm_string_fromchar(const char *buf);
nrm_string_t nrm_string_frombuf(const char *buf, size_t len);

void nrm_string_incref(nrm_string_t s);

void nrm_string_decref(nrm_string_t s);

int nrm_string_cmp(const nrm_string_t, const nrm_string_t);

#endif
