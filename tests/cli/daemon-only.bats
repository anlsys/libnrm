#!/usr/bin/env bats
# vim: set ft=bash:

bats_require_minimum_version 1.5.0

setup() {
	$ABS_TOP_BUILDDIR/nrm-setup -p $ABS_TOP_BUILDDIR -o $BATS_TEST_TMPDIR &
	NRM_SETUP_PID=$!
	# wait for nrm-setup to start sleeping
	while [ ! -e $BATS_TEST_TMPDIR/nrm-setup-ready.log ]; do
		sleep 1
	done
}

@test "exit works" {
	# completely ignore any exit status on nrmc
	run $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc exit
	wait $NRM_SETUP_PID
}

@test "--freq works" {
	if [ -n "$LOG_COMPILER" ]; then
		kill $NRM_SETUP_PID
		skip "disabling timing tests on valgrind"
	fi

	run timeout 2 $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrm-dummy-extra --freq 2
	kill $NRM_SETUP_PID
	cat $BATS_TEST_TMPDIR/nrmd-stderr.log
	event_count=`grep EVENT $BATS_TEST_TMPDIR/nrmd-stderr.log | grep "nrm-dummy-extra-sensor" | wc -l`
	# debug log makes every event appear 4 times, we should see it at least 3 times
	[ $event_count -ge 12 ]
}

teardown_file() {
	run pkill -9 nrm
}
