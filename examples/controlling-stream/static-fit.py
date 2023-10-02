#!/usr/bin/env python2

import pandas as pd
import sys
import scipy.optimize as opt
import numpy as np

###############################################################################
# Init
###############################################################################

parse_file = sys.argv[1]+".parse"

frame = pd.read_csv(parse_file, delim_whitespace=True, names=['pcap', 'rep',
                                                              'progress'
                                                              'start_pkg0'
                                                              'end_pkg0',
                                                              'start_pkg1',
                                                              'end_pkg1',
                                                              'start_time',
                                                              'end_time','runtime',
                                                              'perf'])

# deal with wrap around on rapl counters
energy_max = 262143328850 / 1000000

frame.loc[frame['end_pkg0'] < frame['start_pkg0'],'end_pkg0'] += energy_max
frame.loc[frame['end_pkg1'] < frame['start_pkg1'],'end_pkg1'] += energy_max

frame['total_energy'] = frame['end_pkg0'] - frame['start_pkg0'] +
                        frame['end_pkg1'] - frame['start_pkg1']

frame['exectime'] = (frame['end_time'] - frame['start_time'])
frame['power'] = frame['total_energy'] / frame['exectime']

# two sockets
frame.power = frame.power / 2
# per second
frame.runtime = frame.runtime / 1e9

grouped = frame.groupby(['pcap'])

print("Power Cap, Exectime, Runtime, Power, Progress")
for pcap in list(set(frame['pcap'])):
    g = grouped.get_group(pcap)
    print pcap, g.exectime.mean(), g.runtime.mean(), g.power.mean(), g.progress.mean()


###############################################################################
# Power Model
###############################################################################

pmin = min(frame['pcap'])
pmax = max(frame['pcap'])
power_parameters0 = [1, 0] 

def powermodel(power_requested, slope, offset):
    return slope*power_requested+offset

power_params, power_params_cov = opt.curve_fit(powermodel, frame['pcap'], frame['power'], p0=power_parameters0)
print("RAPL slope:  a:", str(round(power_params[0], 2)))
print("RAPL offset: b:", str(round(power_params[1], 2)))

###############################################################################
# Progress Model
###############################################################################

def power2perf(power, alpha, perf_inf, power_0): # general model formulation
    return perf_inf*(1-np.exp(-alpha*(power-power_0)))

power2perf_param0 = [0.04, frame.progress.mean(), frame.power.min()]
print(power2perf_param0)
power2perf_params, power2perf_param_cov = opt.curve_fit(power2perf, frame['power'], frame['progress'], p0=power2perf_param0, maxfev=10000)

print("Model α:", str(round(power2perf_params[0], 3)))
print("Power offset: β:", str(round(power2perf_params[1], 1)))
print("Linear Gain: K_l:", str(round(power2perf_params[2], 1)))
