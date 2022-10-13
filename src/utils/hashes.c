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
	nrm_uuid_t *uuid;
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
	;
	tmp_element->ptr = malloc(sizeof(void *));
	if (tmp_element == NULL)
		return -NRM_ENOMEM;
	*element = tmp_element;
	return NRM_SUCCESS;
}

int nrm_hash_set_uuid(nrm_hash_t **element, nrm_string_t *uuid)
{
	if ((*element) == NULL)
		return -NRM_EINVAL;
	(*element)->uuid = uuid;
	return NRM_SUCCESS;
}

int nrm_hash_print_uuid(nrm_hash_t *element)
{
	if (element == NULL)
		return -NRM_EINVAL;
	printf("UUID: %s\n", element->uuid);
	return NRM_SUCCESS;
}

int nrm_hash_add(nrm_hash_t **hash_table, nrm_string_t *uuid)
{
	nrm_hash_t *local = NULL;
	int ret = nrm_hash_create_element(&local);
	local->uuid = uuid;
	local->ptr = NULL;
	HASH_ADD(hh, *hash_table, uuid, sizeof(uuid), local);
	return NRM_SUCCESS;
}

int nrm_hash_delete_element(nrm_hash_t *hash_table, nrm_hash_t *element)
{
	if (hash_table == NULL || element == NULL)
		return -NRM_EINVAL;
	HASH_DEL(hash_table, element);
	free(element);
	return NRM_SUCCESS;
}

int nrm_hash_delete_table(nrm_hash_t *hash_table)
{
	if (hash_table == NULL)
		return -NRM_EINVAL;
	nrm_hash_t *iterator = NULL;
	nrm_hash_t *tmp = NULL;
	HASH_ITER(hh, hash_table, iterator, tmp)
	{
		HASH_DEL(hash_table, iterator);
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
	HASH_FIND(hh, hash_table, &uuid_key, sizeof(&uuid_key), tmp);
	*element = tmp;
	return NRM_SUCCESS;
}

int nrm_hash_size(nrm_hash_t *hash_table)
{
	if (hash_table == NULL)
		return -NRM_EINVAL;
	return HASH_COUNT(hash_table);
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

int nrm_hash_iterator_next(nrm_hash_iterator_t **iterator,
                           nrm_hash_t *hash_table)
{
	if (hash_table == NULL)
		return -NRM_EINVAL;
	(*iterator)->element = (hash_table)->hh.next;
	return NRM_SUCCESS;
}

nrm_hash_t *nrm_hash_iterator_get(nrm_hash_iterator_t *iterator)
{
	return (iterator)->element;
}
