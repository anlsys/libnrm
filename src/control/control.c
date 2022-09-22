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

struct nrm_control_method {
	const char *name;
	int (*create)(nrm_control_t **, json_t *);
};

struct nrm_control_method nrm_control_methods[] = {
	{"europar19", nrm_control_europar19_create},
	{0,0},
};

int nrm_control_create(nrm_control_t **control, json_t *config)
{
	json_t *jname;
	char *name;
	jname = json_object_get(config, "name");	
	int err;
	err = json_unpack(jname, "s", &name);
	assert(!err);
	for(int i = 0; nrm_control_methods[i].name != NULL; i++) {
		if(!strcmp(name, nrm_control_methods[i].name))
			return nrm_control_methods[i].create(control, config);
	}
	return -NRM_EINVAL;
}

int nrm_control_getargs(nrm_control_t *control, nrm_vector_t **inputs,
			nrm_vector_t **outputs)
{
	return control->ops->getargs(control, inputs, outputs);
}

int nrm_control_action(nrm_control_t *control, nrm_vector_t *inputs,
		       nrm_vector_t *outputs)
{
	return control->ops->action(control, inputs, outputs);
}

int nrm_control_destroy(nrm_control_t **control)
{
	return (*control)->ops->destroy(control);
}
