/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#ifndef NRM_PARSERS_H
#define NRM_PARSERS_H 1

int nrm_parse_int(const char *, int *);
int nrm_parse_uint(const char *, unsigned int *);
int nrm_parse_double(const char *, double *);
int nrm_parse_llu(const char *, unsigned long long *);

#endif
