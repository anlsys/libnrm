#!/usr/bin/env bats
# vim: set ft=bash:


@test "nrm-geopm --version works" {
	run nrm-geopm --version
	echo $output | grep nrm-geopm
}

@test "nrm-geopm --help works" {
	run nrm-geopm --help
	echo $output | grep nrm-geopm
}
