EXTRA_DIST = \
	tests/test_client.py \
	tests/test_init.py \
	setup.py

pkgpython_PYTHON = nrm/__init__.py \
		   nrm/base.py \
		   nrm/client.py

check: FORCE
	ABS_TOP_BUILDDIR=$(abs_top_builddir) PYTHONPATH=$(srcdir)/ LIBNRM_SO_=$(top_builddir)/.libs/libnrm.so $(PYTHON) -m unittest discover -s $(srcdir)/tests/

FORCE:
