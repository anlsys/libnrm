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

#define NRM_ATOMIC_FINC(p) __atomic_fetch_add(p, 1, __ATOMIC_RELAXED)
#define NRM_ATOMIC_DECF(p) __atomic_sub_fetch(p, 1, __ATOMIC_RELAXED)

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

nrm_string_t nrm_string_fromprintf(const char *format, ...)
{
	va_list ap;
	size_t slen;
	nrm_realstring_t *rs;

	va_start(ap, format);
	/* memory needed to print this */
	slen = vsnprintf(NULL, 0, format, ap);
	va_end(ap);

	rs = nrm_string_new(slen);
	if (rs == NULL)
		return NULL;

	/* print at the end */
	nrm_string_t ret = NRM_STRING_S2C(rs);
	va_start(ap, format);
	vsnprintf(ret, rs->mlen, format, ap);
	va_end(ap);

	return ret;
}

void nrm_string_incref(nrm_string_t s)
{
	nrm_realstring_t *r = NRM_STRING_C2S(s);
	int ret = NRM_ATOMIC_FINC(&(r->rc));
	assert(ret > 0);
}

void nrm_string_decref(nrm_string_t s)
{
	nrm_realstring_t *r = NRM_STRING_C2S(s);
	int ret = NRM_ATOMIC_DECF(&(r->rc));
	assert(ret >= 0);
	if (ret == 0)
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

size_t nrm_string_strlen(const nrm_string_t s)
{
	if (s == NULL)
		return 0;

	nrm_realstring_t *r = NRM_STRING_C2S(s);
	return r->slen;
}

/* not thread safe */
int nrm_string_append(nrm_string_t *dest, nrm_string_t src)
{
	if (dest == NULL)
		return -NRM_EINVAL;

	if (src == NULL)
		return 0;
	
	nrm_realstring_t *r = NRM_STRING_C2S(*dest);
	nrm_realstring_t *s = NRM_STRING_C2S(src);

	nrm_realstring_t *t = nrm_string_new(r->slen + s->slen);
	nrm_string_t ret = NRM_STRING_S2C(t);
	memcpy(ret, *dest, r->slen);
	memcpy(ret+ r->slen, src, s->slen);
	nrm_string_decref(*dest);
	*dest = ret;
	return 0;
}
