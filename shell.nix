# development shell, includes aml dependencies and dev-related helpers
{ pkgs ? import ./. { } }:
with pkgs;
mkShell {
  inputsFrom = [ libnrm ];
  nativeBuildInputs = [ autoreconfHook pkgconfig ];
  buildInputs = [
    # deps for debug
    gdb
    # style checks
    clang-tools
  ];

  CFLAGS =
    "-std=c99 -pedantic -Wall -Wextra -Werror";
}
