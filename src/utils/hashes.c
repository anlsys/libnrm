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
	nrm_string_t *uuid;
	void *ptr;
	UT_hash_handle hh;
};

struct nrm_hash_iterator_s {
	nrm_hash_t *element;
};

int nrm_hash_create_element(nrm_hash_t **element)
{
	nrm_hash_t *tmp_element = (nrm_hash_t *)malloc(sizeof(nrm_hash_t));
	tmp_element->uuid = malloc(sizeof(nrm_string_t *));
	tmp_element->ptr = malloc(sizeof(void *));
	if (tmp_element == NULL)
		return -NRM_ENOMEM;
	*element = tmp_element;
	return NRM_SUCCESS;
}

void *nrm_hash_get_uuid(nrm_hash_t *element)
{
	if (element == NULL)
		return -NRM_EINVAL;
	return element->uuid;
}

void *nrm_hash_get_ptr(nrm_hash_t *element)
{
	if (element == NULL)
		return -NRM_EINVAL;
	return element->ptr;
}

int nrm_hash_add(nrm_hash_t **hash_table, nrm_string_t *uuid, void *ptr)
{
	nrm_hash_t *local = NULL;
	int ret = nrm_hash_create_element(&local);
	if (ret != NRM_SUCCESS)
		return -NRM_ENOMEM;
	local->uuid = uuid;
	local->ptr = ptr;
	HASH_ADD(hh, (*hash_table), uuid, sizeof(uuid), local);
	return NRM_SUCCESS;
}

int nrm_hash_delete_element(nrm_hash_t **hash_table, nrm_string_t *uuid)
{
	if (*hash_table == NULL || uuid == NULL)
		return -NRM_EINVAL;
	nrm_hash_t *tmp;
	HASH_FIND(hh, (*hash_table), &uuid, sizeof(uuid), tmp);
	if (tmp == NULL)
		return -NRM_EINVAL;
	HASH_DEL(*hash_table, tmp);
	free(tmp);
	return NRM_SUCCESS;
}

int nrm_hash_delete_table(nrm_hash_t **hash_table)
{
	if (hash_table == NULL)
		return -NRM_EINVAL;
	nrm_hash_t *iterator = NULL;
	nrm_hash_t *tmp = NULL;
	HASH_ITER(hh, *hash_table, iterator, tmp)
	{
		HASH_DEL(*hash_table, iterator);
	}
	free(iterator);
	free(tmp);
	return NRM_SUCCESS;
}

int nrm_hash_find(nrm_hash_t *hash_table,
                  nrm_hash_t **element,
                  nrm_string_t *uuid_key)
{
	if (hash_table == NULL || uuid_key == NULL)
		return -NRM_EINVAL;
	nrm_hash_t *tmp = NULL;
	HASH_FIND(hh, hash_table, &uuid_key, sizeof(uuid_key), tmp);
	if (tmp == NULL)
		return -NRM_ENOMEM;
	*element = tmp;
	return NRM_SUCCESS;
}

int nrm_hash_size(nrm_hash_t *hash_table, size_t **len)
{
	if (hash_table == NULL)
		return -NRM_EINVAL;
	*len = HASH_COUNT(hash_table);
	return NRM_SUCCESS;
}

int nrm_hash_cpy(nrm_hash_t **hash_table, nrm_hash_t *tmp_table)
{
	nrm_hash_iterator_t *iter = NULL;
	nrm_hash_iterator_create(&iter);
	nrm_hash_t *tmp = NULL;
	assert(tmp_table != NULL);
	nrm_hash_iter(tmp_table, iter, tmp)
	{
		nrm_hash_add(hash_table, tmp->uuid, tmp->ptr);
	}
	return NRM_SUCCESS;
}

int nrm_hash_iterator_create(nrm_hash_iterator_t **iterator)
{
	*iterator = malloc(sizeof(nrm_hash_iterator_t));
	if (*iterator == NULL)
		return -NRM_ENOMEM;
	return NRM_SUCCESS;
}

int nrm_hash_iterator_begin(nrm_hash_iterator_t **iterator,
                            nrm_hash_t *hash_table)
{
	if (hash_table == NULL)
		return -NRM_EINVAL;
	(*iterator)->element = hash_table;
	return NRM_SUCCESS;
}

int nrm_hash_iterator_next(nrm_hash_iterator_t **iterator)
{
	(*iterator)->element = ((*iterator)->element)->hh.next;
	return NRM_SUCCESS;
}

void *nrm_hash_iterator_get(nrm_hash_iterator_t *iterator)
{
	return (iterator)->element;
}