Welcome to NRM's documentation!
===============================

For a high-level overview of NRM, please refer to the Argo website_.

The Node Resource Manager (NRM) is a node-local userspace client-server daemon
for managing your scientific applications. NRM:

   1.  Runs the various tasks that compose an application in resource-constrained slices
   2.  Monitors performance, power use and application progress
   3.  Arbitrates node-level resources, including CPU Cores, NUMA Nodes, and Power budgets

NRM is shipped as ``libnrm``, and includes:

   1. The ``nrmc`` command-line client
   2. The ``nrmd`` daemon
   3. The ``libnrm`` API for application instrumentation

The :doc:`quickstart <pages/quickstart>` guide describes the use of these components.

Install
=======

Use Spack_ to install libnrm::

    $ spack install libnrm

.. toctree::
   :maxdepth: 2
   :caption: User Guides:

   pages/quickstart

.. toctree::
   :maxdepth: 2
   :caption: Instrumentation:

   pages/doxy_c_api
   pages/simple_c_example


Indices and tables
==================

* :ref:`search`

.. _website: https://www.mcs.anl.gov/research/projects/argo/overview/nrm/
.. _Spack: https://spack.io/