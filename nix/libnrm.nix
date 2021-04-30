{ stdenv, autoreconfHook, pkgconfig, zeromq, czmq, gfortran }:
stdenv.mkDerivation {
  src = ../.;
  name = "libnrm";
  nativeBuildInputs = [ autoreconfHook pkgconfig ];
  buildInputs = [ zeromq czmq gfortran ];
}
