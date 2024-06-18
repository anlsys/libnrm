/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include <omp.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	int size, rank;
#pragma omp parallel
	{
		size = omp_get_num_threads();
		rank = omp_get_thread_num();
	}
	fprintf(stdout, "Hello, I'm %u of %u\n", rank, size);
	return 0;
}
