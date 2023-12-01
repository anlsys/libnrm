# NRM

The Argo Node Resource Manager (NRM) is a sub-node compute-resource manager,
capable of:
- Measuring application progress and performance
- Provisioning hardware threads, memory, GPUs, and power.

NRM is shipped as `libnrm`, and includes:

- The `nrmd` daemon
- The `nrmc` command-line client
- The `libnrm` API for  C/C++ application and utility instrumentation

Client Python bindings are under active development.

## Installation

Use [Spack](https://spack.io/) to install `libnrm`:

```
$ spack install libnrm
```

## Additional Info

See [Read the Docs](https://nrm.readthedocs.io/en/latest/) for documentation.

Use the GitHub issues to report bugs or ask for help using this library.
