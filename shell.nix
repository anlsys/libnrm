# development shell, includes aml dependencies and dev-related helpers
{ pkgs ? import ./. { } }:
with pkgs;
mkShell {
  inputsFrom = [ libnrm ];
  nativeBuildInputs = [ autoreconfHook pkg-config ];
  buildInputs = [
    # deps for debug
    gdb
    valgrind
    # style checks
    clang-tools
    python3
    llvmPackages.clang-unwrapped.python
    bear
    # docs
    graphviz
    doxygen
    python3Packages.sphinx
    python3Packages.breathe
    python3Packages.sphinx_rtd_theme
    python3Packages.sphinx-design
    jq
    bats
    # python bindings
    python3Packages.flake8
    python3Packages.black
    # extra binaries
    python3Packages.prometheus-client
  ];
  CFLAGS =
    "-std=c17 -Wall -Wextra -Werror -Wno-builtin-declaration-mismatch";
}
