ACLOCAL_AMFLAGS = -I m4

SUBDIRS = doc

AM_CPPFLAGS = @LIBZMQ_CFLAGS@ $(PTHREAD_CFLAGS)
AM_LDFLAGS = $(PTHREAD_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnrm.pc libnrmf.pc

EXTRA_DIST = autogen.sh libnrm.pc libnrmf.pc README.md

include_HEADERS = src/nrm.h src/nrmf.fh

lib_LTLIBRARIES = libnrm.la libnrmf.la

libnrm_la_SOURCES = \
		    src/downstream_api.c \
		    src/internal-downstream.h \
		    src/timers.c \
		    src/scopes.c \
		    src/bitmaps.c
libnrm_la_LIBADD = @LIBZMQ_LIBS@ $(LTLIBOBJS)

libnrmf_la_SOURCES = \
		     src/downstream_api.c \
		     src/f_downstream_api.c \
		     src/internal-downstream.h \
		     src/timers.c \
		     src/scopes.c \
		     src/bitmaps.c
libnrmf_la_LIBADD = @LIBZMQ_LIBS@ $(LTLIBOBJS)
