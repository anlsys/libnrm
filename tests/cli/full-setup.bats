#!/usr/bin/env bats
# vim: set ft=bash:

setup_file() {
	nrm-setup -p $ABS_TOP_BUILDDIR -o $BATS_FILE_TMPDIR --dummy &
	NRM_SETUP_PID=$!
	# wait for nrm-setup to start sleeping
	sleep 1
}

@test "server connect" {
	# use list-sensors to check if the client can connect to the server
	run -0 nrmc list-sensors
}

@test "list dummy sensor" {
	# can we list sensors
	run -0 nrmc -q list-sensors
	# print the output in case of errors
	echo "$output"
	# actual check: dummy sensor
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-sensor"
}

@test "find dummy sensor" {
	# can we find the dummy sensor directly
	run -0 nrmc -q find-sensor "nrm-dummy-extra-sensor"
	# print the output in case of errors
	echo "$output"
	# check that the output is fine
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-sensor"
}

@test "list dummy actuator" {
	# can we list actuators
	run -0 nrmc -q list-actuators
	# print the output in case of errors
	echo "$output"
	# actual check: dummy actuator
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-actuator"
}

@test "find dummy actuator" {
	# can we find the dummy actuator directly
	run -0 nrmc -q find-actuator "nrm-dummy-extra-actuator"
	# print the output in case of errors
	echo "$output"
	# check that the output is fine
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-actuator"
}

@test "actuate" {
	# find the dummy actuator
	run -0 nrmc -q find-actuator "nrm-dummy-extra-actuator"
	# print the output in case of errors
	echo "$output"
	# extract valid choice from output
	choice=`echo "$output" | jq .[0].choices[-1]`
	echo "$choice"
	run -0 nrmc -q actuate "nrm-dummy-extra-actuator" $choice
}

@test "list slices" {
	# can we list slices
	run -0 nrmc -q list-slices
	# print the output in case of errors
	echo "$output"
	# actual check: just make sure that it's valid json
	echo "$output" | jq
}

@test "list scopes" {
	# can we list scopes
	run -0 nrmc -q list-scopes
	# print the output in case of errors
	echo "$output"
	# actual check: just make sure that it's valid json
	echo "$output" | jq
}

teardown_file() {
	run kill $NRM_SETUP_PID
	run pkill -9 nrm
}
