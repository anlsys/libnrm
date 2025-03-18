#!/usr/bin/env bats
# vim: set ft=bash:

bats_require_minimum_version 1.5.0

setup_file() {
	$ABS_TOP_BUILDDIR/nrm-setup -p $ABS_TOP_BUILDDIR -o $BATS_FILE_TMPDIR --dummy &
	NRM_SETUP_PID=$!
	# wait for nrm-setup to start sleeping
	while [ ! -e $BATS_FILE_TMPDIR/nrm-setup-ready.log ]; do
		sleep 1
	done
}

@test "server connect" {
	# check if the client can connect to the server
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc connect
}

@test "tick (no config)" {
	# check if the daemon can still properly tick without a configuration
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc tick
}

@test "list dummy sensor" {
	# can we list sensors
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc list-sensors
	# actual check: dummy sensor
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-sensor"
}

@test "find dummy sensor" {
	# can we find the dummy sensor directly
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc find-sensor "nrm-dummy-extra-sensor"
	# check that the output is fine
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-sensor"
}

@test "list dummy actuator" {
	# can we list actuators
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc list-actuators
	# actual check: dummy actuator
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-actuator"
}

@test "find dummy actuator" {
	# can we find the dummy actuator directly
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc find-actuator "nrm-dummy-extra-actuator"
	# check that the output is fine
	echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-actuator"
}

@test "actuate" {
	# find the dummy actuator
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc find-actuator "nrm-dummy-extra-actuator"
	# extract valid choice from output
	choice=`echo "$output" | jq .[0].choices[-1]`
	echo "$choice"
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc actuate "nrm-dummy-extra-actuator" $choice
	grep "action $choice" $BATS_FILE_TMPDIR/nrm-dummy-extra-stderr.log
}

@test "list slices" {
	# can we list slices
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc list-slices
	# actual check: just make sure that it's valid json
	echo "$output" | jq
}

@test "list scopes" {
	# can we list scopes
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc list-scopes
	# actual check: just make sure that it's valid json
	echo "$output" | jq
}

@test "send event with unknown scope" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc send-event nrm-dummy-extra-sensor unknown 42
}

@test "listen (no argument)" {
	if [ -n "$LOG_COMPILER" ]; then
		skip "disabling signal tests on valgrind"
	fi
	run -143 --separate-stderr timeout --preserve-status 5 $ABS_TOP_BUILDDIR/nrmc listen
	event_count=`echo "$output" | wc -l`
	[ $event_count -ge 1 ]
}

@test "listen (daemon topic)" {
	if [ -n "$LOG_COMPILER" ]; then
		skip "disabling signal tests on valgrind"
	fi
	run -143 --separate-stderr timeout --preserve-status 5 $ABS_TOP_BUILDDIR/nrmc listen daemon
	event_count=`echo "$output" | wc -l`
	[ $event_count -ge 1 ]
}

@test "listen (null topic)" {
	if [ -n "$LOG_COMPILER" ]; then
		skip "disabling signal tests on valgrind"
	fi
	run -143 --separate-stderr timeout --preserve-status 5 $ABS_TOP_BUILDDIR/nrmc listen null
	event_count=`echo "$output" | wc -l`
	[ $event_count -eq 1 ]
}

teardown_file() {
	run kill $NRM_SETUP_PID
	run pkill -9 nrm
}
