/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#ifndef LIBNRM_INTERNAL_H
#define LIBNRM_INTERNAL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <zmq.h>
#include <jansson.h>

/*******************************************************************************
 * Compat functions
 ******************************************************************************/

#ifndef HAVE_GETCPU
int getcpu(unsigned int *cpu, unsigned int *node);
#endif

/*******************************************************************************
 * LIBNRM Defaults Values
 ******************************************************************************/

/** default URI for the NRM zmq socket for the downstream API */
#define NRM_DEFAULT_DOWNSTREAM_URI "ipc:///tmp/nrm-downstream-event"

/** default ratelimit threshold to avoid overflowing the socket, as an
 * interval between two messages in nanoseconds */
#define NRM_DEFAULT_RATELIMIT_THRESHOLD (10000000LL)

/*******************************************************************************
 * LIBNRM Environmnent variables
 ******************************************************************************/

/** env variable to change the downstream URI **/
#define NRM_ENV_DOWNSTREAM_URI "NRM_DOWNSTREAM_EVENT_URI"

/** env variable for downstream client uuid */
#define NRM_ENV_DOWNSTREAM_CMDID "NRM_CMDID"

/** env variable for ratelimit on messages */
#define NRM_ENV_RATELIMIT "NRM_RATELIMIT"

/** env variable for disabling message transmission (if 0, the library will not
 * open sockets or send messages
 */
#define NRM_ENV_TRANSMIT "NRM_TRANSMIT"

/*******************************************************************************
 * Global variables
 ******************************************************************************/

/* post initialization value for ratelimit threshold */
extern long long int nrm_ratelimit_threshold;
/* post initialization value for transmit */
extern int nrm_transmit;
/* post initialization value for downstream uri */
extern const char *nrm_downstream_uri;

/*******************************************************************************
 * NRM NETCODE
 ******************************************************************************/

struct nrm_net_ctxt {
	void *context;
	void *socket;
};

/* initializes a net context for a downstream client (sending messages to the
 * NRM on the downstream socket).
 * Will not return until the connection is established.
 */
int nrm_net_down_client_init(struct nrm_net_ctxt *ctxt, const char *uri);

/* initializes a net context for a downstream server (recv messages to the
 * NRM on the downstream socket).
 */
int nrm_net_down_server_init(struct nrm_net_ctxt *ctxt, const char *uri);

/* closes a net context.
 * Does not destroy the pointer. */
int nrm_net_fini(struct nrm_net_ctxt *ctxt);

/* sends a raw buffer */
int nrm_net_send(struct nrm_net_ctxt *ctxt, char *buf, size_t bufsize, int
		 flags);

/* recvs a raw buffer */
int nrm_net_recv_multipart(struct nrm_net_ctxt *ctxt, char **identity,
			   char **buf);
/*******************************************************************************
 * NRM SENSOR EMITTER
 ******************************************************************************/

struct nrm_sensor_emitter_ctxt {
	struct nrm_net_ctxt net;
	char *name;
	char *cmd_id;
};

json_t *nrm_time_to_json(nrm_time_t *t);

json_t *nrm_scope_to_json(nrm_scope_t *s);

json_t *nrm_bitmap_to_json(nrm_bitmap_t *b);

#define NRM_CMDPERFORMANCE_FORMAT \
	"{\"timestamp\": %" PRId64 "," \
	" \"info\":" \
	"{\"cmdPerformance\":{\"cmdID\":\"%s\",\"perf\":%lu}}" \
	"}"
#define NRM_CMDPAUSE_FORMAT \
	"{\"timestamp\": %" PRId64 "," \
	" \"info\":" \
	"{\"cmdPause\":{\"cmdID\":\"%s\"}}" \
	"}"
#define NRM_THREADPROGRESS_FORMAT \
	"{\"timestamp\": %" PRId64 "," \
	" \"info\":" \
	"{\"threadProgress\":{\"progress\":%lu,\"downstreamThreadID\":{\"cmdID\":\"%s\",\"taskID\":\"%s\",\"processID\":%d,\"rankID\":%d,\"threadID\":%d}, \"scopes\":%s}}" \
	"}"
#define NRM_THREADPAUSE_FORMAT                                                 \
	"{\"timestamp\": %" PRId64 "," \
	" \"info\":" \
	"{\"threadPause\":{\"downstreamThreadID\":{\"cmdID\":\"%s\",\"taskID\":\"%s\",\"processID\":%d,\"rankID\":%d,\"threadID\":%d}}}" \
	"}"

/*******************************************************************************
 * NRM SENSOR EMITTER
 ******************************************************************************/

struct nrm_sensor_receiver_ctxt {
	struct nrm_net_ctxt net;
};


#ifdef __cplusplus
}
#endif

#endif
