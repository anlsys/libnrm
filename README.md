# NRM

The Argo Node Resource Manager (NRM) is a node-level resource manager, a
user-space software infrastructure to:
- Monitor application performance, resource usage
- Act on a variety of control interfaces to improve performance/efficiency

This repository includes:

- The `nrmd` daemon, a user-space program that centralizes information about
  the system
- The `nrmc` command-line client, to query the daemon, listen to event, trigger
  actions
- The CLI client is a direct utilization of the `libnrm` libraty, a C API for
instrumenting applications, and to communicate with the NRM infrastructure.

Various other binaries and libraries are provided, each to provide NRM with
additional monitoring capabilies (sensors) or control capabilities (actuators).

## Installation

Use [Spack](https://spack.io/) to install `libnrm`:

```
$ spack install libnrm
```

## Additional Info

See [Read the Docs](https://nrm.readthedocs.io/en/latest/) for documentation.

Use the GitHub issues to report bugs or ask for help using this library.
