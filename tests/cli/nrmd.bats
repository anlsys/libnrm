#!/usr/bin/env bats
# vim: set ft=bash:

@test "--version works" {
	nrmd --version
}

@test "exit works" {
	timeout 10 nrmd &>/dev/null 3>&- &
	NRMD_PID=$!
	sleep 1
	timeout 5 nrmc exit
	[ "$?" -eq 0 ]
	wait $NRMD_PID
	[ "$?" -eq 0 ]
}
