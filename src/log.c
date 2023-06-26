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

#include <assert.h>
#include <stdio.h>

#include "nrm.h"

#include "internal/messages.h"
#include "internal/nrmi.h"

/*******************************************************************************
 * Logging functions
 *******************************************************************************/

int nrm_log_level;
static FILE *nrm_log_fd = NULL;
static const char *nrm_log_namespace;

static const char *nrm_log_level_string[] = {
        "quiet", "error", "warning", "normal", "info", "debug",
};

int nrm_log_init(FILE *f, const char *nm)
{
	if (f == NULL || nm == NULL)
		return -NRM_EINVAL;
	nrm_log_fd = f;
	nrm_log_level = NRM_LOG_NORMAL;
	nrm_log_namespace = nm;
	return 0;
}

int nrm_log_setlevel(int level)
{
	if (level < 0)
		level = 0;
	if (level > NRM_LOG_DEBUG)
		level = NRM_LOG_DEBUG;
	nrm_log_level = level;
	return 0;
}

void nrm_log_printf(
        int level, const char *file, unsigned int line, const char *format, ...)
{
	va_list ap;
	if (level > nrm_log_level)
		return;

	if (nrm_log_level == NRM_LOG_DEBUG)
		fprintf(nrm_log_fd, "%s:\t%s:\t%s:\t%u:\t", nrm_log_namespace,
		        nrm_log_level_string[level], file, line);
	else
		fprintf(nrm_log_fd, "%s:\t%s:\t", nrm_log_namespace,
		        nrm_log_level_string[level]);
	va_start(ap, format);
	vfprintf(nrm_log_fd, format, ap);
	va_end(ap);
}

void nrm_log_printmsg(int level, nrm_msg_t *msg)
{
	if (level > nrm_log_level)
		return;
	if (msg == NULL) {
		fprintf(nrm_log_fd, "NULL\n");
		return;
	}
	nrm_msg_fprintf(nrm_log_fd, msg);
}
