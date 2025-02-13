/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#ifndef NRM_ACTUATORS_H
#define NRM_ACTUATORS_H 1

/*******************************************************************************
 * Actuators
 * Entity representing a possible action in the system.
 ******************************************************************************/

typedef struct nrm_actuator_data_s {
	size_t type;
	nrm_string_t uuid;
	nrm_uuid_t *clientid;
	double value;
	union {
		nrm_vector_t *choices;
		double limits[2];
	} u;
} nrm_actuator_data_t;

struct nrm_actuator_ops_s {
	int (*validate_value)(nrm_actuator_t *a, double value);
	void (*destroy)(nrm_actuator_t **a);
};

#define NRM_ACTUATOR_TYPE_DISCRETE 0
#define NRM_ACTUATOR_TYPE_CONTINUOUS 1
#define NRM_ACTUATOR_TYPE_MAX 2

extern struct nrm_actuator_ops_s nrm_actuator_discrete_ops;
extern struct nrm_actuator_ops_s nrm_actuator_continuous_ops;


#endif /* NRM_ACTUATORS_H */
