#!/bin/sh

set -e
set -x
set -u

DATE=$1
LOGFILE=$DATE.log
LOGDIR=$DATE

# find out how many pcaps were measured
pcaps=`grep '^power' $LOGFILE | cut -d' ' -f 2 | sort -u`
echo "list of pcaps: $pcaps"

# same thing for repeats
repeats=`grep '^power' $LOGFILE | cut -d' ' -f 3 | sort -u`
echo "list of repeats: $repeats"

# the goal is to create a file with on each line:
# "pcap repeatno progress start_energy stop_energy exectime lookups/s"
for pcap in $pcaps
do
	for i in $repeats
	do
		lookups=`grep Lookups/s $LOGDIR/app.$pcap.$i.log | cut -d' ' -f 4 | sed -e 's/,//g'`
		runtime=`grep Runtime $LOGDIR/app.$pcap.$i.log | cut -d' ' -f 6`
		progress=`grep nrm-ompt $LOGDIR/$pcap.$i.log | wc -l`
		start_pkg0=`grep -m 1 nrm.geopm.CPU_ENERGY.package.0 $LOGDIR/$pcap.$1.log | cut -d' ' -f 5`
		start_pkg1=`grep -m 1 nrm.geopm.CPU_ENERGY.package.1 $LOGDIR/$pcap.$1.log | cut -d' ' -f 5`
		end_pkg0=`grep nrm.geopm.CPU_ENERGY.package.0 $LOGDIR/$pcap.$1.log | tail -n 1 | cut -d' ' -f 5`
		end_pkg1=`grep nrm.geopm.CPU_ENERGY.package.1 $LOGDIR/$pcap.$1.log | tail -n 1 | cut -d' ' -f 5`
		echo "$pcap $i $progress $start_pkg0 $end_pkg0 $start_pkg1 $end_pkg1 $runtime $lookups"
	done
done
