AM_COLOR_TESTS = yes

if PMPI_API
TESTS_ENVIRONMENT= LD_PRELOAD=$(top_srcdir)/src/.libs/libnrm-pmpi.so \
		   ARGO_CONTAINER_UUID=argo \
		   ARGO_NRM_TRANSMIT=0
MPI_TESTS = mpi_basic mpi_collectives
endif

check_PROGRAMS = $(MPI_TESTS)
TESTS = $(MPI_TESTS) 
