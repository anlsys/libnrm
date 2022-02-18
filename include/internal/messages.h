/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#ifndef LIBNRM_INTERNAL_MESSAGES_H
#define LIBNRM_INTERNAL_MESSAGES_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <zmq.h>
#include <czmq.h>
#include <jansson.h>

#include "internal/nrmi.h"

/*******************************************************************************
 * External Messages: structures filled with all the information needed to send
 * out a message to a different nrm component.
 ******************************************************************************/
enum nrm_msg_type_e {
	NRM_MSG_TYPE_SENSOR_PROGRESS = 0,
	NRM_MSG_TYPE_SENSOR_PAUSE = 1,
};

struct nrm_msg_sspg_s {
	unsigned long progress;
	nrm_scope_t *scope;
	const char *name;
	const char *cmdid;
};

struct nrm_msg_sspa_s {
	const char *name;
	const char *cmdid;
};

struct nrm_msg_s {
	int type;	
	nrm_time_t timestamp;
	union {
		struct nrm_msg_sspg_s sspg;
		struct nrm_msg_sspa_s sspa;
	} u;
};

typedef struct nrm_msg_s nrm_msg_t;

int nrm_msg_send(zsock_t *socket, nrm_msg_t *msg);

nrm_msg_t *nrm_msg_new_progress(nrm_time_t timestamp, unsigned long progress,
				nrm_scope_t *scope);

nrm_msg_t *nrm_msg_new_pause(nrm_time_t timestamp);

void nrm_msg_destroy(nrm_msg_t **msg);

/*******************************************************************************
 * Control Messages: mostly needed to exchange through shared memory between
 * various nrm layers (e.g. brokers and user-facing APIs)
 ******************************************************************************/

/* because of the way zactor behaves, we need to keep the first frame of a
 * message coming from the pipe as a string. We then convert to an integer type
 * for easier logic.
 */


#define NRM_CTRLMSG_TYPE_STRING_SEND "SEND"
#define NRM_CTRLMSG_TYPE_STRING_TERM "$TERM"

enum nrm_ctrlmsg_type_e {
	NRM_CTRLMSG_TYPE_SEND = 0,
	NRM_CTRLMSG_TYPE_TERM = 1,
	NRM_CTRLMSG_TYPE_MAX,
};

struct nrm_ctrlmsg_s {
	int type;
	struct nrm_msg_s *msg; /* almost always, a control message deals with
				  sending or receiving a real message */
};

typedef struct nrm_ctrlmsg_s nrm_ctrlmsg_t;

int nrm_ctrlmsg_send(zsock_t *socket, int type, nrm_msg_t *msg);

nrm_msg_t *nrm_ctrlmsg_recv(zsock_t *socket, int *type);

#ifdef __cplusplus
}
#endif

#endif
