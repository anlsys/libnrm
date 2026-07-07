#!/usr/bin/env bats
# vim: set ft=bash:

bats_require_minimum_version 1.5.0

@test "nrm-nvml --version works" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrm-nvml --version
	echo $output | grep nrm-nvml
}

@test "nrm-nvml --help works" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrm-nvml --help
	echo $output | grep nrm-nvml
}
