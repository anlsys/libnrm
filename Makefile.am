bin_PROGRAMS = 
check_PROGRAMS = 
TESTS = 
BUILT_SOURCES = 
EXTRA_DIST = 
CLEANFILES = 
ACLOCAL_AMFLAGS = -I m4

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
	     $(HWLOC_LIBS)

EXTRA_DIST += autogen.sh libnrm.pc README.md LICENSE CONTRIBUTING.markdown

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnrm.pc

###############################################################################
# Version management
###############################################################################

EXTRA_DIST += $(top_srcdir)/.version
BUILT_SOURCES += $(top_srcdir)/.version
$(top_srcdir)/.version:
		echo $(VERSION) > $@-t && mv $@-t $@
dist-hook:
		echo $(VERSION) > $(distdir)/.tarball-version

###############################################################################
# Proto-c
###############################################################################

SUFFIXES = .proto .pb-c.c .pb-c.h

.proto.pb-c.c:
	$(AM_V_GEN)$(PROTOCC) "--c_out=`pwd`" -I$(top_srcdir) $<

.proto.pb-c.h:
	$(AM_V_GEN)$(PROTOCC) "--c_out=`pwd`" -I$(top_srcdir) $<

###############################################################################
# Includes
###############################################################################

include_HEADERS = include/nrm.h

include_nrm_utilsdir=$(includedir)/nrm/utils
include_nrm_utils_HEADERS = \
			    include/nrm/utils/alloc.h \
			    include/nrm/utils/bitmaps.h \
			    include/nrm/utils/error.h \
			    include/nrm/utils/ringbuffer.h \
			    include/nrm/utils/scopes.h \
			    include/nrm/utils/strings.h \
			    include/nrm/utils/timers.h \
			    include/nrm/utils/uuids.h \
			    include/nrm/utils/vectors.h \
			    include/nrm/utils/hashes.h \
			    include/nrm/utils/version.h

noinst_HEADERS = \
		 include/internal/control.h \
		 include/internal/nrmi.h \
		 include/internal/messages.h \
		 include/internal/roles.h \
		 include/internal/utarray.h \
		 include/internal/uthash.h \
		 include/internal/utlist.h \
		 include/internal/utringbuffer.h

###############################################################################
# Includes
###############################################################################

lib_LTLIBRARIES = libnrm.la

libnrm_la_SOURCES = \
		 src/actuator.c \
		 src/client.c \
		 src/eventbase.c \
		 src/net.c \
		 src/nrm.c \
		 src/messages.c \
		 src/log.c \
		 src/control/control.c \
		 src/control/europar21.c \
		 src/hwloc.c \
		 src/roles/client.c \
		 src/roles/controller.c \
		 src/roles/role.c \
		 src/slices.c \
		 src/sensor.c \
		 src/state.c \
		 src/utils/bitmaps.c \
		 src/utils/error.c \
		 src/utils/hashes.c \
		 src/utils/ringbuffer.c \
		 src/utils/scopes.c \
		 src/utils/strings.c \
		 src/utils/timers.c \
		 src/utils/uuids.c \
		 src/utils/vectors.c

nodist_libnrm_la_SOURCES = \
			   src/msg.pb-c.c \
			   src/msg.pb-c.h

libnrm_la_LIBADD = $(LTLIBOBJS)

nrmd_SOURCES = src/binaries/nrmd.c
nrmd_LDADD = libnrm.la
nrmc_SOURCES = src/binaries/nrmc.c
nrmc_LDADD = libnrm.la
nrm_dummy_extra_SOURCES = src/binaries/nrm-dummy-extra.c
nrm_dummy_extra_LDADD = libnrm.la
bin_PROGRAMS += nrmd nrmc nrm-dummy-extra

dist_noinst_DATA = src/msg.proto
BUILT_SOURCES += src/msg.pb-c.h
CLEANFILES += src/msg.pb-c.c src/msg.pb-c.h

###############################################################################
# Documentation
###############################################################################

# can't make it work nicely with readthedocs, so use a subdir
SUBDIRS = doc

###############################################################################
# TESTS
###############################################################################

# bats tests use a different driver, so we configure them in a different subdir

AM_COLOR_TESTS = yes
AM_CPPFLAGS += $(CHECK_CFLAGS)
AM_CFLAGS += $(CHECK_CFLAGS)
AM_LDFLAGS += $(CHECK_LIBS)
LDADD = libnrm.la

# valgrind support
@VALGRIND_CHECK_RULES@

COMPILED_TESTS = \
		tests/core \
		tests/net \
		tests/eventbase \
		tests/utils/hash \
		tests/utils/vector \
		tests/utils/ringbuffer \
		tests/utils/scope

BATS_TESTS = \
	     tests/cli/nrmc.bats \
	     tests/cli/nrmd.bats

TEST_EXTENSIONS = .bats
BATS_LOG_COMPILER = \
		export TOP_BUILDDIR=$(abs_top_builddir); \
		export TOP_SRCDIR=$(abs_top_srcdir); \
		$(BATS)

# unit tests
UNIT_TESTS = \
	     $(COMPILED_TESTS) \
	     $(BATS_TESTS)

check_PROGRAMS += $(COMPILED_TESTS)
TESTS += $(UNIT_TESTS)
EXTRA_DIST += $(BATS_TESTS)
