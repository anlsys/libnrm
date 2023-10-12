#!/usr/bin/env bats
# vim: set ft=bash:

bats_require_minimum_version 1.5.0

@test "nrm-geopm --version works" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrm-geopm --version
	echo $output | grep nrm-geopm
}

@test "nrm-geopm --help works" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrm-geopm --help
	echo $output | grep nrm-geopm
}
