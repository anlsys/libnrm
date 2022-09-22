/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include "config.h"

#include "nrm.h"

#include "internal/nrmi.h"
#include "internal/control.h"

typedef struct europar19_data {
	nrm_vector_t *inputs;
	nrm_vector_t *outputs;
} nrm_control_europar19_data_t;

static inline double pcapL(double pcap, double a, double b, double alpha, double beta)
{
	return -exp(-alpha*(a*pcap + b - beta));
}

static inline double progressL(double progress, double Kl)
{
	return progress - Kl;
}

static inline double error(double e, double pmax, double pl)
{
	return (1.0 - e)*pmax - pl;
}

static inline double action(double Ki, double deltaTi, double Kp, double e, double prev_e, double prev_pcapL)
{
	return (Ki*deltaTi + Kp) * e - Kp *prev_e + prev_pcapL;
}

int nrm_control_europar19_create(nrm_control_t **control, json_t *config)
{
	nrm_control_t *ret;
	nrm_control_europar19_data_t *data;

	ret = NRM_INNER_MALLOC(nrm_control_t, nrm_control_europar19_data_t);
	if(ret == NULL)
		return -NRM_ENOMEM;

	data = NRM_INNER_MALLOC_GET_FIELD(ret, 2, nrm_control_t,
					  nrm_control_europar19_data_t);
	ret->data = (struct nrm_control_data *)data;
	ret->ops = &nrm_control_europar19_ops;

	nrm_vector_create(&data->inputs, sizeof(nrm_control_input_t));
	nrm_vector_create(&data->outputs, sizeof(nrm_control_output_t));
	nrm_control_input_t in;
	in.sensor_uuid = nrm_string_fromchar("nrm.sensor.rapl");
	in.scope_uuid = nrm_string_fromchar("nrm.scope.package.0");
	in.value = 0.0;
	nrm_vector_push_back(data->inputs, &in);
	in.sensor_uuid = nrm_string_fromchar("nrm.sensor.progress");
	in.scope_uuid = nrm_string_fromchar("nrm.scope.package.0");
	in.value = 0.0;
	nrm_vector_push_back(data->inputs, &in);
	nrm_control_output_t out;
	out.actuator_uuid = nrm_string_fromchar("nrm.actuator.rapl");
	out.value = 0.0;
	nrm_vector_push_back(data->outputs, &out);
	*control = ret;
	return 0;
}

int nrm_control_europar19_getargs(nrm_control_t *control, nrm_vector_t **inputs,
				  nrm_vector_t **outputs)
{
	nrm_control_europar19_data_t *data = (nrm_control_europar19_data_t *)control->data;
	*inputs = data->inputs;
	*outputs = data->outputs;
	return 0;
}

int nrm_control_europar19_action(nrm_control_t *control, nrm_vector_t *inputs,
				 nrm_vector_t *outputs)
{
	return 0;
}

int nrm_control_europar19_destroy(nrm_control_t **control)
{
	nrm_control_t *ret;
	nrm_control_europar19_data_t *data;

	if(control == NULL || *control == NULL)
		return 0;

	ret = *control;
	data = (nrm_control_europar19_data_t *)ret->data;
	nrm_vector_destroy(&data->inputs);
	nrm_vector_destroy(&data->outputs);
	free(ret);
	*control = NULL;
	return 0;
}

struct nrm_control_ops nrm_control_europar19_ops = {
	nrm_control_europar19_create,
	nrm_control_europar19_getargs,
	nrm_control_europar19_action,
	nrm_control_europar19_destroy,
};
