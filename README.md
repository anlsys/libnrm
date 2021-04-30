# NRM Communication library

This library aims to provide a consistent API to communicate with the Argo Node
Resource Manager.

At the moment, only the downstream client side is provided (allowing
applications to report progress back to the NRM). This library will grow with
time.

Current API is available for both C and Fortran programs.

The power policies in NRM need contextual information from the application
(e.g. time spent doing computation and in the barrier during a phase) for
decision. This information from the application can be provided to NRM using
a C downstream API. 

## Requirements

The C downstream API uses ZeroMQ (http://www.zeromq.org) to transmit messages
to NRM. So it needs to be installed.

## Additional Info

Use the github issues to report bugs or ask for help using this library.
