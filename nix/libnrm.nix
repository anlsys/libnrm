{ stdenv, autoreconfHook, pkgconfig, zeromq, czmq, jansson, check, protobufc }:
stdenv.mkDerivation {
  src = ../.;
  name = "libnrm";
  nativeBuildInputs = [ autoreconfHook pkgconfig ];
  buildInputs = [ zeromq check jansson czmq protobufc ];
}
