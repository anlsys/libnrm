#!/usr/bin/env bats
# vim: set ft=bash:

launch_daemon() {
	nrmd &>/dev/null 3>&- &
	NRMD_PID=$!
	# wait until we are sure that the daemon is running
	for i in `seq 1 5`; do
		run timeout 0.2 nrmc -q list-sensors
		echo $output
		if [ "$status" -eq 0 ]; then
			break
		fi
	done
	[ "$status" -eq 0 ]
}

launch_dummy_extra() {
	nrm-dummy-extra &>/dev/null 3>&- &
	NRMD_DUMMY_PID=$!
	# make sure dummy is actually running and registered to the daemon
	for i in `seq 1 5`; do
		run nrmc -q list-sensors
		[ "$status" -eq 0 ]
		echo $output
		echo "$output" | jq .[0].uuid | grep "nrm-dummy-extra-sensor"
		if [ "$?" -eq 0 ]; then
			break
		fi
		sleep 0.1
	done
	[ "$?" -eq 0 ] 
}

setup_file() {
	launch_daemon
	launch_dummy_extra
}

@test "preload, no callbacks" {
	run nrmc -vvv run -d $ABS_TOP_BUILDDIR/.libs/libnrm-pmpi.so ls -al
	echo "$output"
	[ "$status" -eq 0 ]
}

@test "preload, basic omp" {
	run nrmc -vvv run -d $ABS_TOP_BUILDDIR/.libs/libnrm-pmpi.so ./mpi_basic
	echo "$output"
	[ "$status" -eq 0 ]
}

@test "preload, stream omp" {
	run nrmc -vvv run -d $ABS_TOP_BUILDDIR/.libs/libnrm-pmpi.so ./mpi_collectives
	echo "$output"
	[ "$status" -eq 0 ]
}

teardown_file() {
	run kill -9 $NRMD_DUMMY_PID
	run kill $NRMD_PID
}