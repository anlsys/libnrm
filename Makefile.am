ACLOCAL_AMFLAGS = -I m4
SUBDIRS = include src tests

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnrm.pc

EXTRA_DIST = autogen.sh libnrm.pc README.md .version

BUILT_SOURCES = $(top_srcdir)/.version
$(top_srcdir)/.version:
		echo $(VERSION) > $@-t && mv $@-t $@
dist-hook:
		echo $(VERSION) > $(distdir)/.tarball-version
