/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#ifndef LIBNRM_INTERNAL_MSG_H
#define LIBNRM_INTERNAL_MSG_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "msg-pbc.h"

/* convert protobuf-c enums to more nice ones */
#define MSG_TYPE_ACK (NRM__MSGTYPE__ACK)
#define MSG_TYPE_LIST (NRM__MSGTYPE__LIST)

typedef Nrm__Slice nrm_msg_slice_t;
typedef Nrm__SliceList nrm_msg_slicelist_t;
typedef Nrm__List nrm_msg_list_t;
typedef Nrm__Message nrm_msg_t;

#define nrm_msg_init(msg) nrm__message__init(msg)

nrm_msg_t *nrm_msg_create(void);
void nrm_msg_destroy(nrm_msg_t **msg);



#ifdef __cplusplus
}
#endif

#endif
