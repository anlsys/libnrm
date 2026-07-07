=========================
NVIDIA GPU monitoring
=========================

.. highlight:: bash

``nrm-nvml`` is an optional sensor utility that monitors NVIDIA GPUs through the
NVIDIA Management Library (NVML) and reports per-GPU measurements to the
``nrmd`` daemon. It can also, optionally, act as an actuator to set per-GPU
power-management limits.

Requirements
============

Runtime
-------

The NVIDIA driver must be installed, providing ``libnvidia-ml.so.1``. The
``nvidia-smi`` command being functional is a good indicator that the runtime
requirements are met.

Build time
----------

The CUDA toolkit provides the ``nvml.h`` header used at compile time. NVML does
not ship a ``pkg-config`` file, so ``nrm-nvml`` is only built when libnrm is
configured with ``--with-nvml``::

  $ ./configure --with-nvml

If the CUDA toolkit is not in a default location, pass its prefix::

  $ ./configure --with-nvml=/usr/local/cuda

This adds ``-I<prefix>/include`` and ``-L<prefix>/lib64`` to the build. The
detection can also be steered manually with the ``NVML_CFLAGS`` and
``NVML_LIBS`` variables.

Usage
=====

Once ``nrmd`` is running, launch the sensor::

  $ nrm-nvml --freq 1 --event nrm.nvml.power --event nrm.nvml.temperature

If no event is requested, ``nrm.nvml.power`` is measured by default. One scope
of type GPU is created per detected device, so measurements are reported
per-GPU.

Supported events
----------------

Events are selected with ``-e``/``--event``:

.. list-table::
   :header-rows: 1
   :widths: 40 45 15

   * - Event name
     - Description
     - Unit
   * - ``nrm.nvml.power``
     - board power draw
     - Watts
   * - ``nrm.nvml.temperature``
     - GPU die temperature
     - Celsius
   * - ``nrm.nvml.gpu_utilization``
     - GPU compute utilization
     - percent
   * - ``nrm.nvml.memory_utilization``
     - GPU memory-controller utilization
     - percent
   * - ``nrm.nvml.memory_used``
     - used device memory
     - bytes

Metrics that are not supported by a particular device are skipped for that
device with a logged warning; other devices continue to report.

Power control
=============

With ``--enable-power-control``, ``nrm-nvml`` exposes one continuous actuator
per GPU that supports power management, named
``nrm.nvml.gpu.power_limit.<index>``. The actuator's bounds come from the
device's power-management limit constraints, and setting a value calls
``nvmlDeviceSetPowerManagementLimit``.

Setting power limits typically requires elevated privileges. Enabling
persistence mode is recommended::

  $ sudo nvidia-smi -pm 1
  $ sudo nrm-nvml --enable-power-control

The actuator can then be driven through the usual client interface, e.g. with
``nrmc actuate``.
