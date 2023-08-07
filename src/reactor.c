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
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signalfd.h>

#include "internal/nrmi.h"

struct nrm_reactor_s {
	zloop_t *loop;
	nrm_reactor_user_callbacks_t callbacks;
};

int nrm_reactor_signal_callback(zloop_t *loop, zmq_pollitem_t *poller, void *arg)
{
	(void)loop;
	nrm_reactor_t *self = (nrm_reactor_t *)arg;
	struct signalfd_siginfo fdsi;
	ssize_t s = read(poller->fd, &fdsi, sizeof(struct signalfd_siginfo));
	assert(s == sizeof(struct signalfd_siginfo));
	int signalid = fdsi.ssi_signo;
	nrm_log_debug("caught signal %d\n", signalid);

	/* we default to exit */
	int ret = -1;
	if (self->callbacks.signal != NULL)
		ret = self->callbacks.signal(self, signalid);
	return ret;
}

int nrm_reactor_timer_callback(zloop_t *loop, int timerid, void *arg)
{
	(void)loop;
	(void)timerid;
	nrm_reactor_t *self = (nrm_reactor_t *)arg;

	int ret = 0;
	if (self->callbacks.timer != NULL)
		ret = self->callbacks.timer(self);
	return ret;
}

int nrm_reactor_create(nrm_reactor_t **reactor)
{
	if (reactor == NULL)
		return -NRM_EINVAL;

	nrm_reactor_t *ret = calloc(1, sizeof(nrm_reactor_t));
	if (ret == NULL)
		return -NRM_ENOMEM;

	ret->loop = zloop_new();
	assert(ret->loop != NULL);

	/* we always setup signal handling and controller callback */
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGINT);
	int sfd = signalfd(-1, &sigmask, 0);
	assert(sfd != -1);
	zmq_pollitem_t signal_poller = {0, sfd, ZMQ_POLLIN, 0};
	zloop_poller(ret->loop, &signal_poller, nrm_reactor_signal_callback,
	             NULL);

	*reactor = ret;
	return 0;
}

int nrm_reactor_setcallbacks(nrm_reactor_t *reactor,
                            nrm_reactor_user_callbacks_t callbacks)
{
	if (reactor == NULL)
		return -NRM_EINVAL;

	reactor->callbacks = callbacks;
	return 0;
}

int nrm_reactor_settimer(nrm_reactor_t *reactor, nrm_time_t sleeptime)
{
	if (reactor == NULL)
		return -NRM_EINVAL;

	int millisecs = sleeptime.tv_sec * 1e3 + sleeptime.tv_nsec * 1e6;
	zloop_timer(reactor->loop, millisecs, 0, nrm_reactor_timer_callback,
	            reactor);
	return 0;
}

int nrm_reactor_start(nrm_reactor_t *reactor)
{
	if (reactor == NULL)
		return -NRM_EINVAL;

	return zloop_start(reactor->loop);
}

void nrm_reactor_destroy(nrm_reactor_t **reactor)
{
	if (reactor == NULL || *reactor == NULL)
		return;
	nrm_reactor_t *r = *reactor;
	zloop_destroy(&r->loop);
	free(r);
	*reactor = NULL;
}
