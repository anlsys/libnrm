# see semver.org for version info
AC_INIT([libnrm],[0.0.1],[sriduttb@anl.gov])


# are we in the right source dir ?
AC_CONFIG_SRCDIR([src/nrm.h])

# build artefacts in separate dir
AC_CONFIG_AUX_DIR([m4])
AC_CONFIG_MACRO_DIR([m4])

# automake should fail on any error
AM_INIT_AUTOMAKE([-Wall -Werror foreign 1.12])
AM_PROG_AR

# check for libtool
LT_INIT

# check for programs
AC_LANG([C])
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_CPP
AC_TYPE_SIZE_T
AC_TYPE_INTPTR_T

# dependencies
PKG_CHECK_MODULES([LIBZMQ],[libzmq])

# PMPI lib
AC_ARG_ENABLE(pmpi,
	      [AS_HELP_STRING([--enable-pmpi],[Build PMPI library.])],
	      [pmpi=true],[pmpi=false])
if [[ "x$pmpi" = xtrue ]]; then
	AX_MPI([],[AC_MSG_ERROR(could not find mpi library for --enable-pmpi)])
fi
AM_CONDITIONAL([PMPI_API],[test "x$pmpi" = xtrue])

AC_CONFIG_HEADERS([src/config.h])

CFLAGS="$CFLAGS -Werror"

# OpenMP
AC_OPENMP

AC_CONFIG_FILES([Makefile src/Makefile libnrm.pc libf_nrm.pc])
AC_OUTPUT
