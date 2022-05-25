DOXYGEN_BUILD_DIR=$(srcdir)/build-doxygen
SPHINX_BUILD_DIR=$(srcdir)/build-sphinx

$(SPHINX_BUILD_DIR): nrm.doxy conf.py
	$(DOXYGEN) nrm.doxy
	$(SPHINXBUILD) -v -W -b html -a . $(SPHINX_BUILD_DIR)

if BUILD_DOCS

build-docs: $(SPHINX_BUILD_DIR)

install-doc-html: $(SPHINX_BUILD_DIR)
	$(MKDIR_P) $(docdir)/html
	cp -r $(SPHINX_BUILD_DIR)/. $(docdir)/html
else

build-docs:
	@echo "not building documentation"

install-doc-html:

endif

html-local: build-docs

install-html-local: html-local install-doc-html

install-data-local: install-html-local

clean-local:
	rm -rf $(DOXYGEN_BUILD_DIR) $(SPHINX_BUILD_DIR)

EXTRA_DIST= nrm.doxy conf.py index.rst
