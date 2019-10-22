# MPI library to use ARGO Node Resource Manager (MPI-NRM) downstream API 

This Message Passing Interface (MPI) library allows application of runtime
policies for energy efficiency through the MPI standard profiling interface
(PMPI).

The current implementation passes phase contextual information (compute and
barrier times) to the Argo Node Resource Manager (NRM). The NRM using this
contextual information invokes power policies to improve energy efficiency
of the node.

The power policies in NRM need contextual information from the application
(e.g. time spent doing computation and in the barrier during a phase) for
decision. This information from the application can be provided to NRM using
a C downstream API. 

This MPI library written in C/C++ uses the MPI Profiling
Interface (PMPI) to intercept MPI calls and sends the application context
information to NRM using the C downstream API.

## Requirements
The C downstream API uses ZeroMQ (http://www.zeromq.org) to transmit messages
to NRM. So it needs to be installed in addition to any stable MPI version
(however, it has been only tested with MPICH 3.2.1).

Update the following paths in Makefile to point to your ZeroMQ and MPI
installations respectively.

ZMQ_PATH =

& 

MPI_INCLUDE =

Calling a `make` then should create a `libmpi_nrm.so` shared library file.

## Basic Usage

This shared library file can then be used with the NRM
(https://xgitlab.cels.anl.gov/argo/nrm/tree/master) by manually updating the

applicationpreloadlibrary = ''

in `containers.py` at the moment.

The need to manually update will be fixed in the future.

## Additional Info

Report bugs to Sridutt Bhalachandra (<sriduttb@anl.gov>)
