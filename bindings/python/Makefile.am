EXTRA_DIST = \
	setup.py

pkgpython_PYTHON = nrm/__init__.py \
		   nrm/base.py \
		   nrm/client.py

check: FORCE
	PYTHONPATH=$(srcdir)/ LIBNRM_SO_=$(top_builddir)/.libs/libnrm.so $(PYTHON) -m unittest discover -s $(srcdir)/tests/

FORCE:
