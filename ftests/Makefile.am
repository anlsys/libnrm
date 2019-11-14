AM_COLOR_TESTS = yes

AM_DEFAULT_SOURCE_EXT = .f90

TESTS_ENVIRONMENT= LD_PRELOAD=$(top_srcdir)/src/.libs/libnrm-pmpi.so \
		   NRM_ENV_CMDID=argo \
		   ARGO_NRM_TRANSMIT=0

MPI_TESTS = mpi_basic mpi_collectives

check_PROGRAMS = $(MPI_TESTS)
TESTS = $(MPI_TESTS) 
