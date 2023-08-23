#!/usr/bin/env bats
# vim: set ft=bash:

setup() {
	nrm-setup -p $ABS_TOP_BUILDDIR -o $BATS_TEST_TMPDIR &
NRM_SETUP_PID=$!
	# wait for nrm-setup to start sleeping
	sleep 1
}

@test "initialize without signal" {
	#  defaults should be CPU_POWER and DRAM_POWER
	cat $BATS_TEST_TMPDIR/nrm-geopm-stderr.log
	run timeout 5 nrm-geopm
	[ "$status" -eq 0 ]
}

@test "one signal" {
	run timeout 5 nrm-geopm -s CPU_POWER
	cat $BATS_TEST_TMPDIR/nrm-geopm-stderr.log
	# TODO: determine output-string that decides measurement-success
	[ $event_count -eq 1 ]
}

@test "one verbose signal" {
	run timeout 5 nrm-geopm -vs CPU_POWER
	cat $BATS_TEST_TMPDIR/nrm-geopm-stderr.log
	# TODO: determine output-string that decides measurement-success
	[ $event_count -eq 1 ]
}

@test "two signals" {
	run timeout 5 nrm-geopm -s CPU_POWER -s DRAM_POWER
	cat $BATS_TEST_TMPDIR/nrm-geopm-stderr.log
	[ $cpu_event_count -eq 1 ] && [ $dram_event_count -eq 1]
}

teardown_file() {
	run kill $NRM_SETUP_PID
	run pkill -9 nrm
}
