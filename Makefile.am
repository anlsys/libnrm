bin_PROGRAMS = 
check_PROGRAMS = 
TESTS = 
SUBDIRS = 
BUILT_SOURCES = 
EXTRA_DIST = 
CLEANFILES = 
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
	      -include $(top_builddir)/src/config.h \
	      -I$(top_srcdir)/include \
	      -I$(top_builddir)/include

AM_CFLAGS = $(PTHREAD_CFLAGS) \
	    $(LIBZMQ_CFLAGS) \
	    $(LIBCZMQ_CFLAGS) \
	    $(JANSSON_CFLAGS) \
	    $(LIBPROTOC_CFLAGS) \
	    $(HWLOC_CFLAGS) \
	    $(PAPI_CFLAGS) \
	    $(GEOPM_CFLAGS) \
	    $(OPENMP_CFLAGS)

AM_LDFLAGS = $(PTHREAD_LIBS) \
	     $(LIBZMQ_LIBS) \
	     $(LIBCZMQ_LIBS) \
	     $(JANSSON_LIBS) \
	     $(LIBPROTOC_LIBS) \
	     $(HWLOC_LIBS) \
	     $(GEOPM_LIBS) \
	     $(PAPI_LIBS) \
	     $(OPENMP_CFLAGS)

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

include_nrmdir=$(includedir)/nrm
include_nrm_HEADERS = \
		      include/nrm/tools.h

include_nrm_utilsdir=$(includedir)/nrm/utils
include_nrm_utils_HEADERS = \
			    include/nrm/utils/alloc.h \
			    include/nrm/utils/bitmaps.h \
			    include/nrm/utils/error.h \
			    include/nrm/utils/parsers.h \
			    include/nrm/utils/ringbuffer.h \
			    include/nrm/utils/scopes.h \
			    include/nrm/utils/strings.h \
			    include/nrm/utils/timers.h \
			    include/nrm/utils/uuids.h \
			    include/nrm/utils/vectors.h \
			    include/nrm/utils/hashes.h \
			    include/nrm/utils/variables.h \
			    include/nrm/utils/version.h

noinst_HEADERS = \
		 include/internal/actuators.h \
		 include/internal/control.h \
		 include/internal/nrmi.h \
		 include/internal/messages.h \
		 include/internal/roles.h \
		 include/internal/utarray.h \
		 include/internal/uthash.h \
		 include/internal/utlist.h \
		 include/internal/utringbuffer.h

###############################################################################
# Library
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
		    src/reactor.c \
		    src/roles/client.c \
		    src/roles/controller.c \
		    src/roles/role.c \
		    src/sensor.c \
		    src/server.c \
		    src/slices.c \
		    src/state.c \
		    src/timeserie.c \
		    src/tools.c \
		    src/actuators/continuous.c \
		    src/actuators/discrete.c \
		    src/utils/bitmaps.c \
		    src/utils/error.c \
		    src/utils/hashes.c \
		    src/utils/parsers.c \
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

if HAVE_OMPT
lib_LTLIBRARIES += libnrm-ompt.la
libnrm_ompt_la_SOURCES = \
		src/preloads/ompt/entrypoint.c \
		src/preloads/ompt/nrm_omp_callbacks.c \
		src/preloads/ompt/nrm_omp.h

libnrm_ompt_la_LDFLAGS = $(AM_LDFLAGS) -export-symbols-regex '^(ompt)_'
libnrm_ompt_la_LIBADD = libnrm.la
endif

if HAVE_PMPI
SUBDIRS += . src/preloads/pmpi
endif

dist_noinst_DATA = src/msg.proto
BUILT_SOURCES += src/msg.pb-c.h
CLEANFILES += src/msg.pb-c.c src/msg.pb-c.h

###############################################################################
# Programs
###############################################################################

nrmd_SOURCES = src/binaries/nrmd.c
nrmd_LDADD = libnrm.la

nrmc_SOURCES = src/binaries/nrmc.c
nrmc_LDADD = libnrm.la

nrm_dummy_extra_SOURCES = src/binaries/nrm-dummy-extra.c
nrm_dummy_extra_LDADD = libnrm.la

nrm_papiwrapper_SOURCES = src/binaries/nrm-papiwrapper.c
nrm_papiwrapper_LDADD = libnrm.la

if HAVE_GEOPM
nrm_geopm_SOURCES = src/binaries/nrm-geopm.c
nrm_geopm_LDADD = libnrm.la
bin_PROGRAMS += nrm-geopm
endif

bin_SCRIPTS = nrm-setup nrm-prometheus

nrm-setup: src/python/nrm-setup.py
	cp $^ $@
	chmod +x $@

nrm-prometheus: src/python/nrm-prometheus.py
	cp $^ $@
	chmod +x $@

CLEANFILES += $(bin_SCRIPTS)
EXTRA_DIST += src/python/nrm-setup.py \
	      src/python/nrm-prometheus.py

bin_PROGRAMS += nrmd nrmc nrm-dummy-extra nrm-papiwrapper

###############################################################################
# Bindings
###############################################################################

if HAVE_PYTHON
SUBDIRS += bindings/python
endif

###############################################################################
# Documentation
###############################################################################

# can't make it work nicely with readthedocs, so use a subdir
SUBDIRS += doc

EXTRA_DIST += examples

###############################################################################
# TESTS
###############################################################################

# bats tests use a different driver, so we configure them in a different subdir
SUBDIRS += tests/cli

if HAVE_OMPT
SUBDIRS += tests/ompt
endif

if HAVE_PMPI
SUBDIRS += tests/pmpi
endif

if HAVE_GEOPM
SUBDIRS += tests/geopm
endif

AM_COLOR_TESTS = yes
AM_CPPFLAGS += $(CHECK_CFLAGS)
AM_CFLAGS += $(CHECK_CFLAGS)
AM_LDFLAGS += $(CHECK_LIBS)
LDADD = libnrm.la

VALGRIND_SUPPRESSIONS_FILES = $(top_srcdir)/.valgrind.supp

# valgrind support
@VALGRIND_CHECK_RULES@

COMPILED_TESTS = \
		tests/core \
		tests/net \
		tests/eventbase \
		tests/utils/hash \
		tests/utils/vector \
		tests/utils/ringbuffer \
		tests/utils/scope \
		tests/utils/string

# unit tests
UNIT_TESTS = \
	     $(COMPILED_TESTS)

check_PROGRAMS += $(COMPILED_TESTS)
TESTS += $(UNIT_TESTS)

