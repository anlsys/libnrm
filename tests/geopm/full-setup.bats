#!/usr/bin/env bats
# vim: set ft=bash:

setup_file() {
	nrm-setup -p $ABS_TOP_BUILDDIR -o $BATS_TEST_TMPDIR &
NRM_SETUP_PID=$!
	# wait for nrm-setup to start sleeping
	sleep 1
}

@test "initialize without signal" {
	run nrm-geopm
	[ "$status" -eq 0 ]
}

@test "one signal" {
	run timeout 4 nrm-geopm -s CPU_POWER
	kill $NRM_SETUP_PID
	cat $BATS_TEST_TMPDIR/nrm-geopm-stderr.log
	# TODO: determine output-string that decides measurement-success
	[ $event_count -eq 1 ]
}

@test "one verbose signal" {
	run timeout 4 nrm-geopm -vs CPU_POWER
	kill $NRM_SETUP_PID
	cat $BATS_TEST_TMPDIR/nrm-geopm-stderr.log
	# TODO: determine output-string that decides measurement-success
	[ $event_count -eq 1 ]
}

@test "two signals" {
	run timeout 4 nrm-geopm -s CPU_POWER -s DRAM_POWER
	kill $NRM_SETUP_PID
	cat $BATS_TEST_TMPDIR/nrm-geopm-stderr.log
	[ $cpu_event_count -eq 1 ] && [ $dram_event_count -eq 1]
}

teardown_file() {
	run kill $NRM_SETUP_PID
	run pkill -9 nrm
}
