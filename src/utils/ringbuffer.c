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

#include "internal/utringbuffer.h"

struct nrm_ringbuffer_s {
	UT_ringbuffer *ring;
	UT_icd icd;
};



int nrm_ringbuffer_create(nrm_ringbuffer_t **ringbuffer, const size_t size, const size_t element_size)
{
	nrm_ringbuffer_t *a;

	if (ringbuffer == NULL || element_size == 0 || size == 0)
		return -NRM_EINVAL;

	a = malloc(sizeof(*a));
	if (a == NULL)
		return -NRM_ENOMEM;

	a->icd.sz = element_size;
	a->icd.init = NULL;
	a->icd.copy = NULL;
	a->icd.dtor = NULL;

	utringbuffer_new(a->ring, size, &a->icd);
	*ringbuffer = a;
	return NRM_SUCCESS;
}

void nrm_ringbuffer_destroy(nrm_ringbuffer_t **ringbuffer)
{
	if (ringbuffer == NULL)
		return;

	utringbuffer_free((*ringbuffer)->ring);
	free(*ringbuffer);
	*ringbuffer = NULL;
}

int nrm_ringbuffer_length(const nrm_ringbuffer_t *ringbuffer, size_t *length)
{
	if (ringbuffer == NULL || length == NULL)
		return -NRM_EINVAL;

	*length = utringbuffer_len(ringbuffer->ring);
	return NRM_SUCCESS;
}

int nrm_ringbuffer_get(const nrm_ringbuffer_t *ringbuffer, size_t index, void **out)
{
	if (ringbuffer == NULL || out == NULL)
		return -NRM_EINVAL;

	*out = utringbuffer_eltptr(ringbuffer->ring, index);
	if (*out == NULL)
		return -NRM_EDOM;
	return NRM_SUCCESS;
}

int nrm_ringbuffer_push_back(nrm_ringbuffer_t *ringbuffer, const void *element)
{
	if (ringbuffer == NULL || element == NULL)
		return -NRM_EINVAL;

	utringbuffer_push_back(ringbuffer->ring, element);
	return NRM_SUCCESS;
}

int nrm_ringbuffer_isempty(const nrm_ringbuffer_t *ringbuffer)
{
	if (ringbuffer == NULL)
		return 1;
	return utringbuffer_empty(ringbuffer->ring);
}

int nrm_ringbuffer_isfull(const nrm_ringbuffer_t *ringbuffer)
{
	if (ringbuffer == NULL)
		return 0;
	return utringbuffer_full(ringbuffer->ring);
}

int nrm_ringbuffer_clear(const nrm_ringbuffer_t *ringbuffer)
{
	if (ringbuffer == NULL)
		return -NRM_EINVAL;
	utringbuffer_clear(ringbuffer->ring);
	return 0;
}
