#!/usr/bin/env bats
# vim: set ft=bash:

setup() {
	nrm-setup -p $ABS_TOP_BUILDDIR -o $BATS_TEST_TMPDIR &
	NRM_SETUP_PID=$!
}

@test "exit works" {
	# completely ignore any exit status on nrmc
	run -0 timeout 1 nrmc exit
	wait $NRM_SETUP_PID
}

@test "--freq works" {
	run timeout 2 nrm-dummy-extra --freq 2
	kill $NRM_SETUP_PID
	cat $BATS_TEST_TMPDIR/nrmd-stderr.log
	event_count=`grep EVENT $BATS_TEST_TMPDIR/nrmd-stderr.log | wc -l`
	# debug log makes every event appear 3 times
	[ $event_count -ge 12 ]
}

@test "--help works" {
	run timeout 2 nrm-dummy-extra --help
	kill $NRM_SETUP_PID
	help_count= `cat $BATS_TEST_TMPDIR/nrmd-stderr.log | wc -l`
	[ $help_count -eq 12 ]
}

teardown_file() {
	run pkill -9 nrm
}
