#!/usr/bin/env bats

setup() {
	PATH=$TOP_BUILDDIR:$PATH
}

@test "--version works" {
	nrmc --version
}
