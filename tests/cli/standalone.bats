#!/usr/bin/env bats
# vim: set ft=bash:

@test "nrmc --version works" {
	run nrmc --version
	echo $output | grep nrmc
}

@test "nrmd --version works" {
	run nrmd --version
	echo $output | grep nrmd
}

@test "nrm-dummy-extra --version works" {
	run nrm-dummy-extra --version
	echo $output | grep nrm-dummy-extra
}

@test "nrm-papiwrapper --version works" {
	run nrm-papiwrapper --version
	echo $output | grep nrm-papiwrapper
}

@test "nrmc --help works" {
	run nrmc --help
	echo $output | grep nrmc
}

@test "nrmd --help works" {
	run nrmd --help
	echo $output | grep nrmd
}

@test "nrm-dummy-extra --help works" {
	run nrm-dummy-extra --help
	echo $output | grep nrm-dummy-extra
}

@test "nrm-papiwrapper --help works" {
	run nrm-papiwrapper --help
	echo $output | grep nrm-papiwrapper
}

@test "nrmc wrong extra option" {
	run ! nrmc --freq 10
}

@test "nrmd wrong extra option" {
	run ! nrmd --freq 10
}

@test "nrm-dummy-extra bad freq" {
	run ! nrm-dummy-extra --freq aa
}

@test "nrm-papiwrapper bad freq" {
	run ! nrm-papiwrapper --freq aa
}

@test "nrm-papiwrapper no command" {
	run ! nrm-papiwrapper
}