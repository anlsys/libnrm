/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#ifndef NRM_BITMAPS_H
#define NRM_BITMAPS_H 1

#define NRM_BITMAP_MAX 2048
/** The size in Bytes of aml_bitmap **/
#define NRM_BITMAP_BYTES (NRM_BITMAP_MAX / 8)
/** The type used to store bits **/
#define NRM_BITMAP_TYPE unsigned long
/** The number of basic type elements used to store bits **/
#define NRM_BITMAP_SIZE ((size_t)(NRM_BITMAP_BYTES / sizeof(NRM_BITMAP_TYPE)))
/** The number of bits held in each basic type element **/
#define NRM_BITMAP_NBITS ((size_t)(8 * sizeof(NRM_BITMAP_TYPE)))

struct nrm_bitmap {
	unsigned long mask[NRM_BITMAP_SIZE];
};

typedef struct nrm_bitmap nrm_bitmap_t;

/**
 * Check whether a bit in bitmap is set.
 **/
int nrm_bitmap_isset(const struct nrm_bitmap *bitmap, const unsigned int i);

/**
 * Check whether a bit in bitmap is empty.
 **/
int nrm_bitmap_iszero(const struct nrm_bitmap *bitmap);

/**
 * Set a bit in bitmap.
 **/
int nrm_bitmap_set(struct nrm_bitmap *bitmap, const unsigned int i);

/**
 * Set a bit in bitmap atomically
 **/
int nrm_bitmap_set_atomic(struct nrm_bitmap *bitmap, const unsigned int i);

/**
 * Count the number of bits set in bitmap.
 **/
int nrm_bitmap_nset(const struct nrm_bitmap *bitmap);

/**
 * Bitmap conversion to string. Output strings are index numbers wrapped in
 *brackets [].
 **/
char *nrm_bitmap_to_string(const struct nrm_bitmap *bitmap);

/**
 * Copy a bitmap into another.
 */
int nrm_bitmap_copy(struct nrm_bitmap *dest, struct nrm_bitmap *src);

/**
 * Compare two bitmaps. Note that this return 0 on equality, like strcmp
 */
int nrm_bitmap_cmp(struct nrm_bitmap *one, struct nrm_bitmap *two);

/**
 * Fill an array with the list of ints that are set in the bitmap
 */
int nrm_bitmap_to_array(const struct nrm_bitmap *bitmap,
                        size_t *nitems,
                        int32_t **items);

/**
 * Fill a bitmap with the list of ints that are in an array
 */
int nrm_bitmap_from_array(struct nrm_bitmap *bitmap,
                          size_t nitems,
                          int32_t *items);

#endif
