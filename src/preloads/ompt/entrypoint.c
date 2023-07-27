/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "nrm_omp.h"

static ompt_start_tool_result_t nrm_ompt_start;
ompt_set_callback_t nrm_ompt_set_callback;

nrm_client_t *global_client;
nrm_scope_t *global_scope;
nrm_sensor_t *global_sensor;
ompt_finalize_tool_t nrm_finalizer;

int find_allowed_scope(nrm_client_t *client, nrm_scope_t **scope)
{
	/* create a scope based on hwloc_allowed, althouth we know for a fact
	 * that the daemon should already have this scope.
	 */
	nrm_string_t name =
	        nrm_string_fromprintf("nrm.ompt.global.%u", getpid());
	nrm_scope_t *allowed = nrm_scope_create_hwloc_allowed(name);
	nrm_string_decref(name);

	nrm_vector_t *nrmd_scopes;
	nrm_client_list_scopes(client, &nrmd_scopes);

	size_t numscopes;
	int newscope = 0;
	nrm_vector_length(nrmd_scopes, &numscopes);
	for (size_t i = 0; i < numscopes; i++) {
		nrm_scope_t *s;
		nrm_vector_pop_back(nrmd_scopes, &s);
		if (!nrm_scope_cmp(s, allowed)) {
			nrm_scope_destroy(allowed);
			allowed = s;
			newscope = 1;
			continue;
		}
		nrm_scope_destroy(s);
	}
	if (!newscope) {
		nrm_log_error("Could not find an existing scope to match\n");
		return -NRM_EINVAL;
	}
	nrm_vector_destroy(&nrmd_scopes);
	*scope = allowed;
	return 0;
}

void nrm_ompt_atexit(void)
{
	nrm_finalizer();
}

int nrm_ompt_initialize(ompt_function_lookup_t lookup,
                        int initial_device_num,
                        ompt_data_t *tool_data)
{
	ompt_set_result_t ret;

	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "nrm-ompt");
	nrm_log_setlevel(NRM_LOG_DEBUG);
	nrm_log_debug("initialize tool\n");

	// initialize global client
	nrm_client_create(&global_client, nrm_upstream_uri,
	                  nrm_upstream_pub_port, nrm_upstream_rpc_port);

	// create global scope;
	find_allowed_scope(global_client, &global_scope);

	// global sensor
	char *name = "nrm-ompt";
	global_sensor = nrm_sensor_create(name);

	// add global scope and sensor to client, as usual
	nrm_client_add_sensor(global_client, global_sensor);

	/* use the lookup function to retrieve a function pointer to
	 * ompt_set_callback.
	 */
	nrm_ompt_set_callback =
	        (ompt_set_callback_t)lookup("ompt_set_callback");
	assert(nrm_ompt_set_callback != NULL);

	nrm_ompt_register_cbs();

	/* czmq registers an atexit function that ends up being called earlier
	 * than ompt_finalize, as OpenMP relies on return from main too to
	 * trigger it.
	 *
	 * The solution is to register an atexit after czmq, to call the
	 * finalizer before zsys_shutdown.
	 */
	nrm_finalizer = lookup("ompt_finalize_tool");
	assert(nrm_finalizer != NULL);
	atexit(nrm_ompt_atexit);

	/* spec dictates that we return non-zero to keep the tool active */
	return 1;
}

void nrm_ompt_finalize(ompt_data_t *tool_data)
{
	nrm_log_debug("finalize tool\n");
	nrm_scope_destroy(global_scope);
	nrm_client_remove_sensor(global_client, global_sensor);
	nrm_sensor_destroy(&global_sensor);
	nrm_client_destroy(&global_client);
	nrm_finalize();
	return;
}

ompt_start_tool_result_t *ompt_start_tool(unsigned int omp_version,
                                          const char *runtime_version)
{
	fprintf(stderr, "OMPT start: %u, %s\n", omp_version, runtime_version);
	return &nrm_ompt_start;
}

static ompt_start_tool_result_t nrm_ompt_start = {
        .initialize = nrm_ompt_initialize,
        .finalize = nrm_ompt_finalize,
        .tool_data = ompt_data_none,
};
