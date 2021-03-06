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
    protobufc
  ];

  CFLAGS =
    "-std=c99 -pedantic -Wall -Wextra -g -O0";
}
