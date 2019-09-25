AM_CPPFLAGS = @LIBZMQ_CFLAGS@ $(OPENMP_CFLAGS)
AM_LDFLAGS = $(OPENMP_CFLAGS)

lib_LTLIBRARIES = libnrm.la

lib_LTLIBRARIES += libf-nrm.la

if PMPI_API
lib_LTLIBRARIES += libnrm-pmpi.la
endif

libnrm_la_SOURCES = downstream_api.c nrm.h nrm_messaging.h
libnrm_la_LIBADD = @LIBZMQ_LIBS@ 


libf_nrm_la_SOURCES = downstream_api.c f_downstream_api.c nrm.h nrm_messaging.h
libf_nrm_la_LIBADD = @LIBZMQ_LIBS@

libnrm_pmpi_la_SOURCES = downstream_api.c mpi_api.c nrm.h nrm_messaging.h
libnrm_pmpi_la_LIBADD = @LIBZMQ_LIBS@

include_HEADERS = nrm_messaging.h nrm.h f_nrm.h
