/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#ifndef NRM_VECTORS_H
#define NRM_VECTORS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup nrm_vector "NRM Vector API"
 * @brief NRM Vector API
 *
 * Generic vector type storing contiguous elements in a flat buffer.
 * This vector is optimized to align element on elements size boundary.
 * Elements stored in the vector are supposed to be trivially copyable,
 * that is, they are not deep copied.
 *
 * @{
 **/

/** NRM vector structure: some utils are kept opaque on purpose to avoid
 * publicly exporting underlying support code. **/
struct nrm_vector_s;

typedef struct nrm_vector_s nrm_vector_t;

/**
 * Provides the number of elements in the vector.
 *
 * @param[in] vector: an initialized vector structure.
 * @param[out] length: pointer to returned length of vector
 * @return NRM_SUCCESS on success.
 * @return -NRM_EINVAL if `vector` or `length` are NULL.
 **/
int nrm_vector_length(const nrm_vector_t *vector, size_t *length);

/**
 * Get a pointer to element at position `index`.
 *
 * @param[in] vector: an initialized vector structure.
 * @param[in] index: an integer in [[ 0, vector->len [[.
 * @param[in] out: A pointer where to store the pointer to vector element.
 * If `out` is NULL, nothing is stored.
 * @return NRM_SUCCESS on success.
 * @return -NRM_EINVAL if `vector` or `out` are NULL.
 * @return -NRM_EDOM if `index` is out of bounds..
 **/
int nrm_vector_get(const nrm_vector_t *vector, size_t index, void **out);

/**
 * Find the first element matching key.
 *
 * @param[in] vector: an initialized vector structure.
 * @param[in] key: the key to look for.
 * @param[in] comp: A comparison function returning 0 when input elements match.
 * @param[out] pos: A pointer where to store the position of the found
 * element. If pos is NULL, nothing is stored.
 * @return -NRM_EINVAL if `vector`, `key`, `pos`, or `comp` are NULL.
 * @return -NRM_FAILURE if key was not found.
 * @return NRM_SUCCESS if key was found.
 **/
int nrm_vector_find(const nrm_vector_t *vector,
                    const void *key,
                    int (*comp)(const void *, const void *),
                    size_t *pos);

/**
 * Sort vector elements.
 *
 * @param[in] vector: an initialized vector structure.
 * @param[in] comp: A comparison function like linux `qsort` comparison
 *function.
 * @return -NRM_EINVAL if `vector` or `comp` is NULL.
 * @return NRM_SUCCESS.
 **/
int nrm_vector_sort(nrm_vector_t *vector,
                    int (*comp)(const void *, const void *));

/**
 * Find the first element matching key in a sorted vector.
 *
 * @param[in] vector: an initialized vector structure.
 * @param[in] key: the key to look for.
 * @param[in] comp: A comparison function like linux `qsort` comparison
 * function.
 * @param[out] pos: A pointer where to store the position of the found
 * element. If pos is NULL, nothing is stored.
 * @return -NRM_EINVAL if `vector`, `key`, `pos`, or `comp` are NULL.
 * @return -NRM_FAILURE if key was not found.
 * @return NRM_SUCCESS if key was found.
 **/
int nrm_vector_bsearch(const nrm_vector_t *vector,
                       const void *key,
                       int (*comp)(const void *, const void *),
                       size_t *pos);

/**
 * Resizes the vector.
 *
 * @param[in, out] vector: A pointer to an initialized vector structure.
 * The pointer might be updated to point to a new allocation.
 * @param newlen: a new vector length. If newlen is less than current length
 * the vector is truncated to newlen. If newlen is more than current
 * allocated length, then the vector is extended.
 * @return NRM_SUCCESS if successful; -NRM_ENOMEM otherwise.
 **/
int nrm_vector_resize(nrm_vector_t *vector, size_t newlen);

/**
 * Append an element at the end of the vector.
 * Vector is automatically enlarged if needed.
 *
 * @param[in, out] vector: A pointer to an initialized vector structure.
 * The pointer might be updated to point to a new allocation if `vector`
 * needs to be extended.
 * @param[in] element: The element to append to vector.
 * @return NRM_SUCCESS on success, or -NRM_ENOMEM if a reallocation failed.
 **/
int nrm_vector_push_back(nrm_vector_t *vector, const void *element);

/**
 * Remove element at the end of the vector.
 *
 * @param[in] vector: A pointer to an initialized vector structure.
 * @param[out] out: A memory area with size of at
 * least `vector->element_size` to copy end element.
 * If `out` is NULL or `vector` is empty, nothing is copied.
 * @return NRM_SUCCESS on success
 * @return -NRM_EDOM if `vector` is empty.
 * @return -NRM_EINVAL if `vector` is NULL.
 **/
int nrm_vector_pop_back(nrm_vector_t *vector, void *out);

/**
 * Remove and retrieve element at a specific position.
 * This method may require a large `memmove()` call.
 *
 * @param[in] vector: A pointer to an initialized vector structure.
 * @param[in] position: An index position in vector.
 * @param[out] out: A memory area with size of at least `vector->element_size`
 *to copy vector element. If `out` is NULL, nothing is copied.
 * @return -NRM_EINVAL if `vector` or `out` are NULL.
 * @return -NRM_EDOM if `position` is out of bounds.
 * @return NRM_SUCCESS on success.
 **/
int nrm_vector_take(nrm_vector_t *vector, const size_t position, void *out);

int nrm_vector_clear(const nrm_vector_t *vector);

/**
 * Allocate and initialize an empty vector.
 *
 * @param[out] vector: A pointer to an uninitialized vector structure.
 * @param[in] element_size: The size of elements in vector.
 * @return NRM_SUCCESS on success.
 * @return -NRM_ENOMEM if allocation failed.
 * @return -NRM_EINVAL if `element_size` is 0 or `vector` is NULL.
 **/
int nrm_vector_create(nrm_vector_t **vector, const size_t element_size);

/**
 * Release memory occupied by an vector.
 *
 * @param[in, out] vector: a vector created by `nrm_vector_create()`.
 * `NULL` after return.
 **/
void nrm_vector_destroy(nrm_vector_t **vector);

/**
 * Get an element from a vector.
 *
 * @param[in] type: the object type.
 * @param[in] vector: an initialized vector of structure nrm_vector_t.
 * @param[in] index: the index to search for.
 * @param[out] out: a pointer for the element we got.
 **/
#define nrm_vector_get_withtype(type, vector, index, out)                      \
	do {                                                                   \
		void *__p;                                                     \
		nrm_vector_get(vector, index, &__p);                           \
		out = (type *)__p;                                             \
	} while (0)
/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // NRM_VECTOR_H
