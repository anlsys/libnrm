{ stdenv
, autoreconfHook
, bats
, check
, czmq
, git
, hwloc
, jansson
, openmp
, papi
, pkgconfig
, protobufc
, zeromq
}:
stdenv.mkDerivation {
  src = ../.;
  name = "libnrm";
  nativeBuildInputs = [ autoreconfHook pkgconfig git ];
  buildInputs = [
    bats
    check
    czmq
    hwloc
    jansson
    openmp
    papi
    protobufc
    zeromq
  ];
}
