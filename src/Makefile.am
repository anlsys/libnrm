AM_CPPFLAGS = \
	      -I$(top_srcdir)/include \
	      -I$(top_srcdir)/src \
	      @LIBZMQ_CFLAGS@ \
	      @LIBCZMQ_CFLAGS@ \
	      @JANSSON_CFLAGS@ \
	      @LIBPROTOC_CFLAGS@

AM_LDFLAGS = @LIBZMQ_LIBS@ \
	     @LIBCZMQ_LIBS@ \
	     @JANSSON_LIBS@ \
	     @LIBPROTOC_LIBS@

# valgrind support
@VALGRIND_CHECK_RULES@

lib_LTLIBRARIES = libnrm.la

SUFFIXES = .proto .pb-c.c .pb-c.h

%.pb-c.c %.pb-c.h: %.proto
	$(AM_V_GEN)$(PROTOCC) --c_out=$(abs_srcdir) $^

libnrm_la_SOURCES = \
		 client.c \
		 eventbase.c \
		 net.c \
		 nrm.c \
		 messages.c \
		 log.c \
		 roles/client.c \
		 roles/controller.c \
		 roles/monitor.c \
		 roles/role.c \
		 roles/sensor.c \
		 slices.c \
		 sensor.c \
		 state.c \
		 utils/bitmaps.c \
		 utils/error.c \
		 utils/ringbuffer.c \
		 utils/scopes.c \
		 utils/strings.c \
		 utils/timers.c \
		 utils/uuids.c \
		 utils/vectors.c

nodist_libnrm_la_SOURCES = \
			   msg.pb-c.c \
			   msg.pb-c.h

libnrm_la_LIBADD = $(LTLIBOBJS)

if BUILD_BINARIES
nrmd_SOURCES = binaries/nrmd.c
nrmd_LDADD = libnrm.la
nrmc_SOURCES = binaries/nrmc.c
nrmc_LDADD = libnrm.la
bin_PROGRAMS = nrmd nrmc
endif

dist_noinst_DATA = msg.proto
BUILT_SOURCES = msg.pb-c.h
CLEANFILES = msg.pb-c.c msg.pb-c.h
