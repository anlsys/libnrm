#!/usr/bin/env bats
# vim: set ft=bash:

setup_file() {
	nrm-setup -p $ABS_TOP_BUILDDIR -o $BATS_FILE_TMPDIR &
	NRM_SETUP_PID=$!
}

@test "preload, no callbacks" {
	run -0 nrmc -vvv run -d $ABS_TOP_BUILDDIR/.libs/libnrm-ompt.so ls -al
	echo "$output"
}

@test "preload, basic omp" {
	run -0 nrmc -vvv run -d $ABS_TOP_BUILDDIR/.libs/libnrm-ompt.so ./omp_basic
	echo "$output"
}

@test "preload, stream omp" {
	run -0 nrmc -vvv run -d $ABS_TOP_BUILDDIR/.libs/libnrm-ompt.so ./omp_stream
	echo "$output"
}

teardown_file() {
	run kill $NRM_SETUP_PID
	run pkill -9 nrm
}
