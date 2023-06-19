#!/usr/bin/env bats
# vim: set ft=bash:

setup() {
	nrmd &>/dev/null 3>&- &
	NRMD_PID=$!
	nrm-dummy-extra  3>&- &
	NRMD_DUMMY_PID=$!
}

@test "--version works" {
	nrmc --version
}

@test "--help works" {
	nrmc --help
}

@test "server connect" {
	# use list-sensors to check if the client can connect to the server
	run nrmc list-sensors
}

@test "list dummy sensor" {
	[[ $(type -t bats_require_minimum_version) == function ]] && bats_require_minimum_version 1.5.0
	# can we find the dummy sensor
	run --separate-stderr nrmc list-sensors
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-sensor"
}

@test "find dummy sensor" {
	[[ $(type -t bats_require_minimum_version) == function ]] && bats_require_minimum_version 1.5.0
	# can we find the dummy sensor
	run --separate-stderr nrmc list-sensors
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-sensor"
}

teardown() {
	kill $NRMD_DUMMY_PID
	kill $NRMD_PID
}
