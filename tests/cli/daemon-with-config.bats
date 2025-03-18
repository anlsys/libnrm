#!/usr/bin/env bats
# vim: set ft=bash:

bats_require_minimum_version 1.5.0

setup() {
	$ABS_TOP_BUILDDIR/nrm-setup -p $ABS_TOP_BUILDDIR -c $ABS_TOP_SRCDIR/examples/config.json -o $BATS_TEST_TMPDIR &
	NRM_SETUP_PID=$!
	# wait for nrm-setup to start sleeping
	while [ ! -e $BATS_TEST_TMPDIR/nrm-setup-ready.log ]; do
		sleep 1
	done
}

@test "inactive" {
	# completely ignore any exit status on nrmc
	run $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc tick
	sleep 1
	run $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc exit
	wait $NRM_SETUP_PID
}

teardown_file() {
	run pkill -9 nrm
}
