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
#include <jansson.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nrm.h"



#define NRM_BITMAP_EMPTY (0UL)
#define NRM_BITMAP_FULL (~0UL)
#define NRM_BITMAP_NTH(i) ((i) / NRM_BITMAP_NBITS)
#define NRM_BITMAP_ITH(i) (((i) % NRM_BITMAP_NBITS))

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

	__atomic_or_fetch(&bitmap->mask[NRM_BITMAP_NTH(i)],
	                  1UL << NRM_BITMAP_ITH(i), __ATOMIC_RELAXED);
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

int nrm_bitmap_copy(struct nrm_bitmap *dest, struct nrm_bitmap *src)
{
	memcpy(dest->mask, src->mask, NRM_BITMAP_BYTES);
	return 0;
}

int nrm_bitmap_cmp(struct nrm_bitmap *one, struct nrm_bitmap *two)
{
	return memcmp(one->mask, two->mask, NRM_BITMAP_BYTES);
}

/**
 * Bitmap conversion to string. Output strings are index numbers wrapped in
 *brackets [].
 **/
char *nrm_bitmap_to_string(const struct nrm_bitmap *bitmap)
{
	if (bitmap == NULL)
		return NULL;

	size_t bufsize = 256;
	char *buf = calloc(bufsize, sizeof(char));
	assert(buf != NULL);
	size_t nbits = nrm_bitmap_nset(bitmap);

	char *cur = buf;
	/* need to keep space for the null byte */
	size_t size = 1;

	for (size_t i = 0, printed = 0; i < NRM_BITMAP_MAX && printed < nbits;
	     i++) {

		if (nrm_bitmap_isset(bitmap, i)) {
			int used;
			/* if buffer is too small, realloc */
			if (log10f(i) >= bufsize - size) {
				buf = realloc(buf, bufsize * 2);
				bufsize = bufsize * 2;
			}
			/* do we need a comma ? */
			if (printed)
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


json_t *nrm_bitmap_to_json(struct nrm_bitmap *bitmap)
{
	json_t *ret;
	size_t size = nrm_bitmap_nset(bitmap);

	ret = json_array();
	for(size_t i = 0, printed = 0; i < NRM_BITMAP_MAX && printed < size;
	    i++) {
		if (nrm_bitmap_isset(bitmap, i)) {
			json_t *val = json_integer(i);
			json_array_append_new(ret, val);
			printed++;
		}
	}
	return ret;
}


int nrm_bitmap_to_array(const struct nrm_bitmap *map, size_t *nitems, int32_t **items)
{
	size_t size = nrm_bitmap_nset(map);
	*nitems = size;
	*items = calloc(size, sizeof(int32_t));
	if (*items == NULL)
		return -NRM_ENOMEM;
	for (size_t i = 0, set = 0; i < NRM_BITMAP_MAX && set < size; i++) {
		if (nrm_bitmap_isset(map, i)) {
			*items[set] = i;
			set++;
		}
	}
	return 0;
}

int nrm_bitmap_from_array(struct nrm_bitmap *map, size_t nitems, int32_t *items)
{
	for (size_t i = 0; i < nitems; i++)
		nrm_bitmap_set(map, items[i]);
	return 0;
}

int nrm_bitmap_from_json(struct nrm_bitmap *bitmap, json_t *json)
{
	if (!json_is_array(json))
	    return -NRM_EINVAL;

	size_t index;
	json_t *value;
	json_array_foreach(json, index, value) {
		json_int_t i = json_integer_value(value);
		nrm_bitmap_set(bitmap, i);
	}
	return 0;
}

