{ stdenv, autoreconfHook, pkgconfig, git, hwloc, mpich, openmp, python3, python3Packages, systemd, libelf}:
stdenv.mkDerivation {
  src = fetchTarball "https://github.com/geopm/geopm/releases/download/v2.0.2/geopm-service-2.0.2.tar.gz";
  name = "geopmd";
  configureFlags = [ "--disable-mpi" "--disable-fortran"];
  nativeBuildInputs = [ autoreconfHook pkgconfig git ];
  propagatedBuildInputs = [
    libelf 
    python3Packages.dasbus
    python3Packages.psutil
    python3Packages.cffi
    python3Packages.docstring-parser
    python3Packages.sphinx
    python3Packages.sphinx_rtd_theme
    python3Packages.sphinxemoji
    python3Packages.jsonschema
    python3Packages.pyyaml
    python3Packages.pygments
  ];
  buildInputs = [ 
    hwloc
    mpich
    openmp
    systemd
    python3
  ];
}
