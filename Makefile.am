ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = @LIBZMQ_CFLAGS@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnrm.pc libnrmf.pc

EXTRA_DIST = autogen.sh libnrm.pc libnrmf.pc README.md

include_HEADERS = src/nrm.h src/nrmf.h

lib_LTLIBRARIES = libnrm.la libnrmf.la

libnrm_la_SOURCES = src/downstream_api.c src/internal-downstream.h src/timers.c
libnrm_la_LIBADD = @LIBZMQ_LIBS@

libnrmf_la_SOURCES = src/downstream_api.c src/f_downstream_api.c  src/internal-downstream.h src/timers.c
libnrmf_la_LIBADD = @LIBZMQ_LIBS@


