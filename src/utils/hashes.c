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

struct nrm_hash_iterator_s {
	nrm_hash_t *element;
};

static int nrm_hash_create_element(nrm_hash_t **element)
{
	nrm_hash_t *tmp_element = (nrm_hash_t *)malloc(sizeof(nrm_hash_t));
	tmp_element->uuid = NULL;
	tmp_element->ptr = NULL;
	if (tmp_element == NULL)
		return -NRM_ENOMEM;
	*element = tmp_element;
	return NRM_SUCCESS;
}

int nrm_hash_add(nrm_hash_t **hash_table, nrm_string_t uuid, void *ptr)
{
	nrm_hash_t *local = NULL;
	HASH_FIND(hh, (*hash_table), &uuid, strlen(uuid), local);
	if (local != NULL)
		return -NRM_FAILURE;

	int ret = nrm_hash_create_element(&local);
	if (ret != NRM_SUCCESS)
		return -NRM_ENOMEM;
	local->uuid = uuid;
	local->ptr = ptr;
	HASH_ADD_KEYPTR(hh, (*hash_table), &uuid, strlen(uuid), local);
	return NRM_SUCCESS;
}

int nrm_hash_remove(nrm_hash_t **hash_table, nrm_string_t uuid, void **ptr)
{
	if (*hash_table == NULL || uuid == NULL)
		return -NRM_EINVAL;
	nrm_hash_t *tmp;
	*ptr = nrm_hash_find((*hash_table), &tmp, uuid);
	if (tmp == NULL)
		return -NRM_EINVAL;
	printf("tmp = %s\n", tmp->uuid);
	HASH_DEL((*hash_table), tmp);
	free(tmp);
	return NRM_SUCCESS;
}

void nrm_hash_destroy(nrm_hash_t **hash_table)
{
	if (hash_table == NULL)
		return -NRM_EINVAL;
	nrm_hash_t *iterator = NULL;
	nrm_hash_t *tmp = NULL;
	HASH_ITER(hh, (*hash_table), iterator, tmp)
	{
		HASH_DEL((*hash_table), iterator);
		free(tmp);
	}
	free(iterator);
}

void *
nrm_hash_find(nrm_hash_t *hash_table, nrm_hash_t **element, nrm_string_t uuid)
{
	printf("Looking for %s from nrm_hash_find...\n", uuid);
	if (hash_table == NULL || uuid == NULL)
		return -NRM_EINVAL;
	printf("UUID of the first element of the table is %s\n",
	       hash_table->uuid);
	nrm_hash_t *tmp = NULL;
	HASH_FIND(hh, hash_table, &uuid, strlen(uuid), tmp);
	if (tmp == NULL) {
		printf("ELEMENT WAS NOT FOUND!!!\n");
		return -NRM_EINVAL;
	}

	*element = tmp;
	return tmp->ptr;
}

int nrm_hash_size(nrm_hash_t *hash_table, size_t *len)
{
	if (hash_table == NULL)
		return -NRM_EINVAL;
	*len = (size_t)HASH_COUNT(hash_table);
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
	if (iterator == NULL || iterator->element == NULL)
		return NULL;
	return iterator->element->ptr;
}

nrm_string_t nrm_hash_iterator_get_uuid(nrm_hash_iterator_t *iterator)
{
	if (iterator->element != NULL)
		return iterator->element->uuid;
}
