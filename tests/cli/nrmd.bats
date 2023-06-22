#!/usr/bin/env bats
# vim: set ft=bash:

@test "--version works" {
	nrmd --version
}

@test "exit works" {
	nrmd &>/dev/null 3>&- &
	NRMD_PID=$!
	sleep 0.1
	# completely ignore any exit status on nrmc
	run timeout 1 nrmc exit
	wait $NRMD_PID
	[ "$?" -eq 0 ]
}
