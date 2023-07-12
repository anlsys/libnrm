{ pkgs ? import (builtins.fetchTarball "https://github.com/NixOS/nixpkgs/archive/23.05.tar.gz") {}
}:
let
  callPackage = pkgs.lib.callPackageWith pkgs;
in pkgs // {
  libnrm = callPackage ./nix/libnrm.nix { openmp = pkgs.llvmPackages_15.openmp; };
}
