CC = @MPICC@
AM_CPPFLAGS = \
	      -include $(top_builddir)/src/config.h \
	      -I$(top_srcdir)/include \
	      -I$(top_builddir)/include

AM_CFLAGS = $(LIBZMQ_CFLAGS) \
	    $(LIBCZMQ_CFLAGS) \
	    $(JANSSON_CFLAGS) \
	    $(LIBPROTOC_CFLAGS) \
	    $(HWLOC_CFLAGS)

AM_LDFLAGS = $(LIBZMQ_LIBS) \
	     $(LIBCZMQ_LIBS) \
	     $(JANSSON_LIBS) \
	     $(LIBPROTOC_LIBS) \
	     $(HWLOC_LIBS) \
	     $(MPILIBS)

AM_LDFLAGS = -export-symbols-regex '^(MPI|mpi)_'

lib_LTLIBRARIES = libnrm-pmpi.la
libnrm_pmpi_la_SOURCES = \
		src/preloads/pmpi/c_mpi_bindings.c \
		src/preloads/pmpi/mpi_api.c \
		src/preloads/pmpi/nrm_mpi.h
libnrm_pmpi_la_LIBADD = libnrm.la
