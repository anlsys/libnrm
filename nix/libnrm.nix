{ stdenv, autoreconfHook, pkgconfig, zeromq, jansson, check }:
stdenv.mkDerivation {
  src = ../.;
  name = "libnrm";
  nativeBuildInputs = [ autoreconfHook pkgconfig ];
  buildInputs = [ zeromq check jansson];
}
