#!/usr/bin/env python2

import pandas as pd
import sys

parse_file = sys.argv[1]+".parse"

frame = pd.read_csv(parse_file, delim_whitespace=True, names=['pcap', 'rep', 'progress' 'start_pkg0' 'end_pkg0', 'start_pkg1', 'end_pkg1', 'runtime', 'lookups'])

frame['total_energy'] = frame['end_pkg0'] - frame['start_pkg0'] +
                        frame['end_pkg1'] - frame['start_pkg1']

frame['power'] = frame['total_energy'] / frame['runtime']
frame.progress =  frame.progress / frame.runtime

grouped = frame.groupby(['pcap'])

for pcap in list(set(frame['pcap'])):
    g = grouped.get_group(pcap)
    print pcap, g.runtime.mean(), g.power.mean(), g.progress.mean()
