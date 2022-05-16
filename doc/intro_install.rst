.. highlight:: bash

Welcome to the libnrm guide. This document will help you in your use of the
C/C++ and Fortran interface for NRM.

.. toctree::
   :maxdepth: 2

   NRM Home <https://nrm.readthedocs.io/en/main/>
   NRM-Python <https://nrm.readthedocs.io/projects/nrm-python/en/master/>
   NRM-Core <https://nrm.readthedocs.io/projects/nrm-core/en/master/>

Install
=======

The libnrm code can be installed from source::

 git clone https://github.com/anlsys/libnrm.git
 cd libnrm
 ./autogen.sh
 ./configure
 make && make install
