AM_CPPFLAGS = -I$(top_srcdir)/include @LIBZMQ_CFLAGS@ @LIBCZMQ_CFLAGS@ @JANSSON_CFLAGS@
AM_LDFLAGS = @LIBZMQ_LIBS@ @LIBCZMQ_LIBS@ @JANSSON_LIBS@

# valgrind support
@VALGRIND_CHECK_RULES@

lib_LTLIBRARIES = libnrm.la

%.pb-c.c %.pc-c-h: %.proto
	$(PROTOCC) --c_out=$(builddir) $^

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
		 utils/error.c \
		 utils/scopes.c \
		 utils/strings.c \
		 utils/timers.c \
		 utils/uuids.c \
		 utils/vectors.c
nodist_libnrm_la_SOURCES = $(builddir)/msg.pb-c.c $(builddir)/msg.pb-c.h
libnrm_la_LIBADD = $(LTLIBOBJS)

if BUILD_BINARIES
nrmd_shim_SOURCES = binaries/shim.c
nrmd_shim_LDADD = libnrm.la
nrmc_SOURCES = binaries/nrmc.c
nrmc_LDADD = libnrm.la
bin_PROGRAMS = nrmd-shim nrmc
endif

dist_noinst_DATA = msg.proto
BUILT_SOURCES = msg.pb-c.h
MOSTLYCLEANFILES = msg.pb-c.c msg.pb-c.h
