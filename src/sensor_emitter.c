/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

/* Description: This file contains the implementation of a sensor emitter, i.e.
 * a component that intends to send (emit) information about a sensor to the
 * NRM.
 *
 * The application context information transmitted can be used to monitor
 * application progress and/or invoke power policies to improve energy
 * efficiency at the node level.
 */
#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nrm.h"

#include "nrm-internal.h"

struct nrm_sensor_emitter_ctxt *nrm_sensor_emitter_create(void)
{
	struct nrm_sensor_emitter_ctxt *ctxt;
	ctxt = calloc(1, sizeof(struct nrm_sensor_emitter_ctxt));
	assert(ctxt != NULL);
	return ctxt;
}

int nrm_sensor_emitter_destroy(struct nrm_sensor_emitter_ctxt *ctxt)
{
	assert(ctxt != NULL);
	free(ctxt);
	return 0;
}

json_t *nrm_ssem_encode_progress(struct nrm_sensor_emitter_ctxt *ctxt,
				int64_t timestamp, unsigned long progress,
				nrm_scope_t *scope)
{
	json_t *msg;
	json_t *sc;
	sc = nrm_scope_to_json(scope);
	msg = json_pack("{s:i, s:{s:{s:i, s:{s:s, s:s, s:i, s:i, s:i},s:o}}}",
			"timestamp", timestamp,
			"info", "threadProgress", "progress", progress,
			"downstreamThreadID", "cmdID", ctxt->cmd_id,
			"taskID", ctxt->name, "processID", 0, "rankID", 0,
			"scopes", sc);
	return msg;
}

json_t *nrm_ssem_encode_pause(struct nrm_sensor_emitter_ctxt *ctxt,
			      int64_t timestamp)
{
	json_t *msg;
	msg = json_pack("{s:i, s:{s:{s:{s:s, s:s, s:i, s:i, s:i}}}}",
			"timestamp", timestamp,
			"info", "threadPause", "cmdID", ctxt->cmd_id,
			"taskID", ctxt->name, "processID", 0, "rankID", 0);
	return 0;
}


int nrm_sensor_emitter_start(struct nrm_sensor_emitter_ctxt *ctxt,
			     const char *sensor_name)
{
	assert(ctxt != NULL);

	/* env init */
	const char *uri = getenv(NRM_ENV_DOWNSTREAM_URI);
	if (uri == NULL)
		uri = NRM_DEFAULT_DOWNSTREAM_URI;

	/* context init */
	ctxt->cmd_id = getenv(NRM_ENV_DOWNSTREAM_CMDID);
	assert(ctxt->cmd_id != NULL);
	ctxt->name = (char *)sensor_name;

	/* net init */
	nrm_net_down_client_init(&ctxt->net, uri);

	return 0;
}

int nrm_sensor_emitter_exit(struct nrm_sensor_emitter_ctxt *ctxt)
{
	char *buf;
	size_t bufsize;
	nrm_time_t now;
	int64_t tm;
	json_t *msg;
	assert(ctxt != NULL);
	nrm_time_gettime(&now);
	tm = nrm_time_tons(&now);
	msg = nrm_ssem_encode_pause(ctxt, tm);
	assert(msg != NULL);
	bufsize = json_dumpb(msg, NULL, 0, 0);
	assert(bufsize != 0);
	buf = malloc(bufsize);
	assert(buf != NULL);
	int err = nrm_net_send(&ctxt->net, buf, bufsize, ZMQ_DONTWAIT);
	assert(err > 0);
	free(buf);
	json_decref(msg);
	nrm_net_fini(&ctxt->net);
	return 0;
}

int nrm_sensor_emitter_send_progress(struct nrm_sensor_emitter_ctxt *ctxt,
				     unsigned long progress, nrm_scope_t *scope)
{
	char *buf;
	size_t bufsize;
	nrm_time_t now;
	int64_t tm;
	json_t *msg;
	nrm_time_gettime(&now);
	tm = nrm_time_tons(&now);
	msg = nrm_ssem_encode_progress(ctxt, tm, progress, scope);
	bufsize = json_dumpb(msg, NULL, 0, 0);
	assert(bufsize != 0);
	buf = malloc(bufsize);
	assert(buf != NULL);
	int err = nrm_net_send(&ctxt->net, buf, bufsize, ZMQ_DONTWAIT);
	return 0;
}
