#!/usr/bin/env bats
# vim: set ft=bash:

@test "--version works" {
	nrmd --version
}

@test "exit works" {
	timeout 5 nrmd &>/dev/null 3>&- &
	NRMD_PID=$!
	sleep 1
	timeout 2 nrmc exit
	[ "$?" -eq 0 ]
	wait $NRMD_PID
	[ "$?" -eq 0 ]
}
