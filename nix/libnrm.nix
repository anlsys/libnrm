{ stdenv, autoreconfHook, pkgconfig, zeromq, czmq, jansson, check, protobufc, hwloc, git, bats }:
stdenv.mkDerivation {
  src = ../.;
  name = "libnrm";
  nativeBuildInputs = [ autoreconfHook pkgconfig git ];
  buildInputs = [ zeromq check jansson czmq protobufc hwloc bats ];
}
