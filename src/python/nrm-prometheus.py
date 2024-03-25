#!/usr/bin/env python3
# vim: set ft=python3

import subprocess
import argparse
import pathlib
import signal
import time
import os
import nrm

from prometheus_client import start_http_server, Gauge

# Make a program that:
# - creates a client to nrmd
# - figures out the list of sensors available
# - exports them back to prometheus

parser = argparse.ArgumentParser(
    prog="nrm-prometheus", description="export nrmd events to prometheus"
)

parser.add_argument("-u", "--uri", type=str, default=nrm.upstream_uri)
parser.add_argument("-r", "--rpc", type=int, default=nrm.upstream_rpc_port)
parser.add_argument("-p", "--pub", type=int, default=nrm.upstream_pub_port)
parser.add_argument("-e", "--export", type=int, default=9000, help="prometheus port")

args = parser.parse_args()
print(args)

client = nrm.Client(args.uri, args.pub, args.rpc)

sensors = client.list_sensors()
sensors_nrm2prom = {}

start_http_server(args.export)

for s in sensors:
    # we don't actually know the types of these sensors, so we're going to create
    # all of them as gauges for now, making the last value easy to retrieve
    key = s.get_uuid()
    metric_name = key.replace('-','_')
    desc = "NRM Sensor for " + key
    g = Gauge(metric_name, desc)
    sensors_nrm2prom[key] = g
    print(s, g, key, metric_name, desc)

def update_sensor(sensor, time, scope, value):
    print(sensor, time, scope, value)
    # ignore scope and time
    key = sensor.decode()
    metric_name = key.replace('-','_').replace('.','_')
    desc = "NRM Sensor for " + key
    if key in sensors_nrm2prom:
        g = sensors_nrm2prom[key]
    else:
        g = Gauge(metric_name, desc)
    g.set(value)

client.set_event_listener(update_sensor)
client.start_event_listener("")

while True:
    pass
