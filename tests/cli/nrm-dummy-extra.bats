#!/usr/bin/env bats
# vim: set ft=bash:

@test "--version works" {
	nrm-dummy-extra --version
}

@test "--help works" {
	nrm-dummy-extra --help
}

@test "bad freq" {
	run ! nrm-dummy-extra --freq aa
}

@test "--freq works" {
	nrmd &>/dev/null 3>&- &
	NRMD_PID=$!
	sleep 0.1

	run timeout 2 nrm-dummy-extra --freq 0.5
	# TODO check number of events	

	kill $NRMD_PID
}
