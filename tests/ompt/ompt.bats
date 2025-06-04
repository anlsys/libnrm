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

@test "preload, no callbacks" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc -vvv run -t $ABS_TOP_BUILDDIR/.libs/libnrm-ompt.so ls -al
}

@test "preload, basic omp" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc -vvv run -t $ABS_TOP_BUILDDIR/.libs/libnrm-ompt.so ./omp_basic
}

@test "preload, stream omp" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc -vvv run -t $ABS_TOP_BUILDDIR/.libs/libnrm-ompt.so ./omp_stream
}

teardown_file() {
	run kill $NRM_SETUP_PID
	run pkill -9 nrm
}
