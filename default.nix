{ pkgs ? import (builtins.fetchTarball "https://github.com/NixOS/nixpkgs/archive/25.05.tar.gz") {}
}:
let
  callPackage = pkgs.lib.callPackageWith pkgs;
in pkgs // rec {
  libnrm = pkgs.callPackage ./nix/libnrm.nix { openmp = pkgs.llvmPackages_17.openmp; };
}
