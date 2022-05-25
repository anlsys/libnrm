/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

/* error checking logic */
#define utarray_oom()                                                          \
	do {                                                                   \
		goto utarray_error;                                            \
	} while (0)

#include "nrm.h"

#include "internal/utarray.h"

struct nrm_vector_s {
	UT_array *array;
	UT_icd icd;
};

int nrm_vector_create(nrm_vector_t **vector, const size_t element_size)
{
	nrm_vector_t *a;

	if (vector == NULL || element_size == 0)
		return -NRM_EINVAL;

	a = malloc(sizeof(*a));
	if (a == NULL)
		return -NRM_ENOMEM;

	a->icd.sz = element_size;
	a->icd.init = NULL;
	a->icd.copy = NULL;
	a->icd.dtor = NULL;

	utarray_new(a->array, &a->icd);
	*vector = a;
	return NRM_SUCCESS;
utarray_error:
	return -NRM_ENOMEM;
}

void nrm_vector_destroy(nrm_vector_t **vector)
{
	if (vector == NULL)
		return;

	utarray_free((*vector)->array);
	free(*vector);
	*vector = NULL;
}

int nrm_vector_resize(nrm_vector_t *vector, size_t newlen)
{
	if (vector == NULL)
		return -NRM_EINVAL;

	utarray_resize(vector->array, newlen);
	return NRM_SUCCESS;
utarray_error:
	return -NRM_ENOMEM;
}

int nrm_vector_length(const nrm_vector_t *vector, size_t *length)
{
	if (vector == NULL || length == NULL)
		return -NRM_EINVAL;

	*length = utarray_len(vector->array);
	return NRM_SUCCESS;
}

int nrm_vector_get(const nrm_vector_t *vector, size_t index, void **out)
{
	if (vector == NULL || out == NULL)
		return -NRM_EINVAL;

	*out = utarray_eltptr(vector->array, index);
	if (*out == NULL)
		return -NRM_EDOM;
	return NRM_SUCCESS;
}

int nrm_vector_find(const nrm_vector_t *vector,
                    const void *key,
                    int (*comp)(const void *, const void *),
                    size_t *pos)
{
	if (vector == NULL || comp == NULL || key == NULL)
		return -NRM_EINVAL;

	void *elt;
	for (elt = utarray_front(vector->array); elt != NULL;
	     elt = utarray_next(vector->array, elt)) {
		if (!comp(key, elt))
			break;
	}
	if (elt == NULL)
		return -NRM_FAILURE;

	*pos = utarray_eltidx(vector->array, elt);
	return NRM_SUCCESS;
}

int nrm_vector_sort(nrm_vector_t *vector,
                    int (*comp)(const void *, const void *))
{
	if (vector == NULL || comp == NULL)
		return -NRM_EINVAL;

	utarray_sort(vector->array, comp);
	return NRM_SUCCESS;
}

int nrm_vector_bsearch(const nrm_vector_t *vector,
                       const void *key,
                       int (*comp)(const void *, const void *),
                       size_t *pos)
{
	if (vector == NULL || comp == NULL || key == NULL)
		return -NRM_EINVAL;

	void *elt = utarray_find(vector->array, key, comp);
	if (elt == NULL)
		return -NRM_FAILURE;

	*pos = utarray_eltidx(vector->array, elt);
	return NRM_SUCCESS;
}

int nrm_vector_push_back(nrm_vector_t *vector, const void *element)
{
	if (vector == NULL || element == NULL)
		return -NRM_EINVAL;

	utarray_push_back(vector->array, element);
	return NRM_SUCCESS;
utarray_error:
	return -NRM_ENOMEM;
}

int nrm_vector_pop_back(nrm_vector_t *vector, void *out)
{
	if (vector == NULL || out == NULL)
		return -NRM_EINVAL;

	void *back = utarray_back(vector->array);
	if (back == NULL)
		return -NRM_EDOM;

	memcpy(out, back, vector->icd.sz);
	utarray_pop_back(vector->array);
	return NRM_SUCCESS;
}

int nrm_vector_take(nrm_vector_t *vector, const size_t position, void *out)
{
	if (vector == NULL)
		return -NRM_EINVAL;

	void *elt = utarray_eltptr(vector->array, position);
	if (elt == NULL)
		return -NRM_EDOM;

	if (out)
		memcpy(out, elt, vector->icd.sz);

	utarray_erase(vector->array, position, 1);
	return NRM_SUCCESS;
}
