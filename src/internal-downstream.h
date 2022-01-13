/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#ifndef NRM_INTERNAL_DOWNSTREAM_H
#define NRM_INTERNAL_DOWNSTREAM_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <zmq.h>

#define NRM_DEFAULT_URI "ipc:///tmp/nrm-downstream-event"
#define NRM_ENV_URI "NRM_DOWNSTREAM_EVENT_URI"
#define NRM_ENV_CMDID "NRM_CMDID"
#define NRM_ENV_RATELIMIT "NRM_RATELIMIT"
#define NRM_ENV_TRANSMIT "NRM_TRANSMIT"
#define NRM_DEFAULT_RATELIMIT_THRESHOLD (10000000LL)

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

/* min time in nsec between messages: necessary for rate-limiting progress
 * report. For now, 10ms is the threashold. */

struct nrm_context {
	void *context;
	void *socket;
	char *task_id;
	char *cmd_id;
	int rank_id;
	int thread_id;
	pid_t process_id;
	nrm_time_t time;
	unsigned long acc;
	pthread_mutex_t lock;
};

#ifdef __cplusplus
}
#endif

#endif
