AM_CPPFLAGS = @LIBZMQ_CFLAGS@

lib_LTLIBRARIES = libnrm.la

lib_LTLIBRARIES += libf-nrm.la

lib_LTLIBRARIES += libnrm-pmpi.la

libnrm_la_SOURCES = downstream_api.c nrm.h nrm_messaging.h
libnrm_la_LIBADD = @LIBZMQ_LIBS@

libf_nrm_la_SOURCES = downstream_api.c f_downstream_api.c nrm.h nrm_messaging.h
libf_nrm_la_LIBADD = @LIBZMQ_LIBS@

libnrm_pmpi_la_SOURCES = downstream_api.c \
			 mpi_api.c \
			 c_mpi_bindings.c \
			 f_mpi_bindings.c \
			 nrm.h \
			 nrm_mpi.h \
			 nrm_messaging.h

libnrm_pmpi_la_LIBADD = @LIBZMQ_LIBS@
libnrm_pmpi_la_LDFLAGS = -export-symbols-regex '^(MPI|mpi)_'

include_HEADERS = nrm_messaging.h nrm.h f_nrm.h
