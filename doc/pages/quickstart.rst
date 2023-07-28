==========
Quickstart
==========

.. highlight:: bash

Welcome to the quickstart guide for NRM. This document will guide you to get up
and running with running your computational jobs through the Node Resource
Manager (NRM).

Install
=======

Required Dependencies
---------------------

- ``libzmq3-dev``
- ``libzmq5``
- ``libczmq4``
- ``libprotobuf-c``
- ``protobuf-c-compiler``
- ``libjansson-dev``
- ``libjansson4``
- ``libhwloc-dev``

Obtain, build, install
----------------------

The libnrm instrumentation library and power-measuring utilities can be downloaded
from GitHub::

  $ git clone https://github.com/anlsys/libnrm.git

Build and install::

  $ cd libnrm
  $ ./autogen.sh
  $ ./configure; make; make install

Optional Dependencies
---------------------

Power measurement:

- ``libpapi-dev`` - (https://github.com/icl-utk-edu/papi) Build ``nrm-papiwrapper``
- ``geopm`` and ``geopm-service`` - (https://geopm.github.io/) Specify ``--with-geopm`` when configuring libnrm to build  ``nrm-geopm``

Container piece
---------------

The NRM code now supports mapping slices on both Singularity containers and
NodeOS compute containers.

Singularity
^^^^^^^^^^^

For local singularity installation, refer to the Singularity_ installation
page.


Using Spack
^^^^^^^^^^^

::

    $ spack install libnrm

Setup: Launching the `nrmd` daemon
==================================

::

    $ nrmd

NRM's behavior and resource arbitration is controlled by the ``nrmd`` userspace daemon.
It should be launched in the background.

The daemon logs its output to ``stdout`` by
default and optionally accepts a ``.json`` config file as the first positional argument.

::

    Usage: nrmd [options]

    Allowed options:
    --help, -h       : print this help message
    --version, -V     : print program version
    --verbose, -v     : increase verbosity
    --quiet, -q      : no log output
    --log-level, -l <int> : set log level (0-5)
    --uri, -u <str>    : daemon socket uri to connect to
    --rpc-port, -r <uint> : daemon rpc port to use
    --pub-port, -p <uint> : daemon pub/sub port to use

.. _nrmc:

`nrmc` Client utility
=====================

NRM features a ``nrmc`` command-line utility that largely mirrors the capabilities
of the ``client`` object from the library.

::

    Usage: nrmc [options]

    Allowed options:
    --help, -h             : print this help message
    --version, -V          : print program version
    --verbose, -v          : increase verbosity
    --quiet, -q            : no log output
    --log-level, -l <int>  : set log level (0-5)
    --uri, -u <str>        : daemon socket uri to connect to
    --rpc-port, -r  <uint> : daemon rpc port to use
    --pub-port, -p  <uint> : daemon pub/sub port to use

    Available Commands:
    actuate
    add-scope
    add-slice
    add-sensor
    find-actuator
    find-scope
    find-slice
    find-sensor
    listen
    list-actuators
    list-scopes
    list-slices
    list-sensors
    send-event
    remove-scope
    remove-slice
    remove-sensor
    run
    exit

`nrmc run`
----------

Run a command. Optionally inject an additional/replacement library via `-d`, e.g.::

  $ nrmc run -d /path/to/libnrm-ompt.so ./my_omp_app

`nrmc listen`
-------------

Print detected event instances.::

  $ nrmc listen CPU_ENERGY

.. _Singularity: https://singularity.lbl.gov/install-request
.. _GitHub: https://github.com/anlsys/nrm-core/releases
