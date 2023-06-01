#!/usr/bin/env bats

setup() {
	load 'common'
	_common_setup
	PATH=$TOP_BUILDDIR:$PATH
}

@test "--version works" {
	nrmc --version
}
