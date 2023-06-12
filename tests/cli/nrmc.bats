#!/usr/bin/env bats

@test "--version works" {
	nrmc --version
}

@test "--help works" {
	nrmc --help
}
