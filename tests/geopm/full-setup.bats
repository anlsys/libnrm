#!/usr/bin/env bats
# vim: set ft=bash:

bats_require_minimum_version 1.5.0

setup_file() {
	$ABS_TOP_BUILDDIR/nrm-setup -p $ABS_TOP_BUILDDIR -o $BATS_FILE_TMPDIR &
	NRM_SETUP_PID=$!
	# wait for nrm-setup to start sleeping
	while [ ! -e $BATS_FILE_TMPDIR/nrm-setup-ready.log ]; do
		sleep 1
	done
}

@test "initialize without signal" {
	run -143 timeout --preserve-status 5 $ABS_TOP_BUILDDIR/nrm-geopm
	#  default should be CPU_FREQUENCY_STATUS
	grep CPU_FREQUENCY_STATUS $BATS_FILE_TMPDIR/nrmd-stderr.log
}

@test "one signal" {
	run -143 timeout --preserve-status 5 $ABS_TOP_BUILDDIR/nrm-geopm -e CPU_FREQUENCY_STATUS
	grep CPU_FREQUENCY_STATUS $BATS_FILE_TMPDIR/nrmd-stderr.log
}

@test "two signals" {
	run -143 timeout --preserve-status 5 $ABS_TOP_BUILDDIR/nrm-geopm -e TIME -e CPU_FREQUENCY_STATUS
	grep TIME $BATS_FILE_TMPDIR/nrmd-stderr.log
	grep CPU_FREQUENCY_STATUS $BATS_FILE_TMPDIR/nrmd-stderr.log
}

teardown_file() {
	run kill $NRM_SETUP_PID
	run pkill -9 nrm
}
