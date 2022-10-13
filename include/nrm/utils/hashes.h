/*******************************************************************************
 * Copyright 2022 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#ifndef NRM_HASHES_H
#define NRM_HASHES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup nrm_hash "NRM Hash API"
 * @brief NRM Hash API
 *
 * Generic hash type and its iterator.
 *
 * @{
 **/

/** NRM hash structure: some utils are kept opaque on purpose to avoid
 * publicly exporting underlying support code. **/
struct nrm_hash_s;

/** NRM hash iterator structure: some utils are kept opaque on purpose to avoid
 * publicly exporting underlying support code. **/
struct nrm_hash_iterator_s;

typedef struct nrm_hash_s nrm_hash_t;
typedef struct nrm_hash_iterator_s nrm_hash_iterator_t;

/**
 * Allocate memory for a hash table element.
 *
 * @param[in] element: an initialized hash structure.
 * @return NRM_SUCCESS on success.
 * @return NRM_ENOMEM on failure.
 **/
int nrm_hash_create_element(nrm_hash_t **element);

/**
 * Set UUID for a given hash table element.
 *
 * @param[in] element: an initialized hash structure.
 * @param[in] uuid: the UUID to set.
 * @return NRM_SUCCESS on success.
 * @return NRM_EINVAL on failure, i.e. `element` is NULL.
 **/
int nrm_hash_set_uuid(nrm_hash_t **element, nrm_string_t *uuid);

/**
 * Print the UUID for a given hash table element.
 *
 * @param[in] element: an initialized element of hash structure.
 * @return NRM_SUCCESS on success.
 * @return NRM_EINVAL on failure, i.e. `element` is NULL.
 **/
int nrm_hash_print_uuid(nrm_hash_t *element);

/**
 * Add an element to a hash table using the UUID.
 *
 * @param[in] hash_table: an initialized hash structure.
 * @param[in] uuid: the UUID of the new element to add.
 * @return NRM_SUCCESS on success.
 * @return NRM_ENOMEM on failure.
 **/
int nrm_hash_add(nrm_hash_t **hash_table, nrm_string_t *uuid);

/**
 * Delete an element from a hash table.
 *
 * @param[in] hash_table: an initialized hash structure.
 * @param[in] element: an initialized element of hash structure.
 * @return NRM_SUCCESS on success.
 * @return NRM_EINVAL on failure, i.e. `hash_table` or `element` is NULL.
 **/
int nrm_hash_delete_element(nrm_hash_t *hash_table, nrm_hash_t *element);

/**
 * Delete a hash table.
 *
 * @param[in] hash_table: an initialized hash structure.
 * @return NRM_SUCCESS on success.
 * @return NRM_EINVAL on failure, i.e. `hash_table` is NULL.
 **/
int nrm_hash_delete_table(nrm_hash_t *hash_table);

/**
 * Find an element in a hash table.
 *
 * @param[in] hash_table: an initialized hash structure.
 * @param[in] element: an initialized element of hash structure.
 * @param[in] uuid: the UUID of the element to find.
 * @return NRM_SUCCESS on success.
 * @return NRM_EINVAL on failure, i.e. `hash_table` or `uuid` is NULL.
 **/
int nrm_hash_find(nrm_hash_t *hash_table,
                  nrm_hash_t **element,
                  nrm_string_t *uuid);

/**
 * Get the number of elements in a hash table.
 *
 * @param[in] hash_table: an initialized hash structure.
 * @return the number of elements in `hash_table` on success.
 * @return NRM_EINVAL on failure, i.e. `hash_table` is NULL.
 **/
int nrm_hash_size(nrm_hash_t *hash_table);

/**
 * Creates an iterator object of struct nrm_hash_iterator_t.
 *
 * @param[in] iterator: an initialized element of hash iterator structure.
 * @return NRM_SUCCESS on success.
 * @return NRM_ENOMEM on failure.
 **/
int nrm_hash_iterator_create(nrm_hash_iterator_t **iterator);

/**
 * Selects the first element in a hash table.
 *
 * @param[in] iterator: an initialized element of hash iterator structure.
 * @param[in] hash_table: an initialized hash structure.
 * @return NRM_SUCCESS on success.
 * @return NRM_EINVAL on failure, i.e. `hash_table` is NULL.
 **/
int nrm_hash_iterator_begin(nrm_hash_iterator_t **iterator,
                            nrm_hash_t *hash_table);

/**
 * Selects the next element in a hash table.
 *
 * @param[in] iterator: an initialized element of hash iterator structure.
 * @param[in] hash_table: an initialized hash structure.
 * @return NRM_SUCCESS on success.
 * @return NRM_EINVAL on failure, i.e. `hash_table` is NULL.
 **/
int nrm_hash_iterator_next(nrm_hash_iterator_t **iterator,
                           nrm_hash_t *hash_table);

/**
 * Get the nrm_hash_t element that the iterator is pointing to.
 *
 * @param[in] iterator: an iterator element of struct nrm_hash_iterator_t.
 * @return an nrm_hash_t object.
 **/
nrm_hash_t *nrm_hash_iterator_get(nrm_hash_iterator_t *iterator);

/**
 * Start a for loop iterating over the content of a hash table.
 *
 * @param[in] el: an initialized element of hash structure.
 * @param[in] hash_table: an initialized hash structure.
 * @param[in] iter: an initialized element of hash iterator structure.
 * @param[in] tmp: an initialized element of hash structure.
 * @param[out] el: an initialized element of hash structure containing the
 *selected element from the iteration over `hash_table`.
 **/
#define nrm_hash_iter(el, hash_table, iter, tmp)                               \
	for (el = hash_table,                                                  \
	    (hash_table != NULL) ? nrm_hash_iterator_next(&iter, hash_table) : \
	                            NULL,                                       \
	    tmp = (hash_table != NULL) ? nrm_hash_iterator_get(iter) : NULL;   \
	     el != NULL; el = tmp,                                             \
	    (tmp != NULL) ? nrm_hash_iterator_next(&iter, tmp) : NULL,         \
	    tmp = (tmp != NULL) ? nrm_hash_iterator_get(iter) : NULL)

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // NRM_HASH_H
