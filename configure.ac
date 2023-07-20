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
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_LIBOBJ_DIR([lib])

# needed for tests
##################

AC_REQUIRE_AUX_FILE([tap-driver.sh])

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

# support for testing command line programs
AC_CHECK_PROG([BATS], [bats], [bats])
AS_IF([test "x$BATS" = "x"],[AC_MSG_ERROR(could not find bats)],[])

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
PKG_CHECK_MODULES([CHECK],[check],,
		  AC_MSG_ERROR([unable to find check]))

PKG_CHECK_MODULES([HWLOC],[hwloc >= 2.5],,
		  AC_MSG_ERROR([unable to find libhwloc]))

PKG_CHECK_MODULES([JANSSON],[jansson],,
		  AC_MSG_ERROR([unable to find jansson]))

PKG_CHECK_MODULES([LIBZMQ],[libzmq],,
		  AC_MSG_ERROR([unable to find libzmq]))

PKG_CHECK_MODULES([LIBCZMQ],[libczmq],,
		  AC_MSG_ERROR([unable to find libczmq]))

PKG_CHECK_MODULES([PAPI],[papi],,
		  AC_MSG_ERROR([unable to find papi]))

AC_CHECK_LIB(m, ceil,,[AC_MSG_ERROR([unable to find libm])])

# GEOPM
#########

AC_ARG_WITH([geopm],
	    [AS_HELP_STRING([--with-geopm],
			    [Use geopm @<:@default=no@:>@])],
	    [with_geopm=$withval], [with_geopm=no])
AS_IF([test "x$with_geopm" != "xno"],
      [
       saved_CFLAGS=$CFLAGS
       saved_LIBS=$LIBS
       CFLAGS="$CFLAGS $GEOPM_CFLAGS"
       LIBS="$LIBS $GEOPM_LIBS"
       AC_SEARCH_LIBS([geopm_pio_num_signal_name], [geopmd], [], [AC_MSG_ERROR([unable to find GEOPM])])
       AC_CHECK_HEADERS([geopm_pio.h],,[AC_MSG_ERROR([unable to find geopm_pio.h])])
       AC_CHECK_HEADERS([geopm_topo.h],,[AC_MSG_ERROR([unable to find geopm_topo.h])])
       have_geopm=1
       CFLAGS="$saved_CFLAGS"
       LIBS="$saved_LIBS"
      ],
      [
       have_geopm=0
      ]
)

AC_SUBST(GEOPM_CFLAGS)
AC_SUBST(GEOPM_LIBS)
AM_CONDITIONAL([HAVE_GEOPM],[test "$have_geopm" = "1"])
AC_DEFINE([HAVE_GEOPM],[$have_geopm], [geopm support])
AC_SUBST([HAVE_GEOPM])

# omp-tools support
####################

AC_OPENMP
save_CFLAGS="$CFLAGS"
CFLAGS="$OPENMP_CFLAGS"
AC_CHECK_HEADER([omp-tools.h],have_ompt="true",
		[AC_MSG_WARN([Can't find omp-tools.h, deactivating OMPT])
		OPENMP_CFLAGS=""
		have_ompt="false"
		])
CFLAGS="$save_CFLAGS"
AM_CONDITIONAL([HAVE_OMPT], [test "x$have_ompt" == "xtrue"])

# protobuf-c
##############

PKG_CHECK_MODULES([LIBPROTOC], [libprotobuf-c],,
		  AC_MSG_ERROR([unable to find protobuf-c]))
AC_CHECK_PROG([PROTOCC], [protoc-c], [protoc-c], [no])
AS_IF([test "x${PROTOCC}" == "xno"],
      [AC_MSG_ERROR([unable to find protoc-c])])




# options
#########

AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_FILES([Makefile
		 libnrm.pc
	 	 doc/Makefile
		 tests/cli/Makefile
		 tests/cli/bats-driver.sh
		 tests/ompt/Makefile
		 include/nrm/utils/version.h])
AC_OUTPUT

# print out what was configured
cat <<EOF
-------------------------------------------------------------------------------
LIBNRM

Version: $PACKAGE_VERSION

FEATURES:
---------

CHECK:
=======

CFLAGS:  $CHECK_CFLAGS
LDFLAGS: $CHECK_LIBS

HWLOC:
=======

CFLAGS:  $HWLOC_CFLAGS
LDFLAGS: $HWLOC_LIBS

JANSSON:
=======

CFLAGS:  $JANSSON_CFLAGS
LDFLAGS: $JANSSON_LIBS

LIBZMQ:
=======

CFLAGS:  $LIBZMQ_CFLAGS
LDFLAGS: $LIBZMQ_LIBS

LIBCZMQ:
=======

CFLAGS:  $LIBCZMQ_CFLAGS
LDFLAGS: $LIBCZMQ_LIBS

PAPI:
=======

CFLAGS:  $PAPI_CFLAGS
LDFLAGS: $PAPI_LIBS

GEOPM:
=======

Active:  $have_geopm
CFLAGS:  $GEOPM_CFLAGS
LDFLAGS: $GEOPM_LIBS

-------------------------------------------------------------------------------
EOF
