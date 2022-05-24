# see semver.org for version info
AC_INIT([libnrm],[0.7.0],[swann@anl.gov])


# are we in the right source dir ?
AC_CONFIG_SRCDIR([src/nrm.h])

# build artefacts in separate dir
AC_CONFIG_AUX_DIR([m4])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_LIBOBJ_DIR([lib])

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

# getcpu is somewhat new, and might be missing from the system libc
AC_REPLACE_FUNCS(getcpu)

# check doxygen + sphinx for documentation build

AC_ARG_ENABLE(docs,
[AS_HELP_STRING([--enable-docs],
		[Generate full html documentation (default is no).])],
[docs=true],[docs=false])

if test "x$docs" = "xtrue"; then
	AC_CHECK_PROG([DOXYGEN], [doxygen], [doxygen], [no])
	if test "x$DOXYGEN" = "xno"; then
		AC_MSG_ERROR([Doxygen not found])
	fi

	AC_CHECK_PROG([SPHINXBUILD], [sphinx-build], [sphinx-build], [no])
	if test "x$SPHINXBUILD" = "xno"; then
		AC_MSG_ERROR([Sphinx not found])
	fi
fi
AM_CONDITIONAL([BUILD_DOCS],[ test "x$docs" = "xtrue" ])

# dependencies
PKG_CHECK_MODULES([LIBZMQ],[libzmq])

# doc in m4/ax_pthread.m4. Defines automake PTHREAD_CFLAGS and PTHREAD_LIBS
AX_PTHREAD([],[AC_MSG_ERROR([Cannot find how to compile with pthreads.])])
CC="$PTHREAD_CC"

# check for libtool
LT_INIT
AC_SEARCH_LIBS([dlsym], [dl dld], [], [
	AC_MSG_ERROR([unable to find the dlsym() function])
])

AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_FILES([Makefile
		 doc/Makefile
		 libnrm.pc
		 libnrmf.pc])
AC_OUTPUT
