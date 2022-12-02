## NRM Communication library - libnrm

This library aims to provide a consistent API to communicate with the Argo Node
Resource Manager.

At the moment, only the downstream client side is provided (allowing
applications to report progress back to the NRM). This library will grow with
time.

Current API is available for C/C++ programs.

The power policies in NRM need contextual information from the application
(e.g. time spent doing computation and in the barrier during a phase) for
decision. This information from the application can be provided to NRM using
the C downstream API.

# Dependencies

Besides a C compiler, you need the `autoconf`, `automake`, `libtool`, and `pkg-config` build tools.

## Compile-time

* `libzmq3-dev`
* `libczmq-dev`
* `libprotobuf-c-dev`
* `protobuf-c-compiler`
* `libjansson-dev`
* `libhwloc-dev`
* `check`

## Runtime

* `libzmq5`
* `libczmq4`
* `libjansson4`


# Downloading

## Main repository

Main repository on GitHub: https://github.com/anlsys/libnrm

```
git clone https://github.com/anlsys/libnrm.git; cd libnrm
```

## Official Releases

See the releases page on GitHub: https://github.com/anlsys/libnrm/releases

```
wget https://github.com/anlsys/libnrm/releases/download/v0.7.0/libnrm-0.7.0.tar.gz
tar xvf libnrm-0.7.0.tar.gz; cd libnrm-0.7.0
```

# Building and Installing

From the libnrm root directory:

```
mkdir install
./autogen.sh
./configure --prefix=$PWD/install
make; make install
```

Specify `--enable-binaries` to `./configure` to build the NRM daemon and command-line utility.

## From Spack

libnrm is also available on the Spack package manager:

```
spack install libnrm
```

See the libnrm entry on the Spack docs for more info: https://spack.readthedocs.io/en/latest/package_list.html#libnrm

# Documentation

Available on Read the Docs: https://nrm.readthedocs.io/en/latest/libnrm.html

# Additional Info

Use the GitHub issues to report bugs or ask for help using this library. Contributions are welcome.

# Example Usage

See a simple example here: https://nrm.readthedocs.io/projects/libnrm/en/latest/pages/simple_c_example.html

Some utilities that use libnrm instrumentation are available in the nrm-extra repository:
https://github.com/anlsys/nrm-extra/tree/master/src
