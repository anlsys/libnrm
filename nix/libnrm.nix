{ stdenv, autoreconfHook, pkgconfig, zeromq, czmq, jansson, check }:
stdenv.mkDerivation {
  src = ../.;
  name = "libnrm";
  nativeBuildInputs = [ autoreconfHook pkgconfig ];
  buildInputs = [ zeromq check jansson czmq ];
}
