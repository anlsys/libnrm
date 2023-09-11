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

typedef struct europar21_data {
	double a, b, alpha, beta, Kl, t, e, max;
	double prev_e;
	nrm_vector_t *inputs;
	nrm_vector_t *outputs;
} nrm_control_europar21_data_t;

static inline double
pcap_raw2L(double pcap, double a, double b, double alpha, double beta)
{
	return -exp(-alpha * (a * pcap + b - beta));
}

static inline double progress_raw2L(double progress, double Kl)
{
	return progress - Kl;
}

static inline double error(double e, double pmax, double pl)
{
	return (1.0 - e) * pmax - pl;
}

static inline double actionL(double Ki,
                            double deltaTi,
                            double Kp,
                            double e,
                            double prev_e,
                            double prev_pcapL)
{
	return (Ki * deltaTi + Kp) * e - Kp * prev_e + prev_pcapL;
}

static inline double action_L2raw(double actL, double alpha, double beta,
				  double a, double b)
{
	return (-log(-actL) / alpha + beta - b) / a;
}

int nrm_control_europar21_create(nrm_control_t **control, json_t *config)
{
	nrm_control_t *ret;
	nrm_control_europar21_data_t *data;

	nrm_log_debug("configuring europar21 control loop\n");
	ret = NRM_INNER_MALLOC(nrm_control_t, nrm_control_europar21_data_t);
	if (ret == NULL)
		return -NRM_ENOMEM;

	data = NRM_INNER_MALLOC_GET_FIELD(ret, 2, nrm_control_t,
	                                  nrm_control_europar21_data_t);
	ret->data = (struct nrm_control_data *)data;
	ret->ops = &nrm_control_europar21_ops;

	json_t *object = json_object_get(config, "inputs");
	assert(object != NULL);
	size_t idx;
	json_t *value;

	nrm_vector_create(&data->inputs, sizeof(nrm_control_input_t));
	json_array_foreach(object, idx, value)
	{
		char *sensor, *scope;
		int err;
		json_error_t error;
		nrm_control_input_t in;
		err = json_unpack_ex(value, &error, 0, "{s:s, s:s}", "sensor",
		                     &sensor, "scope", &scope);
		assert(!err);
		in.sensor_uuid = nrm_string_fromchar(sensor);
		in.scope_uuid = nrm_string_fromchar(scope);
		in.value = 0.0;
		nrm_vector_push_back(data->inputs, &in);
	}

	object = json_object_get(config, "outputs");
	assert(object != NULL);
	nrm_vector_create(&data->outputs, sizeof(nrm_control_output_t));
	json_array_foreach(object, idx, value)
	{
		char *actuator;
		int err;
		json_error_t error;
		nrm_control_output_t out;
		err = json_unpack_ex(value, &error, 0, "s", &actuator);
		assert(!err);
		out.actuator_uuid = nrm_string_fromchar(actuator);
		out.value = 0.0;
		nrm_vector_push_back(data->outputs, &out);
	}

	object = json_object_get(config, "args");
	assert(object != NULL);
	int err;
	json_error_t error;
	err = json_unpack_ex(object, &error, 0,
	                     "{s:f, s:f, s:f, s:f, s:f, s:f}", "a", &data->a,
	                     "b", &data->b, "alpha", &data->alpha, "beta",
	                     &data->beta, "Kl", &data->Kl, "t", &data->t, "e",
	                     &data->e, "max", &data->max);
	assert(!err);
	*control = ret;
	return 0;
}

int nrm_control_europar21_getargs(nrm_control_t *control,
                                  nrm_vector_t **inputs,
                                  nrm_vector_t **outputs)
{
	nrm_control_europar21_data_t *data =
	        (nrm_control_europar21_data_t *)control->data;
	*inputs = data->inputs;
	*outputs = data->outputs;
	return 0;
}

int nrm_control_europar21_action(nrm_control_t *control,
                                 nrm_vector_t *inputs,
                                 nrm_vector_t *outputs)
{
	nrm_control_input_t *in;
	nrm_control_output_t *out;
	double prog, pcap;
	nrm_control_europar21_data_t *data;
	data = (nrm_control_europar21_data_t *)control->data;
	nrm_vector_get_withtype(nrm_control_input_t, inputs, 0, in);
	if (in == NULL)
		return -NRM_EINVAL;

	if (in->value == 0.0)
		return 0.0;

	prog = in->value;
	nrm_vector_get_withtype(nrm_control_output_t, outputs, 0, out);
	if (out == NULL)
		return -NRM_EINVAL;
	pcap = out->value;

	nrm_log_debug("progress: %f, pcap: %f\n", prog, pcap);
	double progL, pcapL, newe, actL, act;
	progL = progress_raw2L(prog, data->Kl);
	pcapL = pcap_raw2L(pcap, data->a, data->b, data->alpha, data->beta);
	newe = error(data->e, data->max, progL);
	actL = actionL(1.0 / (data->Kl * data->t), 1, 1.0 / (data->Kl * data->t),
	             newe, data->prev_e, pcapL);
	act = action_L2raw(actL, data->alpha, data->beta, data->a, data->b);
	nrm_log_debug("new pcap: %f\n", act);
	data->prev_e = newe;
	out->value = act;
	return 0;
}

int nrm_control_europar21_destroy(nrm_control_t **control)
{
	nrm_control_t *ret;
	nrm_control_europar21_data_t *data;

	if (control == NULL || *control == NULL)
		return 0;

	ret = *control;
	data = (nrm_control_europar21_data_t *)ret->data;
	nrm_vector_destroy(&data->inputs);
	nrm_vector_destroy(&data->outputs);
	free(ret);
	*control = NULL;
	return 0;
}

struct nrm_control_ops nrm_control_europar21_ops = {
        nrm_control_europar21_create,
        nrm_control_europar21_getargs,
        nrm_control_europar21_action,
        nrm_control_europar21_destroy,
};
