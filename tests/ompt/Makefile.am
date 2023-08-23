AM_COLOR_TESTS = yes

AM_CFLAGS = \
	    $(OPENMP_CFLAGS)

AM_LDFLAGS = \
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

@VALGRIND_CHECK_RULES@

TESTS = $(BATS_TESTS)
EXTRA_DIST = $(BATS_TESTS)
