/*******************************************************************************
 * Copyright 2022 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include "nrm.h"

#include "internal/uthash.h"

struct nrm_hash_s {
	nrm_string_t uuid;
	void *ptr;
	UT_hash_handle hh;
};

static int nrm_hash_create_element(nrm_hash_t **element)
{
	nrm_hash_t *tmp_element = (nrm_hash_t *)malloc(sizeof(nrm_hash_t));
	if (tmp_element == NULL)
		return -NRM_ENOMEM;
	tmp_element->uuid = NULL;
	tmp_element->ptr = NULL;
	*element = tmp_element;
	return NRM_SUCCESS;
}

int nrm_hash_add(nrm_hash_t **hash_table, nrm_string_t uuid, void *ptr)
{
	nrm_hash_t *local = NULL;
	HASH_FIND(hh, (*hash_table), uuid, nrm_string_strlen(uuid), local);
	if (local != NULL)
		return -NRM_FAILURE;

	int ret = nrm_hash_create_element(&local);
	if (ret != NRM_SUCCESS)
		return -NRM_ENOMEM;
	local->uuid = uuid;
	local->ptr = ptr;
	HASH_ADD_KEYPTR(hh, (*hash_table), uuid, nrm_string_strlen(uuid),
	                local);
	return NRM_SUCCESS;
}

int nrm_hash_remove(nrm_hash_t **hash_table, nrm_string_t uuid, void **ptr)
{
	if (*hash_table == NULL || uuid == NULL)
		return -NRM_EINVAL;

	nrm_hash_t *tmp;
	HASH_FIND(hh, (*hash_table), &uuid, nrm_string_strlen(uuid), tmp);

	*ptr = tmp;
	if (tmp != NULL) {
		HASH_DEL((*hash_table), tmp);
		free(tmp);
	}
	return NRM_SUCCESS;
}

void nrm_hash_destroy(nrm_hash_t **hash_table)
{
	if (hash_table == NULL)
		return;
	nrm_hash_t *iterator = NULL;
	nrm_hash_t *tmp = NULL;
	HASH_ITER(hh, (*hash_table), iterator, tmp)
	{
		HASH_DEL((*hash_table), iterator);
		free(iterator);
	}
	*hash_table = NULL;
}

int nrm_hash_find(nrm_hash_t *hash_table, nrm_string_t uuid, void **ptr)
{
	if (hash_table == NULL || uuid == NULL || ptr == NULL)
		return -NRM_EINVAL;

	nrm_hash_t *tmp = NULL;
	HASH_FIND(hh, hash_table, uuid, nrm_string_strlen(uuid), tmp);
	if (tmp == NULL)
		return -NRM_EINVAL;

	*ptr = tmp->ptr;
	return NRM_SUCCESS;
}

int nrm_hash_size(nrm_hash_t *hash_table, size_t *len)
{
	if (hash_table == NULL)
		return -NRM_EINVAL;
	*len = (size_t)HASH_COUNT(hash_table);
	return NRM_SUCCESS;
}

nrm_hash_iterator_t nrm_hash_iterator_begin(nrm_hash_t *hash_table)
{
	nrm_hash_iterator_t ret = hash_table;
	return ret;
}

nrm_hash_iterator_t nrm_hash_iterator_next(nrm_hash_iterator_t iterator)
{
	return iterator->hh.next;
}

void *nrm_hash_iterator_get(nrm_hash_iterator_t iterator)
{
	if (iterator == NULL)
		return NULL;
	return iterator->ptr;
}

nrm_string_t nrm_hash_iterator_get_uuid(nrm_hash_iterator_t iterator)
{
	if (iterator != NULL)
		return iterator->uuid;
	else
		return NULL;
}
