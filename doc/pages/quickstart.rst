==========
Quickstart
==========

.. highlight:: bash

This document will guide you to get up
and running with running your computational jobs through the Node Resource
Manager (NRM).

Install
=======

Using Spack
-----------

Using Spack_ is recommended to install libnrm::

    $ spack install libnrm

libnrm also includes two optional utilities for power/event measurement on a variety of leadership-class systems:

  - ``nrm-papiwrapper``, requiring ``papi`` (https://github.com/icl-utk-edu/papi)
  - ``nrm-geopm``, requiring ``GEOPM`` (https://geopm.github.io/)

These can be installed as variants of libnrm::

    $ spack install libnrm ^papi
    $ spack install libnrm ^geopm mpi=False cflags="--with-geopm"

Build, install from source
--------------------------

The libnrm instrumentation library and power-measuring utilities can be downloaded
from GitHub::

  $ git clone https://github.com/anlsys/libnrm.git

Build and install::

  $ cd libnrm
  $ ./autogen.sh
  $ ./configure; make; make install

.. dropdown:: Dependency Ubuntu packages

  - ``libzmq3-dev``
  - ``libzmq5``
  - ``libczmq4``
  - ``libprotobuf-c``
  - ``protobuf-c-compiler``
  - ``libjansson-dev``
  - ``libjansson4``
  - ``libhwloc-dev``

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
of the ``client`` object from the API.

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

Included Replacement libraries
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

libnrm includes replacement libraries for MPI and openMP that have each been
instrumented with libnrm. These will be installed in ``lib`` whereever libnrm
was installed.

For example, supposing libnrm was configured via ``./configure --prefix=$PWD/build``,
then to inject libnrm's instrumented MPI::

  mpiexec -n 16 nrmc run -d $PWD/build/lib/libnrm-pmpi.so ./my_mpi_app

`nrmc listen`
-------------

Print detected event instances.

Listen to all events::

  $ nrmc listen

Listen to events acknowledged by the daemon::

  $ nrmc listen daemon.events.raw

.. _Spack: https://spack.io/