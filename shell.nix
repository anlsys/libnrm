# development shell, includes aml dependencies and dev-related helpers
{ pkgs ? import ./. { } }:
with pkgs;
mkShell {
  inputsFrom = [ libnrm ];
  nativeBuildInputs = [ autoreconfHook pkgconfig ];
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
    jq
    bats
  ];

  CFLAGS =
    "-std=c99 -pedantic -Wall -Wextra -g -O0";
}
