#!/usr/bin/env bats
# vim: set ft=bash:

bats_require_minimum_version 1.5.0

setup() {
	$ABS_TOP_BUILDDIR/nrm-setup -p $ABS_TOP_BUILDDIR -o $BATS_TEST_TMPDIR &
	NRM_SETUP_PID=$!
	sleep 1
}

@test "exit works" {
	# completely ignore any exit status on nrmc
	run $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc exit
	wait $NRM_SETUP_PID
	[ "$?" -eq 0 ]
}

@test "--freq works" {
	run timeout 2 $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrm-dummy-extra --freq 2
	kill $NRM_SETUP_PID
	cat $BATS_TEST_TMPDIR/nrmd-stderr.log
	event_count=`grep EVENT $BATS_TEST_TMPDIR/nrmd-stderr.log | wc -l`
	# debug log makes every event appear 3 times
	[ $event_count -ge 12 ]
}

teardown_file() {
	run pkill -9 nrm
}
