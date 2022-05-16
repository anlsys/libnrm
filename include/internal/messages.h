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

/* convert protobuf-c enum values to more nice ones */
typedef enum _Nrm__MSGTYPE nrm_msg_msgtype_e;
#define NRM_MSG_TYPE_ACK (NRM__MSGTYPE__ACK)
#define NRM_MSG_TYPE_LIST (NRM__MSGTYPE__LIST)
#define NRM_MSG_TYPE_ADD (NRM__MSGTYPE__ADD)
#define NRM_MSG_TYPE_REMOVE (NRM__MSGTYPE__REMOVE)
#define NRM_MSG_TYPE_EVENT (NRM__MSGTYPE__EVENT)
#define NRM_MSG_TYPE_MAX (5)

typedef enum _Nrm__TARGETTYPE nrm_msg_targettype_e;
#define NRM_MSG_TARGET_TYPE_SLICE (NRM__TARGETTYPE__SLICE)
#define NRM_MSG_TARGET_TYPE_SENSOR (NRM__TARGETTYPE__SENSOR)
#define NRM_MSG_TARGET_TYPE_SCOPE (NRM__TARGETTYPE__SCOPE)
#define NRM_MSG_TARGET_TYPE_MAX (3)


typedef Nrm__Add nrm_msg_add_t;
typedef Nrm__Event nrm_msg_event_t;
typedef Nrm__List nrm_msg_list_t;
typedef Nrm__Remove nrm_msg_remove_t;
typedef Nrm__Scope nrm_msg_scope_t;
typedef Nrm__ScopeList nrm_msg_scopelist_t;
typedef Nrm__Sensor nrm_msg_sensor_t;
typedef Nrm__SensorList nrm_msg_sensorlist_t;
typedef Nrm__Slice nrm_msg_slice_t;
typedef Nrm__SliceList nrm_msg_slicelist_t;

#define nrm_msg_add_init(msg)        nrm__add__init(msg)
#define nrm_msg_event_init(msg)      nrm__event__init(msg)
#define nrm_msg_init(msg)            nrm__message__init(msg)
#define nrm_msg_list_init(msg)       nrm__list__init(msg)
#define nrm_msg_remove_init(msg)     nrm__remove__init(msg)
#define nrm_msg_scope_init(msg)      nrm__scope__init(msg)
#define nrm_msg_scopelist_init(msg)  nrm__scope_list__init(msg)
#define nrm_msg_sensor_init(msg)     nrm__sensor__init(msg)
#define nrm_msg_sensorlist_init(msg) nrm__sensor_list__init(msg)
#define nrm_msg_slice_init(msg)      nrm__slice__init(msg)
#define nrm_msg_slicelist_init(msg)  nrm__slice_list__init(msg)

nrm_msg_t *nrm_msg_create(void);
void nrm_msg_destroy(nrm_msg_t **msg);

int nrm_msg_fill(nrm_msg_t *msg, int type);
int nrm_msg_set_event(nrm_msg_t *msg, nrm_time_t time, nrm_uuid_t *uuid, nrm_scope_t *scope, double value);
int nrm_msg_set_add_scope(nrm_msg_t *msg, nrm_scope_t *scope);
int nrm_msg_set_add_sensor(nrm_msg_t *msg, char *name, nrm_uuid_t *uuid);
int nrm_msg_set_add_slice(nrm_msg_t *msg, char *name, nrm_uuid_t *uuid);
int nrm_msg_set_list_scopes(nrm_msg_t *msg, nrm_vector_t *scopes);
int nrm_msg_set_list_sensors(nrm_msg_t *msg, nrm_vector_t *sensors);
int nrm_msg_set_list_slices(nrm_msg_t *msg, nrm_vector_t *slices);
int nrm_msg_set_remove(nrm_msg_t *msg, int type, nrm_uuid_t *uuid);

nrm_scope_t *nrm_scope_create_frommsg(nrm_msg_scope_t *msg);
nrm_slice_t *nrm_slice_create_frommsg(nrm_msg_slice_t *msg);
nrm_sensor_t *nrm_sensor_create_frommsg(nrm_msg_sensor_t *msg);
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

int nrm_msg_pub(zsock_t *socket, nrm_string_t topic, nrm_msg_t *msg);
nrm_msg_t *nrm_msg_sub(zsock_t *socket, nrm_string_t *topic);
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

int nrm_ctrlmsg__send(zsock_t *socket, int type, void *, void *);
int nrm_ctrlmsg__recv(zsock_t *socket, int *type, void **, void **);

int nrm_ctrlmsg_sendmsg(zsock_t *socket, int type, nrm_msg_t *msg, nrm_uuid_t *to);
nrm_msg_t *nrm_ctrlmsg_recvmsg(zsock_t *socket, int *type, nrm_uuid_t **from);
int nrm_ctrlmsg_pub(zsock_t *socket, int type, nrm_string_t topic, nrm_msg_t *msg);
int nrm_ctrlmsg_sub(zsock_t *socket, int type, nrm_string_t topic);

#define NRM_CTRLMSG_2SEND(p,q,m) do { m = (nrm_msg_t *)p; } while(0)
#define NRM_CTRLMSG_2SENDTO(p,q,m,t) do { m = (nrm_msg_t *)p; t = (nrm_uuid_t *)q; } while(0)
#define NRM_CTRLMSG_2SUB(p,q,s) do { s = (nrm_string_t)p; } while(0)
#define NRM_CTRLMSG_2PUB(p,q,s,m) do { s = (nrm_string_t)p; m = (nrm_msg_t *)q; } while(0)

#ifdef __cplusplus
}
#endif

#endif
