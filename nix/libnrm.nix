{ stdenv, autoreconfHook, pkgconfig, zeromq, czmq, gfortran }:
stdenv.mkDerivation {
  src = ../.;
  name = "libnrm";
  nativeBuildInputs = [ autoreconfHook gfortran pkgconfig ];
  buildInputs = [ zeromq czmq gfortran ];
}
