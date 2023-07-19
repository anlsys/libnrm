#!/usr/bin/env bats
# vim: set ft=bash:

setup() {
	nrm-setup -p $ABS_TOP_BUILDDIR -o $BATS_TEST_TMPDIR
	NRM_SETUP_PID=$!
}

@test "exit works" {
	# completely ignore any exit status on nrmc
	run timeout 1 nrmc exit
	wait $NRM_SETUP_PID
	[ "$?" -eq 0 ]
}

@test "--freq works" {
	run timeout 2 nrm-dummy-extra --freq 0.5
	kill $NRM_SETUP_PID
	event_count=`grep EVENT $BATS_TEST_TMPDIR/nrmd-stderr.log | wc -l`
	# debug log makes every event appear twice
	[ $event_count -ge 8 ]
}

teardown_file() {
	run pkill -9 nrm
}
