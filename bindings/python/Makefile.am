EXTRA_DIST = \
	nrm/__init__.py \
	nrm/bindings.py \
	nrm/client.py \
	setup.py

python_PYTHON = setup.py

check: FORCE
	PYTHONPATH=$(srcdir)/ LIBNRM_SO_=$(libdir).so python3 -m unittest discover -s $(srcdir)/tests/

FORCE:
