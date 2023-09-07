#!/bin/sh

# Script to perform data collection for the static characterization of an
# application, in line with our Europar21 methodology on modeling the
# progress/powercap relationship.
#
# Eventually we should replace this with a python script using the python
# bindings, but just using the cli tools for now should be fine.

# make sure we stop on errors, and log everything
set -x
set -u
DATE=`date +%Y%m%d.%H%M%S`
LOGFILE="$DATE.log"
LOGDIR="$DATE"
echo "Start: $DATE"
DELIMITER="################################################################################"

# remember this script in the log
echo -e "$DELIMITER\n$DELIMITER\n\n" >> $LOGFILE
cat $0 >> $LOGFILE
echo -e "\n$DELIMITER\n" >> $LOGFILE

# Gather some info on the system
uname -a >> $LOGFILE
cat /proc/cmdline >> $LOGFILE
lscpu >> $LOGFILE

echo -e "\n$DELIMITER\n" >> $LOGFILE

# save environment
env >> $LOGFILE
ulimit -n 4096
ulimit -a >> $LOGFILE

echo -e "\n$DELIMITER\n" >> $LOGFILE

# CONFIG: prefixes, commands
############################

PREFIX=$HOME/local
APPPATH=$HOME/XSBench/openmp-threading

REPEATS=5
PRELOAD_PATH=$PREFIX/lib/libnrm-ompt.so

export PATH=$PREFIX/bin:$APPPATH:$PATH

# Experiment
############################

mkdir -p $LOGDIR
nrm-setup -p $PREFIX/bin -o $LOGDIR &
NRM_SETUP_PID=$!

# wait for nrm-setup to start sleeping
while [ ! -e $DATE/nrm-setup-ready.log ]; do
	sleep 1
done

# launch sensor/actuator
nrm-geopm -vvv -f 2 1>$LOGDIR/nrm-geopm-stdout.log 2>$LOGDIR/nrm-geopm-stderr.log &
NRM_GEOPM_PID=$!
sleep 5

for pcap in `nrmc find-actuator nrm.geopm.cpu.power | jq -r '.[0].choices | join("\n")'`
do
	nrmc actuate nrm.geopm.cpu.power $pcap
	for i in `seq 1 $REPEATS`;
	do
		echo "power $pcap $i" >> $LOGFILE
		nrmc listen >> $LOGDIR/$pcap.$i.log &
		NRM_LISTEN_PID=$!
		nrmc run -d $PRELOAD_PATH XSBench -s large -l 200 >> $LOGDIR/app.$pcap.$i.log
		kill $NRM_LISTEN_PID
	done
done

kill $NRM_GEOPM_PID
kill $NRM_SETUP_PID
