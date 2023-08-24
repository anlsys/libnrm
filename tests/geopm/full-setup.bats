#!/usr/bin/env bats
# vim: set ft=bash:

bats_require_minimum_version 1.5.0

setup_file() {
	$ABS_TOP_BUILDDIR/nrm-setup -p $ABS_TOP_BUILDDIR -o $BATS_TEST_TMPDIR &
	NRM_SETUP_PID=$!
	# wait for nrm-setup to start sleeping
	while [ ! -e $BATS_FILE_TMPDIR/nrm-setup-ready.log ]; do
		sleep 1
	done
}

@test "initialize without signal" {
	#  defaults should be CPU_POWER and DRAM_POWER
	run -0 --separate-stderr timeout 5 $ABS_TOP_BUILDDIR/nrm-geopm
	cat $BATS_TEST_TMPDIR/nrm-geopm-stderr.log
}

@test "one signal" {
	run timeout 5 $ABS_TOP_BUILDDIR/nrm-geopm -e CPU_POWER
	cat $BATS_TEST_TMPDIR/nrm-geopm-stderr.log
	# TODO: determine output-string that decides measurement-success
	[ $event_count -eq 1 ]
}

@test "one verbose signal" {
	run timeout 5 $ABS_TOP_BUILDDIR/nrm-geopm -vvvve CPU_POWER
	cat $BATS_TEST_TMPDIR/nrm-geopm-stderr.log
	# TODO: determine output-string that decides measurement-success
	[ $event_count -eq 1 ]
}

@test "two signals" {
	run timeout 5 $ABS_TOP_BUILDDIR/nrm-geopm -e CPU_POWER -e DRAM_POWER
	cat $BATS_TEST_TMPDIR/nrm-geopm-stderr.log
	[ $cpu_event_count -eq 1 ] && [ $dram_event_count -eq 1]
}

teardown_file() {
	run kill $NRM_SETUP_PID
	run pkill -9 nrm
}
