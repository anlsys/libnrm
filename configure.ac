# see semver.org for version info
#################################

m4_define([VERSION_STRING], m4_esyscmd([build-aux/git-version-gen .tarball-version]))
m4_define([VERSION_FIELDS], m4_split(VERSION_STRING, [\.]))
m4_define([VERSION_MAJOR], m4_argn(1, VERSION_FIELDS))
m4_define([VERSION_MINOR], m4_argn(2, VERSION_FIELDS))
m4_define([VERSION_PATCH], m4_argn(3, VERSION_FIELDS))
m4_define([VERSION_REVISION], m4_argn(4, VERSION_FIELDS))

# init
######

AC_INIT([libnrm],[0.7.0],[swann@anl.gov])
AC_CONFIG_SRCDIR([include/nrm.h])
AC_CONFIG_AUX_DIR([m4])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_LIBOBJ_DIR([lib])

# automake should fail on any error
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects 1.12])

# check for programs
####################
AC_LANG([C])
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CPP
AC_TYPE_SIZE_T
AC_TYPE_INTPTR_T
AM_PROG_AR
LT_INIT

# extra configuration
#####################

AC_SUBST([PACKAGE_VERSION_MAJOR],[VERSION_MAJOR])
AC_SUBST([PACKAGE_VERSION_MINOR],[VERSION_MINOR])
AC_SUBST([PACKAGE_VERSION_PATCH],[VERSION_PATCH])
AC_SUBST([PACKAGE_VERSION_REVISION],[VERSION_REVISION])

# support for testing with valgrind
###################################

AX_VALGRIND_DFLT([helgrind], [off])
AX_VALGRIND_DFLT([drd], [off])
AX_VALGRIND_DFLT([sgcheck], [off])
AX_VALGRIND_CHECK

# getcpu is somewhat new, and might be missing from the system libc
AC_REPLACE_FUNCS(getcpu)

# doc in m4/ax_pthread.m4. Defines automake PTHREAD_CFLAGS and PTHREAD_LIBS
AX_PTHREAD([],[AC_MSG_ERROR([Cannot find how to compile with pthreads.])])
CC="$PTHREAD_CC"

# check for libtool
LT_INIT
AC_SEARCH_LIBS([dlsym], [dl dld], [], [
  AC_MSG_ERROR([unable to find the dlsym() function])
])

# dependencies
##############
PKG_CHECK_MODULES([LIBZMQ],[libzmq],,
		  AC_MSG_ERROR([unable to find libzmq]))

PKG_CHECK_MODULES([LIBCZMQ],[libczmq],,
		  AC_MSG_ERROR([unable to find libzmq]))

PKG_CHECK_MODULES([JANSSON],[jansson],,
		  AC_MSG_ERROR([unable to find jansson]))

PKG_CHECK_MODULES([CHECK],[check],,
		  AC_MSG_ERROR([unable to find check]))


# protobuf-c
##############

PKG_CHECK_MODULES([PROTOC], [libprotobuf-c],,
		  AC_MSG_ERROR([unable to find protobuf-c]))
AC_CHECK_PROG([PROTOCC], [protoc-c], [proto-c], [no])
AS_IF([test "x${PROTOCC}" == "xno"],
      [AC_MSG_ERROR([unable to find protoc-c])])




# options
#########

AC_ARG_ENABLE([binaries],
	      [AS_HELP_STRING([--enable-binaries],
			      [build optional binaries (dev tools)])]
	      ,[binaries=true],[binaries=false])
AM_CONDITIONAL([BUILD_BINARIES],[test "x$binaries" = "xtrue"])


AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_FILES([Makefile
		 include/Makefile
		 src/Makefile
		 tests/Makefile
		 libnrm.pc
		 include/nrm/utils/version.h])
AC_OUTPUT
