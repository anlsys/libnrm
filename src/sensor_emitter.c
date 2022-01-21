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

int nrm_sensor_emitter_encode_progress(struct nrm_sensor_emitter_ctxt *ctxt,
				       int64_t timestamp,
				       char *buf, size_t bufsize)
{
	json_t *msg;
	size_t written;
	msg = json_pack("{s:i, s:{s:{s:i, s:{s:s, s:s, s:i, s:i, s:i}}}}",
			"timestamp", timestamp,
			"info", "threadProgress", "progress", ctxt->acc,
			"downstreamThreadID", "cmdID", ctxt->cmd_id,
			"taskID", ctxt->name, "processID", 0, "rankID", 0);
	assert(msg != NULL);
	written = json_dumpb(msg, buf, bufsize, JSON_COMPACT);
	assert(written <= bufsize);
	json_decref(msg);
	return 0;
}

int nrm_sensor_emitter_encode_pause(struct nrm_sensor_emitter_ctxt *ctxt,
				    int64_t timestamp,
				    char *buf, size_t bufsize)
{
	json_t *msg;
	size_t written;
	msg = json_pack("{s:i, s:{s:{s:{s:s, s:s, s:i, s:i, s:i}}}}",
			"timestamp", timestamp,
			"info", "threadPause", "cmdID", ctxt->cmd_id,
			"taskID", ctxt->name, "processID", 0, "rankID", 0);
	assert(msg != NULL);
	written = json_dumpb(msg, buf, bufsize, JSON_COMPACT);
	assert(written <= bufsize);
	json_decref(msg);
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

	/* app init */
	nrm_time_gettime(&ctxt->time);
	ctxt->acc = 0;
	return 0;
}

int nrm_sensor_emitter_fini(struct nrm_sensor_emitter_ctxt *ctxt)
{
	char buf[512];
	int err;
	nrm_time_t now;
	int64_t tm;
	assert(ctxt != NULL);
	nrm_time_gettime(&now);
	tm = nrm_time_tons(&now);
	if (ctxt->acc != 0) {
		nrm_sensor_emitter_encode_progress(ctxt, tm, buf, 512);
		err = nrm_net_send(&ctxt->net, buf, 512, 0);
	}
	nrm_sensor_emitter_encode_pause(ctxt, tm, buf, 512);
	err = nrm_net_send(&ctxt->net, buf, 512, 0);
	assert(err > 0);
	nrm_net_fini(&ctxt->net);
	return 0;
}

int nrm_sensor_emitter_send_progress(struct nrm_sensor_emitter_ctxt *ctxt,
				     unsigned long progress)
{
	char buf[512];
	nrm_time_t now;
	int64_t tm;
	nrm_time_gettime(&now);
	tm = nrm_time_tons(&now);
	int64_t timediff = nrm_time_diff(&ctxt->time, &now);
	ctxt->acc += progress;
	if (timediff > nrm_ratelimit_threshold) {
		nrm_sensor_emitter_encode_progress(ctxt, tm, buf, 512);
		int err = nrm_net_send(&ctxt->net, buf, 512, ZMQ_DONTWAIT);
		if (err == -1) {
			assert(errno == EAGAIN);
			/* send would block, so act like a ratelimit */
		} else {
			assert(err > 0);
			ctxt->acc = 0;
			ctxt->time = now;
		}
	}
	return 0;
}
