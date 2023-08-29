AM_CPPFLAGS = \
	      -include $(top_builddir)/src/config.h \
	      -I$(top_srcdir)/include \
	      -I$(top_builddir)/include
AM_LDFLAGS = -export-symbols-regex '^(MPI|mpi)_' @MPILIBS@
CC=@MPICC@

lib_LTLIBRARIES = libnrm-pmpi.la
libnrm_pmpi_la_SOURCES = c_mpi_bindings.c \
			 mpi_api.c \
			 nrm_mpi.h
libnrm_pmpi_la_LIBADD = $(top_builddir)/libnrm.la
