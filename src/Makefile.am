AM_CPPFLAGS = -I$(top_srcdir)/include @LIBZMQ_CFLAGS@ @LIBCZMQ_CFLAGS@ @JANSSON_CFLAGS@
AM_LDFLAGS = @LIBZMQ_LIBS@ @LIBCZMQ_LIBS@ @JANSSON_LIBS@

# valgrind support
@VALGRIND_CHECK_RULES@

lib_LTLIBRARIES = libnrm.la

libnrm_la_SOURCES = \
		 net.c \
		 nrm.c \
		 messages.c \
		 roles/client.c \
		 roles/controller.c \
		 roles/monitor.c \
		 roles/role.c \
		 roles/sensor.c \
		 slices.c \
		 utils/bitmaps.c \
		 utils/scopes.c \
		 utils/strings.c \
		 utils/timers.c \
		 utils/uuids.c
libnrm_la_LIBADD = $(LTLIBOBJS)

if BUILD_BINARIES
nrmd_shim_SOURCES = binaries/shim.c
nrmd_shim_LDADD = libnrm.la
nrmc_SOURCES = binaries/nrmc.c
nrmc_LDADD = libnrm.la
bin_PROGRAMS = nrmd-shim nrmc
endif
