AM_COLOR_TESTS = yes

AM_LDFLAGS = @MPILIBS@
CC= @MPICC@

TESTS_ENVIRONMENT= LD_PRELOAD=$(top_builddir)/src/mpi/.libs/libnrm-pmpi.so

MPI_TESTS = mpi_basic mpi_collectives

check_PROGRAMS = $(MPI_TESTS)
TESTS = $(MPI_TESTS)
