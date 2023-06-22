#!/usr/bin/env bats
# vim: set ft=bash:

@test "--version works" {
	nrmd --version
}

@test "exit works" {
	timeout 5 nrmd &>/dev/null 3>&- &
	NRMD_PID=$!
	sleep 1
	run timeout 2 nrmc -q exit
	[ "$status" -eq 0 ]
	wait $NRMD_PID
	[ "$?" -eq 0 ]
}
