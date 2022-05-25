/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#ifndef NRM_RINGBUFFER_H
#define NRM_RINGBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

struct nrm_ringbuffer_s;

typedef struct nrm_ringbuffer_s nrm_ringbuffer_t;

int nrm_ringbuffer_length(const nrm_ringbuffer_t *ringbuffer, size_t *length);


int nrm_ringbuffer_isempty(const nrm_ringbuffer_t *ringbuffer);
int nrm_ringbuffer_isfull(const nrm_ringbuffer_t *ringbuffer);
int nrm_ringbuffer_clear(const nrm_ringbuffer_t *ringbuffer);

int nrm_ringbuffer_get(const nrm_ringbuffer_t *ringbuffer, size_t index, void **out);

int nrm_ringbuffer_push_back(nrm_ringbuffer_t *ringbuffer, const void *element);

int nrm_ringbuffer_create(nrm_ringbuffer_t **ringbuffer, const size_t size, const size_t element_size);

/**
 * Release memory occupied by an ringbuffer.
 *
 * @param[in, out] ringbuffer: a ringbuffer created by `nrm_ringbuffer_create()`.
 * `NULL` after return.
 **/
void nrm_ringbuffer_destroy(nrm_ringbuffer_t **ringbuffer);

#ifdef __cplusplus
}
#endif

#endif // NRM_ringbuffer_H
