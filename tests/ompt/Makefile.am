AM_COLOR_TESTS = yes

AM_CPPFLAGS = \
	      -include $(top_builddir)/src/config.h \
	      -I$(top_srcdir)/include \
	      -I$(top_builddir)/include

AM_CFLAGS = $(LIBZMQ_CFLAGS) \
	    $(LIBCZMQ_CFLAGS) \
	    $(JANSSON_CFLAGS) \
	    $(LIBPROTOC_CFLAGS) \
	    $(HWLOC_CFLAGS) \
	    $(PAPI_CFLAGS) \
	    $(OPENMP_CFLAGS)

AM_LDFLAGS = $(LIBZMQ_LIBS) \
	     $(LIBCZMQ_LIBS) \
	     $(JANSSON_LIBS) \
	     $(LIBPROTOC_LIBS) \
	     $(HWLOC_LIBS) \
	     $(PAPI_LIBS) \
	     $(OPENMP_CFLAGS)
# can't run those tests in parallel, they launch the daemon
.NOTPARALLEL:

BATS_TESTS = \
	     ompt.bats
	     

TEST_EXTENSIONS = .bats
BATS_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
		$(top_srcdir)/build-aux/tap-driver.sh
BATS_LOG_COMPILER = $(SHELL) $(top_builddir)/tests/cli/bats-driver.sh

OMPT_BINARIES = \
		omp_basic \
		omp_stream

check_PROGRAMS = $(OMPT_BINARIES)

TESTS = $(BATS_TESTS)
EXTRA_DIST = $(BATS_TESTS)
