/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************
 *
 *    this file is generated, modifications will be erased.
*/


#define NRM_DEFAULT_URI "ipc:///tmp/nrm-downstream-event"
#define NRM_ENV_URI "NRM_DOWNSTREAM_EVENT_URI"
#define NRM_ENV_CMDID "NRM_CMDID"
#define NRM_ENV_RATELIMIT "NRM_RATELIMIT"
#define NRM_ENV_TRANSMIT "NRM_TRANSMIT"
#define NRM_DEFAULT_RATELIMIT_THRESHOLD (10000000LL)

#define NRM_CMDPERFORMANCE_FORMAT "{\"cmdPerformance\":{\"cmdID\":\"%s\",\"perf\":%d}}"
#define NRM_CMDPAUSE_FORMAT "{\"cmdPause\":{\"cmdID\":\"%s\"}}"
#define NRM_THREADPROGRESS_FORMAT "{\"threadProgress\":{\"progress\":%d,\"downstreamThreadID\":{\"cmdID\":\"%s\",\"taskID\":\"%s\",\"processID\":%d,\"rankID\":%d,\"threadID\":%d}}}"
#define NRM_THREADPAUSE_FORMAT "{\"threadPause\":{\"downstreamThreadID\":{\"cmdID\":\"%s\",\"taskID\":\"%s\",\"processID\":%d,\"rankID\":%d,\"threadID\":%d}}}"
#define NRM_THREADPHASECONTEXT_FORMAT "{\"threadPhaseContext\":{\"downstreamThreadID\":{\"cmdID\":\"%s\",\"taskID\":\"%s\",\"processID\":%d,\"rankID\":%d,\"threadID\":%d},\"phaseContext\":{\"computetime\":%d,\"aggregation\":%d,\"totaltime\":%d,\"cpu\":%d}}}"
#define NRM_THREADPHASEPAUSE_FORMAT "{\"threadPhasePause\":{\"downstreamThreadID\":{\"cmdID\":\"%s\",\"taskID\":\"%s\",\"processID\":%d,\"rankID\":%d,\"threadID\":%d}}}"