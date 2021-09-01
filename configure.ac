# see semver.org for version info
AC_INIT([libnrm],[0.7.0],[swann@anl.gov])


# are we in the right source dir ?
AC_CONFIG_SRCDIR([src/nrm.h])

# build artefacts in separate dir
AC_CONFIG_AUX_DIR([m4])
AC_CONFIG_MACRO_DIR([m4])

# automake should fail on any error
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects 1.12])

# check for programs
AC_LANG([C])
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CPP
AC_TYPE_SIZE_T
AC_TYPE_INTPTR_T
AC_PROG_FC
AC_FC_WRAPPERS
AM_PROG_AR

# dependencies
PKG_CHECK_MODULES([LIBZMQ],[libzmq])

# check for libtool
LT_INIT
AC_SEARCH_LIBS([dlsym], [dl dld], [], [
  AC_MSG_ERROR([unable to find the dlsym() function])
])

AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_FILES([Makefile
		 libnrm.pc
		 libnrmf.pc])
AC_OUTPUT
