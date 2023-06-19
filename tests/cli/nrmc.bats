#!/usr/bin/env bats
# vim: set ft=bash:

setup() {
	nrmd &>/dev/null 3>&- &
	NRMD_PID=$!
	nrm-dummy-extra &>/dev/null 3>&- &
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
	# can we find the dummy sensor
	run nrmc -q list-sensors
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# actual check we want
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-sensor"
}

@test "find dummy sensor" {
	# can we find the dummy sensor
	run nrmc -q list-sensors
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# actual check we want
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-sensor"
}

teardown() {
	kill $NRMD_DUMMY_PID
	kill $NRMD_PID
}
