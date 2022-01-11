/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include "nrm.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NRM_BITMAP_EMPTY       (0UL)
#define NRM_BITMAP_FULL        (~0UL)
#define NRM_BITMAP_NTH(i)      ((i) / NRM_BITMAP_NBITS)
#define NRM_BITMAP_ITH(i)      (((i) % NRM_BITMAP_NBITS))

/**
 * Check whether a bit in bitmap is set.
 **/
int nrm_bitmap_isset(const struct nrm_bitmap *bitmap, const unsigned int i)
{
	if (bitmap == NULL)
		return -1;
	if (i >= NRM_BITMAP_MAX)
		return -1;
	return !!(bitmap->mask[NRM_BITMAP_NTH(i)] & (1UL << NRM_BITMAP_ITH(i)));
}

/**
 * Set a bit in bitmap.
 **/
int nrm_bitmap_set(struct nrm_bitmap *bitmap, const unsigned int i)
{
	if (bitmap == NULL)
		return -1;
	if (i >= NRM_BITMAP_MAX)
		return -1;
	bitmap->mask[NRM_BITMAP_NTH(i)] |= (1UL << NRM_BITMAP_ITH(i));
	return 0;
}

/**
 * Set a bit in bitmap.
 **/
int nrm_bitmap_set_atomic(struct nrm_bitmap *bitmap, const unsigned int i)
{
	if (bitmap == NULL)
		return -1;
	if (i >= NRM_BITMAP_MAX)
		return -1;

	__atomic_and_fetch(&bitmap->mask[NRM_BITMAP_NTH(i)], 1UL <<
			   NRM_BITMAP_ITH(i), __ATOMIC_RELAXED);
	return 0;
}

/**
 * Count the number of bits set in bitmap.
 **/
int nrm_bitmap_nset(const struct nrm_bitmap *bitmap)
{
	unsigned long test = 1UL;
	int nset = 0;

	if (bitmap == NULL)
		return -1;

	for (size_t n = 0; n < NRM_BITMAP_SIZE; n++) {
		unsigned long b = bitmap->mask[n];

		for (size_t i = 0; i < NRM_BITMAP_NBITS; i++) {
			nset += b & test ? 1 : 0;
			b = b >> 1;
		}
	}
	return nset;
}

/**
 * Bitmap conversion to string. Output strings are index numbers wrapped in brackets [].
 **/
char *nrm_bitmap_to_string(const struct nrm_bitmap *bitmap)
{
	if(bitmap == NULL)
		return NULL;

	size_t bufsize = 256;
	char *buf = calloc(bufsize, sizeof(char));
	assert(buf != NULL);
	size_t nbits = nrm_bitmap_nset(bitmap);

	char *cur = buf;
	size_t size = 0;

	for(size_t i = 0, printed = 0; i < NRM_BITMAP_MAX && printed <
	    nbits; i++) {

		if(nrm_bitmap_isset(bitmap, i)) {
			int used;
			/* if buffer is too small, realloc */
			if(log10f(i) >= bufsize - size) {
				buf = realloc(buf, bufsize * 2);
				bufsize = bufsize * 2;
			}
			/* do we need a comma ? */
			if(printed)
				used = snprintf(cur, bufsize - size, ",%zd", i);
			else
				used = snprintf(cur, bufsize - size, "%zd", i);
			assert(used >= 0);
			size += used;
			cur += used;
			printed++;
		}
	}
	return buf;
}
