#!/usr/bin/env bats
# vim: set ft=bash:

bats_require_minimum_version 1.5.0

@test "nrmc --version works" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc --version
	echo $output | grep nrmc
}

@test "nrmd --version works" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmd --version
	echo $output | grep nrmd
}

@test "nrm-dummy-extra --version works" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrm-dummy-extra --version
	echo $output | grep nrm-dummy-extra
}

@test "nrm-papiwrapper --version works" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrm-papiwrapper --version
	echo $output | grep nrm-papiwrapper
}

@test "nrmc --help works" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc --help
	echo $output | grep nrmc
}

@test "nrmd --help works" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmd --help
	echo $output | grep nrmd
}

@test "nrm-dummy-extra --help works" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrm-dummy-extra --help
	echo $output | grep nrm-dummy-extra
}

@test "nrm-papiwrapper --help works" {
	run -0 --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrm-papiwrapper --help
	echo $output | grep nrm-papiwrapper
}

@test "nrmc wrong extra option" {
	run ! --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmc --freq 10
}

@test "nrmd wrong extra option" {
	run ! --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrmd --freq 10
}

@test "nrm-dummy-extra bad freq" {
	run ! --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrm-dummy-extra --freq aa
}

@test "nrm-papiwrapper bad freq" {
	run ! --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrm-papiwrapper --freq aa
}

@test "nrm-papiwrapper no command" {
	run ! --separate-stderr $LOG_COMPILER $LOG_FLAGS $ABS_TOP_BUILDDIR/nrm-papiwrapper
}
