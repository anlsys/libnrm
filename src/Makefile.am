AM_CPPFLAGS = -I$(top_srcdir)/include @LIBZMQ_CFLAGS@ @JANSSON_CFLAGS@
AM_LDFLAGS = @LIBZMQ_LIBS@ @JANSSON_LIBS@

# valgrind support
@VALGRIND_CHECK_RULES@

lib_LTLIBRARIES = libnrm.la

libnrm_la_SOURCES = \
		 nrm.c \
		 net.c
		 downstream_api.c \
		 internal-downstream.h \
		 utils/bitmaps.c \
		 utils/scopes.c \
		 utils/timers.c
libnrm_la_LIBADD = $(LTLIBOBJS)
