#!/usr/bin/env bats
# vim: set ft=bash:

setup_file() {
	run pkill -9 nrm
}

launch_daemon() {
	nrmd &>/dev/null 3>&- &
	NRMD_PID=$!
	# wait until we are sure that the daemon is running
	for i in `seq 1 5`; do
		run timeout 0.2 nrmc -q list-sensors
		echo $output
		if [ "$status" -eq 0 ]; then
			break
		fi
	done
	[ "$status" -eq 0 ]
}

launch_dummy_extra() {
	nrm-dummy-extra &>/dev/null 3>&- &
	NRMD_DUMMY_PID=$!
	# make sure dummy is actually running and registered to the daemon
	for i in `seq 1 5`; do
		run nrmc -q list-sensors
		[ "$status" -eq 0 ]
		echo $output
		echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-sensor"
		if [ "$?" -eq 0 ]; then
			break
		fi
		sleep 0.1
	done
	[ "$?" -eq 0 ] 
}

setup() {
	launch_daemon
	launch_dummy_extra
}

@test "server connect" {
	# use list-sensors to check if the client can connect to the server
	run nrmc list-sensors
}

@test "list dummy sensor" {
	# can we list sensors
	run nrmc -q list-sensors
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# actual check: dummy sensor
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-sensor"
}

@test "find dummy sensor" {
	# can we find the dummy sensor directly
	run nrmc -q find-sensor "nrm-dummy-extra-sensor"
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# check that the output is fine
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-sensor"
}

@test "list dummy actuator" {
	# can we list actuators
	run nrmc -q list-actuators
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# actual check: dummy actuator
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-actuator"
}

@test "find dummy actuator" {
	# can we find the dummy actuator directly
	run nrmc -q find-actuator "nrm-dummy-extra-actuator"
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# check that the output is fine
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-actuator"
}

@test "actuate" {
	# find the dummy actuator
	run nrmc -q find-actuator "nrm-dummy-extra-actuator"
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# extract valid choice from output
	choice=`echo "$output" | jq .[0].choices[-1]`
	echo "$choice"
	run nrmc -q actuate "nrm-dummy-extra-actuator" $choice
	[ "$status" -eq 0 ]
}

@test "list slices" {
	# can we list slices
	run nrmc -q list-slices
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# actual check: just make sure that it's valid json
	echo "$output" | jq
}

@test "list scopes" {
	# can we list scopes
	run nrmc -q list-scopes
	[ "$status" -eq 0 ]
	# print the output in case of errors
	echo "$output"
	# actual check: just make sure that it's valid json
	echo "$output" | jq
}

teardown() {
	kill $NRMD_DUMMY_PID
	kill $NRMD_PID
}

teardown_file() {
	run pkill -9 nrm
}
