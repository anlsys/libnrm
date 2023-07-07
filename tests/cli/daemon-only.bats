#!/usr/bin/env bats
# vim: set ft=bash:

launch_daemon() {
	nrmd &>/dev/null 3>&- &
	NRMD_PID=$!
	# wait until we are sure that the daemon is running
	for i in `seq 1 5`; do
		run timeout 0.2 nrmc -q list-sensors
		if [ "$status" -eq 0 ]; then
			break
		fi
	done
	[ "$status" -eq 0 ]
}

setup() {
	launch_daemon
}

@test "exit works" {
	# completely ignore any exit status on nrmc
	run timeout 1 nrmc exit
	wait $NRMD_PID
	[ "$?" -eq 0 ]
}

@test "--freq works" {
	run timeout 2 nrm-dummy-extra --freq 0.5
	# TODO check number of events	
	kill $NRMD_PID
}

teardown_file() {
	run pkill -9 nrm
}
