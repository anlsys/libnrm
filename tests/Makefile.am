AM_COLOR_TESTS = yes

AM_CFLAGS = -I$(top_srcdir)/include @LIBZMQ_CFLAGS@ @JANSSON_CFLAGS@ @CHECK_CFLAGS@
AM_LDFLAGS = ../src/libnrm.la @LIBZMQ_LIBS@ @JANSSON_LIBS@ @CHECK_LIBS@

# valgrind support
@VALGRIND_CHECK_RULES@

GENERAL_TESTS = \
		core \
		net \
		utils/scope

# unit tests
UNIT_TESTS = \
	     $(GENERAL_TESTS)

check_PROGRAMS = $(UNIT_TESTS)
TESTS = $(UNIT_TESTS)
