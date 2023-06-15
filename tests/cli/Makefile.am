AM_COLOR_TESTS = yes

# can't run those tests in parallel, they launch the daemon
.NOTPARALLEL:

BATS_TESTS = \
	     nrmc.bats \
	     nrmd.bats

TEST_EXTENSIONS = .bats
BATS_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
		$(top_srcdir)/build-aux/tap-driver.sh
BATS_LOG_COMPILER = $(SHELL) $(top_builddir)/tests/cli/bats-driver.sh

TESTS = $(BATS_TESTS)
EXTRA_DIST = $(BATS_TESTS)
