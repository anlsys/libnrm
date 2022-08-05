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

.. _clients:

Client Configuration and Reports
--------------------------------

NRM clients are used by any program that intends to communicate with a NRM daemon (``nrmd``).
Initiate most RPCs, retrieve information about the state of the daemon, can register new
elements, send events, listen to state changes.

.. doxygenfunction:: nrm_client_create

.. doxygenfunction:: nrm_client_add_scope

.. doxygenfunction:: nrm_client_add_sensor

.. doxygenfunction:: nrm_client_send_event

.. doxygenfunction:: nrm_client_destroy

.. _scopes:

Scope Configuration
-------------------

An NRM ``scope`` contains a list of resources corresponding to a *type of progress*
to be reported to NRM.

.. doxygenfile:: scopes.h
  :project: nrm

.. note::
  For example, if reporting power usage for one socket and one GPU,
  you may want to instantiate two scopes. For the first scope, while looping over corresponding
  CPU logical indexes, call ``nrm_scope_add()`` with ``NRM_SCOPE_TYPE_CPU``
  and set ``num`` to each index.

.. doxygenfunction:: nrm_client_list_scopes

.. _sensors:

Sensor Configuration
--------------------

An NRM ``sensor`` corresponds to events to be reported to NRM.

.. doxygenfunction:: nrm_sensor_create

.. doxygenfunction:: nrm_sensor_destroy

.. _logs:

Logging Interface
-----------------

.. doxygenfunction:: nrm_log_init

.. doxygenfunction:: nrm_log_setlevel

Log Levels include:

.. doxygendefine:: NRM_LOG_QUIET
.. doxygendefine:: NRM_LOG_ERROR
.. doxygendefine:: NRM_LOG_WARNING
.. doxygendefine:: NRM_LOG_NORMAL
.. doxygendefine:: NRM_LOG_INFO
.. doxygendefine:: NRM_LOG_DEBUG

.. doxygenfunction:: nrm_log_printf

.. note::

  Optionally use ``nrm_log_error()``, ``nrm_log_warning()``, ``nrm_log_normal()``, ``nrm_log_info()``, or ``nrm_log_debug()``
  in place of ``nrm_log_printf()``

.. _timers:

Timers
------

High Resolution Timers
type and functions to save a timestamp and compute a difference.
Resolution should be in nanoseconds.

.. doxygenfile:: timers.h
  :project: nrm