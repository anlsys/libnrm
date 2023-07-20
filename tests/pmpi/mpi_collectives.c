/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include <assert.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int size, rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	double local[size];
	double recv[size];
	for (int i = 0; i < size; i++)
		local[i] = (double)rank;

	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Allreduce(local, recv, size, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

	for (int i = 0; i < size; i++)
		assert(fabs(recv[i] - ((size * (size - 1)) / 2.0)) < 1.e-14);
	MPI_Finalize();
	return 0;
}
