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
Initiate most RPCs, retrieve information about the state of the daemon, register new
elements, send events, listen to state changes.

libnrm also features a command-line :ref:`nrmc<nrmc>` utility that mirrors most client functionality.

.. doxygenfunction:: nrm_client_create

.. doxygenfunction:: nrm_client_find

.. doxygenfunction:: nrm_client_send_event

.. doxygenfunction:: nrm_client_destroy

.. tab-set::

  .. tab-item:: Scopes

    .. doxygenfunction:: nrm_client_add_scope

    .. doxygenfunction:: nrm_client_list_scopes

    .. doxygenfunction:: nrm_client_remove_scope

  .. tab-item:: Sensors

    .. doxygenfunction:: nrm_client_add_sensor

    .. doxygenfunction:: nrm_client_list_sensors

    .. doxygenfunction:: nrm_client_remove_sensor

  .. tab-item:: Actuators

    .. doxygenfunction:: nrm_client_add_actuator

    .. doxygenfunction:: nrm_client_list_actuators

    .. doxygenfunction:: nrm_client_remove_actuator

    .. doxygenfunction:: nrm_client_actuate

  .. tab-item:: Slices

    .. doxygenfunction:: nrm_client_add_slice

    .. doxygenfunction:: nrm_client_list_slices

    .. doxygenfunction:: nrm_client_remove_slice

  .. tab-item:: Callbacks

    .. doxygenfunction:: nrm_client_set_event_listener

    .. doxygenfunction:: nrm_client_start_event_listener

    .. doxygenfunction:: nrm_client_set_actuate_listener

    .. doxygenfunction:: nrm_client_start_actuate_listener

.. _scopes:

Scope Configuration
-------------------

An NRM ``scope`` contains a list of resources corresponding to a *type of progress*
to be reported to NRM. Types of progress reports include:

.. doxygendefine:: NRM_SCOPE_TYPE_CPU
.. doxygendefine:: NRM_SCOPE_TYPE_NUMA
.. doxygendefine:: NRM_SCOPE_TYPE_GPU

**Functions**

.. doxygenfunction:: nrm_scope_create

.. doxygenfunction:: nrm_scope_add

.. doxygenfunction:: nrm_scope_add_atomic

.. note::
  For example, if reporting power usage for one socket and one GPU,
  you may want to instantiate two scopes. For the first scope, while looping over corresponding
  CPU logical indexes, call ``nrm_scope_add()`` with ``NRM_SCOPE_TYPE_CPU``
  and set ``num`` to each index.

.. doxygenfunction:: nrm_scope_length

.. doxygenfunction:: nrm_scope_destroy

.. doxygenfunction:: nrm_scope_dup

.. doxygenfunction:: nrm_scope_cmp

.. doxygenfunction:: nrm_scope_snprintf

.. doxygenfunction:: nrm_scope_threadshared

.. doxygenfunction:: nrm_scope_threadprivate

.. _sensors:

Sensor Configuration
--------------------

An NRM ``sensor`` corresponds to events to be reported to NRM.

.. doxygenfunction:: nrm_sensor_create

.. doxygenfunction:: nrm_sensor_destroy

Slice Configuration
-------------------

An NRM ``slice`` names and assigns a uuid to a set of resources.

.. doxygenfunction:: nrm_slice_create

.. doxygenfunction:: nrm_slice_destroy

.. doxygenfunction:: nrm_slice_fprintf

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

**Functions**

.. doxygenfunction:: nrm_log_printf

.. note::

  Optionally use ``nrm_log_error()``, ``nrm_log_warning()``, ``nrm_log_normal()``, ``nrm_log_info()``, or ``nrm_log_debug()``
  in place of ``nrm_log_printf()`` for sensible defaults:

.. _timers:

Timers
------

High Resolution Timers
type and functions to save a timestamp and compute a difference.
Resolution should be in nanoseconds.

.. doxygentypedef:: nrm_time_t

.. doxygenfunction:: nrm_time_gettime

.. doxygenfunction:: nrm_time_diff

.. doxygenfunction:: nrm_time_tons

.. doxygenfunction:: nrm_time_fromns