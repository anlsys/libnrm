#!/usr/bin/env python3
# vim: set ft=python3

import subprocess
import argparse
import pathlib
import signal
import time
import os

# Make a program that:
# - launches nrmd, nrm-dummy-extra, all the other sensors,
# - setup log redirection to proper log files
# - listen to SIGTERM and clean everything

parser = argparse.ArgumentParser(prog="nrm-setup",
                                 description="setup nrmd and its utils")

parser.add_argument('-o', '--output', type=pathlib.Path, default='.')
parser.add_argument('-p', '--prefix', type=pathlib.Path, default='.')

args = parser.parse_args()
print(args)

# SIGTERM Handling:
# kill everything, exit loop
done = False
children = []
def signal_handler(*args):
    for c in children:
        os.kill(c.pid, signal.SIGTERM)
    done = True
signal.signal(signal.SIGTERM, signal_handler)
signal.signal(signal.SIGINT, signal_handler)

# Launch everyone
running = []
nrmd_path = (args.prefix / "nrmd").absolute()
nrm_dummy_extra_path = args.prefix / "nrm-dummy-extra"

nrmd_stdout = open(args.output / "nrmd-stdout.log", 'w+')
assert nrmd_stdout != None
nrmd_stderr = open(args.output / "nrmd-stderr.log", 'w+')
assert nrmd_stderr != None
nrmd = subprocess.Popen([nrmd_path, '-vvv'], stdin=None, stdout=nrmd_stdout,
                        stderr=nrmd_stderr)
children.append(nrmd)
running.append(nrmd)
while not done:
    time.sleep(1)
    err = 0
    newr = []
    for c in running:
        ret = c.poll()
        if ret is not None:
            print(f"{c} returned with code {ret}")
            err = err or ret
        else:
            newr.append(c)
    running = newr
    if not running:
        err = 1
        break
