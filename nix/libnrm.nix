{ stdenv
, autoreconfHook
, bats
, check
, czmq
, git
, hwloc
, jansson
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
    papi
    protobufc
    zeromq
  ];
}
