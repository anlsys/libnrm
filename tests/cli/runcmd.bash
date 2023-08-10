# vim: set ft=bash:

# wrap the bats run command to expose LOG_COMPILER and LOG_FLAGS
# this is particularly important for check-valgrind, as it will prepend
# the whole libtool --mode=execute valgrind in front of commands

# we also use ABS_TOP_BUILDDIR in the path for any nrm command, otherwise
# libtool will not replace it with the actual binary

bats_require_minimum_version 1.5.0

bats-nrm-run() {
	t=`run echo $1 | grep "nrm"`
	if [ $? -eq 0 ]; then
		cmd=$ABS_TOP_BUILDDIR/$1
	else
		cmd=$1
	fi
	run --separate-stderr $LOG_COMPILER $LOG_FLAGS $cmd ${@:2}
}
