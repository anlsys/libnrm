#!/usr/bin/env bats
# vim: set ft=bash:

setup_file() {
	nrm-setup -p $ABS_TOP_BUILDDIR -o $BATS_FILE_TMPDIR --dummy &
	NRM_SETUP_PID=$!
	# wait for nrm-setup to start sleeping
	sleep 1
}

setup() {
	load 'runcmd.bash'
}

@test "server connect" {
	# use list-sensors to check if the client can connect to the server
	bats-nrm-run nrmc list-sensors
	[ "$status" -eq 0 ]
}

@test "list dummy sensor" {
	# can we list sensors
	bats-nrm-run nrmc -q list-sensors
	[ "$status" -eq 0 ]
	# actual check: dummy sensor
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-sensor"
}

@test "find dummy sensor" {
	# can we find the dummy sensor directly
	bats-nrm-run nrmc -q find-sensor "nrm-dummy-extra-sensor"
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# check that the output is fine
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-sensor"
}

@test "list dummy actuator" {
	# can we list actuators
	bats-nrm-run nrmc -q list-actuators
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# actual check: dummy actuator
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-actuator"
}

@test "find dummy actuator" {
	# can we find the dummy actuator directly
	bats-nrm-run nrmc -q find-actuator "nrm-dummy-extra-actuator"
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# check that the output is fine
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-actuator"
}

@test "actuate" {
	# find the dummy actuator
	bats-nrm-run nrmc -q find-actuator "nrm-dummy-extra-actuator"
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# extract valid choice from output
	choice=`echo "$output" | jq .[0].choices[-1]`
	echo "$choice"
	bats-nrm-run nrmc -q actuate "nrm-dummy-extra-actuator" $choice
	[ "$status" -eq 0 ]
}

@test "list slices" {
	# can we list slices
	bats-nrm-run nrmc -q list-slices
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# actual check: just make sure that it's valid json
	echo "$output" | jq
}

@test "list scopes" {
	# can we list scopes
	bats-nrm-run nrmc -q list-scopes
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# actual check: just make sure that it's valid json
	echo "$output" | jq
}

teardown_file() {
	run kill $NRM_SETUP_PID
	run pkill -9 nrm
}
