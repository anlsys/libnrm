/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *******************************************************************************/

#include "config.h"

#include "nrm.h"

#include "internal/nrmi.h"

void nrm_time_gettime(nrm_time_t *now)
{
	clock_gettime(CLOCK_REALTIME, now);
}

int64_t nrm_time_diff(const nrm_time_t *start, const nrm_time_t *end)
{
	int64_t timediff = (end->tv_nsec - start->tv_nsec) +
	                   1000000000 * (end->tv_sec - start->tv_sec);
	return timediff;
}

int64_t nrm_time_tons(const nrm_time_t *now)
{
	return now->tv_nsec + 1000000000 * now->tv_sec;
}

nrm_time_t nrm_time_fromfreq(double freq)
{
	int64_t sleeptime = 1e9 / freq;
	return nrm_time_fromns(sleeptime);
}

nrm_time_t nrm_time_fromns(int64_t ns)
{
	nrm_time_t ret;
	ret.tv_sec = ns / 1000000000;
	ret.tv_nsec = ns % 1000000000;
	return ret;
}

json_t *nrm_time_to_json(nrm_time_t *time)
{
	/* jansson doesn't support unsigned longs, so we end up doing the
	 * conversion ourselves.
	 */
	int64_t ns = nrm_time_tons(time);

	/* snprintf can be used to figure out how much characters will be
	 * written.
	 */
	size_t bufsize;
	char *buf;
	bufsize = snprintf(NULL, 0, "%" PRId64 "", ns);
	assert(bufsize > 0);
	bufsize++;
	buf = calloc(1, bufsize);
	assert(buf != NULL);
	snprintf(buf, bufsize, "%" PRId64 "", ns);

	json_t *msg;
	msg = json_pack("s", buf);
	assert(msg != NULL);
	free(buf);
	return msg;
}

int nrm_time_from_json(nrm_time_t *time, json_t *json)
{
	char *s;
	int64_t ns;
	int err;
	err = json_unpack(json, "s", &s);
	if (err)
		return -1;
	err = sscanf(s, "%" PRId64 "", &ns);
	assert(err == 1);
	*time = nrm_time_fromns(ns);
	return 0;
}
