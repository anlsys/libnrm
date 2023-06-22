#!/usr/bin/env bats
# vim: set ft=bash:

@test "--version works" {
	nrmd --version
}

@test "exit works" {
	timeout 3 nrmd &>/dev/null 3>&- &
	NRMD_PID=$!
	sleep 0.5
	run nrmc -q exit
	[ "$status" -eq 0 ]
	wait $NRMD_PID
	[ "$?" -eq 0 ]
}
