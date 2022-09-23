/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#ifndef LIBNRM_INTERNAL_CONTROL_H
#define LIBNRM_INTERNAL_CONTROL_H 1

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Global definitions
 ******************************************************************************/

typedef struct nrm_control_s nrm_control_t;

typedef struct {
	nrm_string_t sensor_uuid;
	nrm_string_t scope_uuid;
	double value;
} nrm_control_input_t;

typedef struct {
	nrm_string_t actuator_uuid;
	double value;
} nrm_control_output_t;

struct nrm_control_data;

struct nrm_control_ops {
	int (*create)(nrm_control_t **control, json_t *config);
	int (*getargs)(nrm_control_t *control, nrm_vector_t **inputs, nrm_vector_t **outputs);
	int (*action)(nrm_control_t *control, nrm_vector_t *inputs, nrm_vector_t *outputs);
	int (*destroy)(nrm_control_t **control);
};

struct nrm_control_s {
	struct nrm_control_ops *ops;
	struct nrm_control_data *data;
};

int nrm_control_getargs(nrm_control_t *control, nrm_vector_t **inputs,
			nrm_vector_t **outputs);

int nrm_control_action(nrm_control_t *control, nrm_vector_t *inputs,
		       nrm_vector_t *outputs);

int nrm_control_create(nrm_control_t **control, json_t *config);

int nrm_control_destroy(nrm_control_t **control);

/*******************************************************************************
 * Known control methods
 ******************************************************************************/

int nrm_control_europar21_create(nrm_control_t **control, json_t *config);

extern struct nrm_control_ops nrm_control_europar21_ops;

#ifdef __cplusplus
}
#endif

#endif
