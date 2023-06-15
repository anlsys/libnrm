==========
Quickstart
==========

.. highlight:: bash

Welcome to the quickstart guide for NRM. This document will guide you to get up
and running with running your computational jobs through the Node Resource
Manager (NRM).

Install
=======

Container piece
---------------

The NRM code now supports mapping slices on both Singularity containers and
NodeOS compute containers.

Singularity
^^^^^^^^^^^

For local singularity installation, refer to the Singularity_ installation
page.

NRM
---

The NRM core library must be installed first. Binary releases are available on GitHub_.
If installing via this method, ensure that ``LD_LIBRARY_PATH`` points to the directory
that contains the extracted shared library files. Otherwise, set the ``NRMSO``
and ``PYNRMSO`` environment variables like the following::

    tar xf nrm-core-v0.7.0-x86_64-linux.tar.gz
    export NRMSO=${PWD}/nrm-core-v0.7.0-x86_64-linux/lib/libnrm-core.so
    export PYNRMSO=${PWD}/nrm-core-v0.7.0-x86_64-linux/lib/libnrm-core-python.so

The ``nrm`` client is available in ``nrm-core-v0.7.0-x86_64-linux/bin``.

Alternatively, the NRM core components (the ``nrmd`` daemon and ``nrm`` client)
can be installed in other ways:

Using Spack
^^^^^^^^^^^

The various components of NRM are available on Spack. The ``nrm-core`` package
contains the core library and ``py-nrm`` contains the Python interface. The ``nrm``
package contains documentation and example configuration scripts, and also installs
the other components::


    spack install nrm-core
    spack install py-nrm

or::

    spack install nrm

Using Pip
^^^^^^^^^

After obtaining NRM-Core, you should be able to get the NRM Python interface
and most of its dependencies on any machine with::

 pip install https://github.com/anlsys/nrm-python.git

Setup: Launching the `nrmd` daemon
==================================

NRM's behavior is controlled by the ``nrmd`` userspace daemon.  The ``nrmd`` daemon
should be launched by the application framework in the background and manages
the resource arbitration on the machine.

The daemon is launched via ``nrmd`` and logs its output to ``/tmp/nrm_log`` by
default.

``nrmd`` command-line options
-----------------------------

The ``nrmd`` daemon is mainly configured
through its command-line options::

    Usage: nrmd [-i|--stdin] [CONFIG] [-y|--yaml]
      NRM Daemon

    Available options:
      -h,--help                Show this help text
      -i,--stdin               Read configuration on stdin.
      CONFIG                   Input configuration with .yml/.yaml/.dh/.dhall
                               extension. Leave void for stdin (dhall) input.
      -y,--yaml                Assume configuration to be yaml instead of dhall.
      -h,--help                Show this help text

Running jobs using `nrm`
========================

Tasks are configured using a config file called a *manifest* and started using the ``nrm``
command-line utility. Here's an example manifest that allocates two CPUS and
enables application progress monitoring with a one-second rate limit::

  name: basic
  version: 0.0.1
  app:
    container:
      cpus: 2
      mems: 1
    perfwrapper: true
    monitoring:
      ratelimit: 1000000000

This manifest can be used in the following way to launch a command::

 $ nrm run /path/to/manifest.yaml echo "foobar"
 foobar
 INFO:nrm:process ended: msg_up_rpc_rep_process_exit(api=u'up_rpc_rep', container_uuid=u'b54f12ed-6418-4b32-b6ab-2dda7503a1c8', status=u'0', type=u'process_exit')
 INFO:nrm:command ended: msg_up_rpc_rep_process_exit(api=u'up_rpc_rep', container_uuid=u'b54f12ed-6418-4b32-b6ab-2dda7503a1c8', status=u'0', type=u'process_exit')

You have run your first nrm-enabled command. See the
:doc:`configuration guide <config>` for an in-depth
description of the manifest file format.

``nrm`` command-line options
----------------------------

The ``nrm`` command-line client can be used for a number of operations::

  usage: nrm [-h] [-v] {run,kill,list,listen,setpower} ...

  positional arguments:
    {run,kill,list,listen,setpower}

  optional arguments:
    -h, --help            show this help message and exit
    -v, --verbose         verbose logging information

Start containerized tasks, using a container specification we refer to as an application *manifest*::

  usage: nrm run [-h] [-u [UCONTAINERNAME]] manifest command ...

  positional arguments:
    manifest              manifest file to apply
    command               command to execute
    args                  command arguments

  optional arguments:
    -h, --help            show this help message and exit
    -u [UCONTAINERNAME], --ucontainername [UCONTAINERNAME]
                          user-specified name for container used to attach
                          proceses

Listen for performance and power data::

  usage: nrm listen [-h] [-u UUID] [-f FILTER]

  optional arguments:
    -h, --help            show this help message and exit
    -u UUID, --uuid UUID  container uuid to listen for
    -f FILTER, --filter FILTER
                          type of message to filter and prettyprint, in
                          {power,performance}

List running tasks::

  usage: nrm list [-h]

  optional arguments:
    -h, --help  show this help message and exit

Kill tasks::

  usage: nrm kill [-h] uuid

  positional arguments:
    uuid        uuid of the container

  optional arguments:
    -h, --help  show this help message and exit

Set a node power target::

  usage: nrm setpower [-h] [-f] limit

  positional arguments:
    limit         set new power limit

  optional arguments:
    -h, --help    show this help message and exit
    -f, --follow  listen for power changes


.. _Singularity: https://singularity.lbl.gov/install-request
.. _GitHub: https://github.com/anlsys/nrm-core/releases
