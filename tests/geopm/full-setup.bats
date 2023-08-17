#!/usr/bin/env bats
# vim: set ft=bash:

setup_file() {
	nrm-setup -p $ABS_TOP_BUILDDIR -o $BATS_TEST_TMPDIR --dummy &
	NRM_SETUP_PID=$!
	# wait for nrm-setup to start sleeping
	sleep 1
}

@test "initialize without signal" {
	run nrm-geopm
	[ "$status" -eq 0 ]
}

teardown_file() {
	run kill $NRM_SETUP_PID
	run pkill -9 nrm
}
