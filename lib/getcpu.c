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

#include <sched.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef HAVE_GETCPU
int getcpu(unsigned int *cpu, unsigned int *node)
{
	/* the syscall has an extra unused parameter */
	return syscall(SYS_getcpu, cpu, node, NULL);
}
#endif
