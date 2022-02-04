/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

/* Description: This file contains the implementation of a sensor receiver
 * a component that intends to receive sensor information from other sensors. It
 * is usually a component of the NRM daemon.
 *
 */
#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nrm.h"

#include "nrm-internal.h"

struct nrm_sensor_receiver_ctxt *nrm_sensor_receiver_create(void)
{
	struct nrm_sensor_receiver_ctxt *ctxt;
	ctxt = calloc(1, sizeof(struct nrm_sensor_receiver_ctxt));
	assert(ctxt != NULL);
	return ctxt;
}

int nrm_sensor_receiver_destroy(struct nrm_sensor_receiver_ctxt *ctxt)
{
	assert(ctxt != NULL);
	free(ctxt);
	return 0;
}

//json_t *nrm_sser_decode_progress(struct nrm_sensor_receiver_ctxt *ctxt,
//				 int64_t timestamp, unsigned long progress,
//				 nrm_scope_t *scope)
//{
//	json_t *msg;
//	json_t *sc;
//	sc = nrm_scope_to_json(scope);
//	msg = json_pack("{s:i, s:{s:{s:i, s:{s:s, s:s, s:i, s:i, s:i},s:o}}}",
//			"timestamp", timestamp,
//			"info", "threadProgress", "progress", progress,
//			"downstreamThreadID", "cmdID", ctxt->cmd_id,
//			"taskID", ctxt->name, "processID", 0, "rankID", 0,
//			"scopes", sc);
//	return msg;
//}
//
//json_t *nrm_sser_decode_pause(struct nrm_sensor_receiver_ctxt *ctxt,
//			      int64_t timestamp)
//{
//	json_t *msg;
//	msg = json_pack("{s:i, s:{s:{s:{s:s, s:s, s:i, s:i, s:i}}}}",
//			"timestamp", timestamp,
//			"info", "threadPause", "cmdID", ctxt->cmd_id,
//			"taskID", ctxt->name, "processID", 0, "rankID", 0);
//	return 0;
//}


int nrm_sensor_receiver_start(struct nrm_sensor_receiver_ctxt *ctxt)
{
	assert(ctxt != NULL);

	/* env init */
	const char *uri = getenv(NRM_ENV_DOWNSTREAM_URI);
	if (uri == NULL)
		uri = NRM_DEFAULT_DOWNSTREAM_URI;

	/* net init */
	nrm_net_down_server_init(&ctxt->net, uri);

	return 0;
}

int nrm_sensor_receiver_exit(struct nrm_sensor_receiver_ctxt *ctxt)
{
	nrm_net_fini(&ctxt->net);
	return 0;
}

int nrm_sensor_receiver_recv(struct nrm_sensor_receiver_ctxt *ctxt,
			     char **identity, char **buf)
{
	nrm_net_recv_multipart(&ctxt->net, identity, buf);
	return 0;
}
