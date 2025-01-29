{ stdenv
, autoreconfHook
, bats
, check
, czmq
, git
, hwloc
, jansson
, mpich
, openmp
, papi
, pkg-config
, protobufc
, zeromq
}:
stdenv.mkDerivation {
  src = ../.;
  name = "libnrm";
  nativeBuildInputs = [ autoreconfHook pkg-config git ];
  buildInputs = [
    bats
    check
    czmq
    hwloc
    jansson
    mpich
    openmp
    papi
    protobufc
    zeromq
  ];
}
