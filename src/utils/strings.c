/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nrm.h"

struct nrm_realstring_s {
	size_t slen;
	size_t mlen;
	unsigned int rc;
};

typedef struct nrm_realstring_s nrm_realstring_t;

#define NRM_STRING_HEADER_SIZE (sizeof(nrm_realstring_t))
#define NRM_STRING_C2S(s)                                                      \
	(nrm_realstring_t *)(((intptr_t)s) - NRM_STRING_HEADER_SIZE)
#define NRM_STRING_S2C(s) (char *)(((intptr_t)s) + NRM_STRING_HEADER_SIZE)

static nrm_realstring_t *nrm_string_new(size_t slen)
{
	/* header + buffer + null byte */
	size_t len = NRM_STRING_HEADER_SIZE + slen + 1;
	nrm_realstring_t *ret = calloc(len, sizeof(char));
	if (ret) {
		ret->mlen = slen + 1;
		ret->slen = slen;
		ret->rc = 1;
	}
	return ret;
}

nrm_string_t nrm_string_fromchar(const char *string)
{
	/* from libc example, computes the number of characters needed to print
	 * the string
	 */
	size_t slen = snprintf(NULL, 0, "%s", string);
	nrm_realstring_t *rs = nrm_string_new(slen);
	if (rs == NULL)
		return NULL;
	nrm_string_t ret = NRM_STRING_S2C(rs);
	memcpy(ret, string, slen);
	return ret;
}

nrm_string_t nrm_string_frombuf(const char *string, size_t len)
{
	nrm_realstring_t *rs = nrm_string_new(len);
	if (rs == NULL)
		return NULL;
	nrm_string_t ret = NRM_STRING_S2C(rs);
	memcpy(ret, string, len);
	return ret;
}
void nrm_string_incref(nrm_string_t s)
{
	nrm_realstring_t *r = NRM_STRING_C2S(s);
	r->rc++;
}

void nrm_string_decref(nrm_string_t s)
{
	nrm_realstring_t *r = NRM_STRING_C2S(s);
	r->rc--;
	if (r->rc == 0)
		free(r);
}

int nrm_string_cmp(nrm_string_t one, nrm_string_t two)
{
	if (one == NULL && two == NULL)
		return 0;
	if (one == NULL || two == NULL)
		return 1;
	nrm_realstring_t *r1 = NRM_STRING_C2S(one);
	return strncmp(one, two, r1->slen);
}
