{ pkgs ? import (builtins.fetchTarball "https://github.com/NixOS/nixpkgs/archive/23.05.tar.gz") {}
}:
let
  callPackage = pkgs.lib.callPackageWith pkgs;
in pkgs // rec {
  geopmd = pkgs.callPackage ./nix/geopmd.nix { openmp = pkgs.llvmPackages_12.openmp; };
  libnrm = pkgs.callPackage ./nix/libnrm.nix { openmp = pkgs.llvmPackages_15.openmp; inherit geopmd; };
}
