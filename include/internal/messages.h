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

#include "nrm.h"
#include "internal/nrmi.h"

/*******************************************************************************
 * External Messages: structures filled with all the information needed to send
 * out a message to a different nrm component.
 ******************************************************************************/

enum nrm_msg_type_e {
	NRM_MSG_TYPE_SENSOR_PROGRESS = 0,
	NRM_MSG_TYPE_SENSOR_PAUSE = 1,
	NRM_MSG_TYPE_REQ_LIST = 2,
	NRM_MSG_TYPE_REP_LIST = 3,
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

enum nrm_msg_req_type_e {
	NRM_MSG_REQ_TARGET_SLICES = 0,
};

struct nrm_msg_reql_s {
	int target;
};

struct nrm_msg_repl_s {
	int target;
	nrm_vector_t *items;
};

struct nrm_msg_s {
	int type;	
	nrm_time_t timestamp;
	union {
		struct nrm_msg_sspg_s sspg;
		struct nrm_msg_sspa_s sspa;
		struct nrm_msg_reql_s reql;
		struct nrm_msg_repl_s repl;
	} u;
};

int nrm_msg_send(zsock_t *socket, nrm_msg_t *msg);

nrm_msg_t *nrm_msg_recv(zsock_t *socket, int *msg_type);

nrm_msg_t *nrm_msg_recvfrom(zsock_t *socket, int *msg_type, nrm_uuid_t **from);

/*******************************************************************************
 * Control Messages: mostly needed to exchange through shared memory between
 * various nrm layers (e.g. brokers and user-facing APIs)
 ******************************************************************************/

/* because of the way zactor behaves, we need to keep the first frame of a
 * message coming from the pipe as a string. We then convert to an integer type
 * for easier logic.
 */


#define NRM_CTRLMSG_TYPE_STRING_TERM "$TERM"
#define NRM_CTRLMSG_TYPE_STRING_SEND "SEND"
#define NRM_CTRLMSG_TYPE_STRING_RECV "RECV"
#define NRM_CTRLMSG_TYPE_STRING_PUB "PUB"
#define NRM_CTRLMSG_TYPE_STRING_SUB "SUB"

enum nrm_ctrlmsg_type_e {
	NRM_CTRLMSG_TYPE_TERM = 0,
	NRM_CTRLMSG_TYPE_SEND = 1,
	NRM_CTRLMSG_TYPE_RECV = 2,
	NRM_CTRLMSG_TYPE_PUB = 3,
	NRM_CTRLMSG_TYPE_SUB = 4,
	NRM_CTRLMSG_TYPE_MAX,
};

struct nrm_ctrlmsg_s {
	int type;
	struct nrm_msg_s *msg; /* almost always, a control message deals with
				  sending or receiving a real message */
	nrm_uuid_t *uuid; /* need to keep track of where those messages come
			     from */
};

typedef struct nrm_ctrlmsg_s nrm_ctrlmsg_t;

int nrm_ctrlmsg_send(zsock_t *socket, int type, nrm_msg_t *msg, nrm_uuid_t *to);

nrm_msg_t *nrm_ctrlmsg_recv(zsock_t *socket, int *type, nrm_uuid_t **from);

#ifdef __cplusplus
}
#endif

#endif
