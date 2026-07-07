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
	nvidia-smi -L 2>/dev/null | grep -q GPU || skip "no NVIDIA GPU available"
	run -143 timeout --preserve-status 5 $ABS_TOP_BUILDDIR/nrm-nvml
	#  default should be nrm.nvml.power
	grep nrm.nvml.power $BATS_FILE_TMPDIR/nrmd-stderr.log
}

@test "one signal" {
	nvidia-smi -L 2>/dev/null | grep -q GPU || skip "no NVIDIA GPU available"
	run -143 timeout --preserve-status 5 $ABS_TOP_BUILDDIR/nrm-nvml -e nrm.nvml.power
	grep nrm.nvml.power $BATS_FILE_TMPDIR/nrmd-stderr.log
}

@test "two signals" {
	nvidia-smi -L 2>/dev/null | grep -q GPU || skip "no NVIDIA GPU available"
	run -143 timeout --preserve-status 5 $ABS_TOP_BUILDDIR/nrm-nvml -e nrm.nvml.power -e nrm.nvml.temperature
	grep nrm.nvml.power $BATS_FILE_TMPDIR/nrmd-stderr.log
	grep nrm.nvml.temperature $BATS_FILE_TMPDIR/nrmd-stderr.log
}

teardown_file() {
	run kill $NRM_SETUP_PID
	run pkill -9 nrm
}
