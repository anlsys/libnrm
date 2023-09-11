#!/usr/bin/env python2

import pandas as pd
import sys
import scipy.optimize as opt
import numpy as np

###############################################################################
# Init
###############################################################################

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


###############################################################################
# Power Model
###############################################################################

pmin = min(frame['pcap'])
pmax = max(frame['pcap'])
power_parameters0 = [1, 0] 

def powermodel(power_requested, slope, offset):
    return slope*power_requested+offset

power_params, power_params_cov = opt.curve_fit(powermodel, frame['pcap'], frame['power'], p0=power_parameters0)
power_model = powermodel(frame['pcap'].loc[pmin:pmax], power_params[0], power_params[1])

###############################################################################
# Progress Model
###############################################################################

# Getting K_L, alpha, beta
sc = {}
sc_requested = {}
pcap2perf_model = {}
power2perf_params = {}

elected_performance_sensor = 'progress_frequency_median' # choose between: 'average_performance_periods' 'average_progress_count' 'average_performance_frequency'
for cluster in clusters:
    sc[cluster] = pd.DataFrame([data[cluster][trace]['aggregated_values'][elected_performance_sensor]['median'].mean() for trace in traces[cluster][0]], index=[data[cluster][trace]['aggregated_values']['rapls'] for trace in traces[cluster][0]], columns=[elected_performance_sensor])
    sc[cluster].sort_index(inplace=True)
    sc_requested[cluster] = pd.DataFrame([data[cluster][trace]['aggregated_values'][elected_performance_sensor]['median'].mean() for trace in traces[cluster][0]], index=[data[cluster][trace]['parameters']['config-file']['actions'][0]['args'][0] for trace in traces[cluster][0]], columns=[elected_performance_sensor])
    sc_requested[cluster].sort_index(inplace=True)

def power2perf(power, alpha, perf_inf, power_0): # general model formulation
    return perf_inf*(1-np.exp(-alpha*(power-power_0)))

def pcap2perf(pcap, a, b, perf_inf, alpha, power_0): # general model formulation
    return perf_inf*(1-np.exp(-alpha*(a*pcap+b-power_0)))

# Model optimisation 
for cluster in clusters:
    # init param
    power2perf_param0 = [0.04, (sc[cluster].at[sc[cluster].index[-1],elected_performance_sensor]+sc[cluster].at[sc[cluster].index[-2],elected_performance_sensor]+sc[cluster].at[sc[cluster].index[-3],elected_performance_sensor])/3, min(sc[cluster].index)]                                        # guessed params
    # Optimization
    # print(cluster)
    power2perf_param_opt, power2perf_param_cov = opt.curve_fit(power2perf, sc[cluster].index, sc[cluster][elected_performance_sensor], p0=power2perf_param0, maxfev=2000) # add optional parameter maxfev=5000 if the curve is not converging.     
    power2perf_params[cluster] = power2perf_param_opt
    # Model
    pcap2perf_model[cluster] = pcap2perf(sc_requested[cluster].index, power_parameters[cluster][0], power_parameters[cluster][1], power2perf_params[cluster][1], power2perf_params[cluster][0], power2perf_params[cluster][2]) # model with optimized perfinf

