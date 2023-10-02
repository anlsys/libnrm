#!/bin/sh

set -e
set -u

DATE=$1
LOGFILE=$DATE.log
LOGDIR=$DATE

# find out how many pcaps were measured
pcaps=`grep '^xp start .* power' $LOGFILE | awk '{print $5}' | sort -u`

# same thing for repeats
repeats=`grep '^xp start .* power' $LOGFILE | awk '{print $6}' | sort -u`

# the goal is to create a file with on each line:
# "pcap repeatno progress start_energy stop_energy exectime lookups/s"
lines=1
for pcap in $pcaps
do
	for i in $repeats
	do
		appruntime=`grep Runtime $LOGDIR/app.$pcap.$i.log | cut -d' ' -f 6`
		progress=`grep progress $LOGDIR/nrmd-stderr.log | awk "NR==$lines {print \\\$4 }" | sed -e's/,//g'`
		start_time=`grep '^xp start' $LOGFILE | grep "power $pcap $i" | cut -d' ' -f 3`
		end_time=`grep '^xp end' $LOGFILE | grep "power $pcap $i" | cut -d' ' -f 3`
		start_pkg0=`grep -A 4 "^xp start .* power $pcap $i" $LOGFILE | awk 'NR==2 { print $1 }'`
		start_pkg1=`grep -A 4 "^xp start .* power $pcap $i" $LOGFILE | awk 'NR==3 { print $1 }'`
		end_pkg0=`grep -A 4 "^xp start .* power $pcap $i" $LOGFILE | awk 'NR==4 { print $1 }'`
		end_pkg1=`grep -A 4 "^xp start .* power $pcap $i" $LOGFILE | awk 'NR==5 { print $1 }'`
		echo "$pcap $i $progress $start_pkg0 $end_pkg0 $start_pkg1 $end_pkg1 $start_time $end_time $appruntime"
		lines=$(($lines+1))
	done
done
