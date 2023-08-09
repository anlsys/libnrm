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
	sigset_t oldmask;
	nrm_reactor_user_callbacks_t callbacks;
};

int nrm_reactor_signal_callback(zloop_t *loop,
                                zmq_pollitem_t *poller,
                                void *arg)
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
		ret = self->callbacks.signal(self, fdsi);
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

int nrm_reactor_create(nrm_reactor_t **reactor, sigset_t *sigmask)
{
	int err;

	if (reactor == NULL)
		return -NRM_EINVAL;

	nrm_reactor_t *ret = calloc(1, sizeof(nrm_reactor_t));
	if (ret == NULL)
		return -NRM_ENOMEM;

	ret->loop = zloop_new();
	if (ret->loop == NULL) {
		err = -errno;
		goto err_reactor;
	}

	sigset_t dmask;
	if (sigmask == NULL) {
		sigemptyset(&dmask);
		sigaddset(&dmask, SIGINT);
		sigaddset(&dmask, SIGTERM);
		sigmask = &dmask;
	}

	/* we always setup signal handling */
	err = pthread_sigmask(SIG_BLOCK, sigmask, &ret->oldmask);
	if (err == -1) {
		err = -errno;
		goto err_zloop;
	}

	int sfd = signalfd(-1, sigmask, 0);
	if (sfd == -1) {
		err = -errno;
		goto err_mask;
	}

	zmq_pollitem_t signal_poller = {0, sfd, ZMQ_POLLIN, 0};
	zloop_poller(ret->loop, &signal_poller, nrm_reactor_signal_callback,
	             ret);

	*reactor = ret;
	return 0;
err_mask:
	pthread_sigmask(SIG_SETMASK, &ret->oldmask, NULL);
err_zloop:
	zloop_destroy(&ret->loop);
err_reactor:
	free(ret);
	return err;
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

	int millisecs = sleeptime.tv_sec * 1000 + sleeptime.tv_nsec / 1000000;
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
	pthread_sigmask(SIG_SETMASK, &r->oldmask, NULL);
	free(r);
	*reactor = NULL;
}
