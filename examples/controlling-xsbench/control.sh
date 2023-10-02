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

REPEATS=5

export PATH=$PREFIX/bin:$PATH
export OMP_PROC_BIND=true
export OMP_NUM_THREADS=104

cat ./static.json

# Experiment
############################

mkdir -p $LOGDIR
taskset -a -c 0 nrm-setup -p $PREFIX/bin -o $LOGDIR -c ./control.json &
NRM_SETUP_PID=$!

# wait for nrm-setup to start sleeping
while [ ! -e $DATE/nrm-setup-ready.log ]; do
	sleep 1
done

# launch sensor/actuator
nrm-geopm -vvv 1>$LOGDIR/nrm-geopm-stdout.log 2>$LOGDIR/nrm-geopm-stderr.log &
NRM_GEOPM_PID=$!
sleep 5

for pcap in `nrmc find-actuator nrm.geopm.cpu.power | jq -r '.[0].choices | join("\n")'`
do
	nrmc actuate nrm.geopm.cpu.power $pcap
	for i in `seq 1 $REPEATS`;
	do
		now=`date +%s`
		echo "control start $now power $pcap $i" >> $LOGFILE
		geopmread CPU_ENERGY package 0 >> $LOGFILE
                geopmread CPU_ENERGY package 1 >> $LOGFILE
                nrm-papiwrapper -i -e PAPI_TOT_INS -f 10 XSBench -s large -l 600 -t 104 >> $LOGDIR/app.$i.log
                now=`date +%s`
                geopmread CPU_ENERGY package 0 >> $LOGFILE
                geopmread CPU_ENERGY package 1 >> $LOGFILE
                echo "control end $now power $pcap $i" >> $LOGFILE
	done
done

kill $NRM_GEOPM_PID
kill $NRM_SETUP_PID
