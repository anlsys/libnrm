.. highlight:: bash

Welcome to the libnrm guide. This document will help you in your use of the
C/C++ interface for NRM.

By including libnrm instrumentation, applications can connect to NRM and take
advantage of its dynamic power management capabilities.

See the complete API and C examples below.

Install
=======

The libnrm code can be installed from source::

 git clone https://github.com/anlsys/libnrm.git
 cd libnrm
 ./autogen.sh
 ./configure
 make && make install
