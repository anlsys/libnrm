AM_CPPFLAGS = -I$(top_srcdir)/include @LIBZMQ_CFLAGS@ @LIBCZMQ_CFLAGS@ @JANSSON_CFLAGS@
AM_LDFLAGS = @LIBZMQ_LIBS@ @LIBCZMQ_LIBS@ @JANSSON_LIBS@

# valgrind support
@VALGRIND_CHECK_RULES@

lib_LTLIBRARIES = libnrm.la

libnrm_la_SOURCES = \
		 nrm.c \
		 roles/sensor.c \
		 roles/monitor.c \
		 messages.c \
		 utils/bitmaps.c \
		 utils/scopes.c \
		 utils/timers.c
libnrm_la_LIBADD = $(LTLIBOBJS)

if BUILD_BINARIES
nrmd_shim_SOURCES = binaries/shim.c
nrmd_shim_LDADD = libnrm.la
bin_PROGRAMS = nrmd-shim
endif
