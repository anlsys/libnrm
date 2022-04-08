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

#include "msg.pb-c.h"

/* convert protobuf-c enums to more nice ones */
#define NRM_MSG_TYPE_ACK (NRM__MSGTYPE__ACK)
#define NRM_MSG_TYPE_LIST (NRM__MSGTYPE__LIST)
#define NRM_MSG_TYPE_ADD (NRM__MSGTYPE__ADD)
#define NRM_MSG_TYPE_MAX (3)

#define NRM_MSG_LIST_TYPE_SLICE (NRM__LISTTYPE__SLICE)


typedef Nrm__Slice nrm_msg_slice_t;
typedef Nrm__SliceList nrm_msg_slicelist_t;
typedef Nrm__Add nrm_msg_add_t;
typedef Nrm__List nrm_msg_list_t;
typedef Nrm__Message nrm_msg_t;

#define nrm_msg_slice_init(msg)     nrm__slice__init(msg)
#define nrm_msg_slicelist_init(msg) nrm__slice_list__init(msg)
#define nrm_msg_add_init(msg)       nrm__add__init(msg)
#define nrm_msg_list_init(msg)      nrm__list__init(msg)
#define nrm_msg_init(msg)           nrm__message__init(msg)

nrm_msg_t *nrm_msg_create(void);
void nrm_msg_destroy(nrm_msg_t **msg);

int nrm_msg_fill(nrm_msg_t *msg, int type);
int nrm_msg_set_add_slice(nrm_msg_t *msg, char *name, nrm_uuid_t *uuid);
int nrm_msg_set_list_slices(nrm_msg_t *msg, nrm_vector_t *slices);

/*******************************************************************************
 * Printing
 ******************************************************************************/

int nrm_msg_fprintf(FILE *f, nrm_msg_t *msg);

/*******************************************************************************
 * Socket Interaction
 ******************************************************************************/

int nrm_msg_send(zsock_t *socket, nrm_msg_t *msg);
int nrm_msg_sendto(zsock_t *socket, nrm_msg_t *msg, nrm_uuid_t *to);

nrm_msg_t *nrm_msg_recv(zsock_t *socket);
nrm_msg_t *nrm_msg_recvfrom(zsock_t *socket, nrm_uuid_t **from);

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
