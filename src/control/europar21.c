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
	int active;
	double a, b, alpha, beta, Kl, t, e, tobj;
	double pcap_max;
	double pcap_min_L, pcap_max_L;
	double prev_e;
	nrm_vector_t *inputs;
	nrm_vector_t *outputs;
} nrm_control_europar21_data_t;

static inline double
linearize(double value, double a, double b, double alpha, double beta)
{
	return -exp(-alpha * (a * value + b - beta));
}

static inline double error(double e, double Kl, double pcap_max_L, double prog)
{
	return e * Kl * (1 + pcap_max_L) - prog;
}

static inline double thresholding(double val, double min, double max)
{
	return fmax(fmin(val, max), min);
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

static inline double
delinearize(double value, double a, double b, double alpha, double beta)
{
	return (-log(-value) / alpha + beta - b) / a;
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

	json_t *object = json_object_get(config, "active");
	assert(object != NULL);
	json_unpack(object, "b", &data->active);

	object = json_object_get(config, "inputs");
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
		in.events = NULL;
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
	                     "{s:f, s:f, s:f, s:f, s:f, s:f, s:f, s:f, s:f, s:f}",
	                     "a", &data->a, "b", &data->b, "alpha",
	                     &data->alpha, "beta", &data->beta, "Kl", &data->Kl,
	                     "t", &data->t, "e", &data->e, "tobj", &data->tobj,
			     "min", &data->pcap_min_L, "max", &data->pcap_max);
	data->pcap_min_L = linearize(data->pcap_min_L, data->a, data->b, data->alpha, data->beta);
	data->pcap_max_L = linearize(data->pcap_max, data->a, data->b, data->alpha, data->beta);
	nrm_log_debug("europar21 params: %f %f %f %f %f %f %f %f %f %f\n", data->a, data->b, data->alpha,
		      data->beta, data->Kl, data->t, data->e, data->tobj, data->pcap_min_L, data->pcap_max_L);
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

double nrm_control_europar21_events2progress(nrm_vector_t *events)
{
	nrm_vector_t *diffs;
	size_t numevents;
	nrm_event_t *e, *prev_e;
	nrm_vector_create(&diffs, sizeof(double));
	nrm_vector_length(events, &numevents);
	nrm_vector_get_withtype(nrm_event_t, events, 0, prev_e);
	for (size_t i = 1; i < numevents; i++) {
		nrm_vector_get_withtype(nrm_event_t, events, i, e);
		int64_t delta = nrm_time_diff(&prev_e->time, &e->time);
		double val = e->value;
		double nv = val / (delta * 1e-9);
		nrm_vector_push_back(diffs, &nv);
		prev_e = e;
		nrm_log_debug("progress: event: %f %f %f\n", val, delta * 1e-9, nv);
	}

	nrm_log_debug("progress: will use %zu freqs for median\n",
	              numevents - 1);
	double *a, *b, median;
	nrm_vector_sort(diffs, nrm_vector_sort_double_cmp);
	if ((numevents - 1) % 2 == 1) {
		nrm_vector_get_withtype(double, diffs, (numevents - 1) / 2, a);
		median = *a;
		nrm_log_debug("median odd: %f %f\n", median, *a);
	} else {
		nrm_vector_get_withtype(double, diffs, (numevents - 1) / 2 - 1,
		                        a);
		nrm_vector_get_withtype(double, diffs, (numevents - 1) / 2, b);
		median = (*a + *b) / 2.0;
		nrm_log_debug("median even: %f %f %f\n", median, *a, *b);
	}
	nrm_vector_destroy(&diffs);
	return median;
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

	size_t numevents;
	nrm_vector_length(in->events, &numevents);
	if (in->events == NULL || numevents <= 1)
		return 0;

	prog = nrm_control_europar21_events2progress(in->events);
	nrm_vector_get_withtype(nrm_control_output_t, outputs, 0, out);
	if (out == NULL)
		return -NRM_EINVAL;
	pcap = out->value;

	nrm_log_normal("progress: %f pcap: %f\n", prog, pcap);
	if (data->active) {
		/* sanity check, if prog is not even remotely close to Kl, we disable
		 * control and use max power */
		if (prog < (1.0-data->e)*data->Kl || prog > 2.0*data->Kl) {
			out->value = data->pcap_max;
			data->prev_e = 0;
		} else {
			double pcapL, newe, actL, threshold, act;
			pcapL = linearize(pcap, data->a, data->b, data->alpha,
					  data->beta);
			newe = error(data->e, data->Kl, data->pcap_max_L, prog);
			actL = actionL(1.0 / (data->Kl * data->tobj/3), 1,
				       data->t / (data->Kl * data->tobj), newe,
				       data->prev_e, pcapL);
			threshold = thresholding(actL, data->pcap_min_L, data->pcap_max_L);
			act = delinearize(threshold, data->a, data->b, data->alpha,
					  data->beta);
			data->prev_e = newe;
			out->value = act;
			nrm_log_debug("model: %f %f %f %f %f\n", pcapL, newe, act, threshold, actL);
		}	
	}
	nrm_log_debug("new pcap: %f\n", out->value);
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
