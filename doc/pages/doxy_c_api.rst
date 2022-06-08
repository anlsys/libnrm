libnrm API
==========
.. highlight:: C

The following describes the C API for libnrm instrumentation.
The corresponding header file can be found in ``src/nrm.h``. See
:doc:`the Examples<simple_c_example>` for code samples and more information.

Library Initialization
----------------------

.. doxygenfunction:: nrm_init

.. doxygenfunction:: nrm_finalize

Scope Configuration
-------------------

An NRM ``scope`` contains a list of resources corresponding to a *type of progress*
to be reported to NRM.

.. doxygenfunction:: nrm_scope_create

.. doxygenfunction:: nrm_scope_add

.. doxygenfunction:: nrm_scope_add_atomic

.. note::
  For example, if reporting power usage for one socket and one GPU,
  you may want to instantiate two scopes. For the first scope, while looping over corresponding
  CPU logical indexes, call ``nrm_scope_add()`` with the ``NRM_SCOPE_TYPE_CPU`` type
  and set ``num`` to each index.

.. doxygenfunction:: nrm_scope_length

.. doxygenfunction:: nrm_scope_destroy

.. doxygenfunction:: nrm_scope_snprintf

.. doxygenfunction:: nrm_scope_threadshared

.. doxygenfunction:: nrm_scope_threadprivate
