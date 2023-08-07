#!/usr/bin/env python3
# vim: set ft=python3

import subprocess
import argparse
import pathlib
import signal
import time
import os


class NRMBinary:
    def __init__(self, name, candie=False):
        self.name = name
        self.candie = candie

    def launch(self, options):
        self.abspath = (options.prefix / self.name).absolute()
        self.stdout = open(options.output / (self.name + "-stdout.log"), "w+")
        self.stderr = open(options.output / (self.name + "-stderr.log"), "w+")
        assert self.stdout != None
        assert self.stderr != None
        self.proc = subprocess.Popen(
            [self.abspath, "-vvv"], stdin=None, stdout=self.stdout, stderr=self.stderr
        )
        assert self.proc

    def isdead(self):
        return self.proc.poll() is not None

    def kill(self):
        if self.candie:
            os.kill(self.proc.pid, signal.SIGTERM)
        else:
            os.kill(self.proc.pid, signal.SIGKILL)


class NRMSetup:
    def __init__(self):
        self.returncode = 0
        self.others = []

    def launch(self, args, nrmd, others):
        nrmd.launch(args)
        self.nrmd = nrmd

        for c in others:
            c.launch(args)
            self.others.append(c)

    def wait(self):
        self.done = False
        while not self.done:
            time.sleep(1)
            if self.nrmd.isdead():
                self.done = True

    def exit_handler(self, *args):
        print("caught signal, exiting")
        if not self.nrmd.isdead():
            self.nrmd.kill()
        for c in self.others:
            if not c.isdead():
                c.kill()
        self.done = True

    def child_handler(self, *args):
        # only one child so exit
        print("child terminated, exiting", args)
        self.done = True

    def cleanup(self):
        ret = self.nrmd.proc.poll()
        self.returncode = self.returncode or ret
        for c in self.others:
            if not c.isdead():
                c.kill()
        return self.returncode


# Make a program that:
# - launches nrmd, nrm-dummy-extra, all the other sensors,
# - setup log redirection to proper log files
# - listen to SIGTERM and clean everything

parser = argparse.ArgumentParser(
    prog="nrm-setup", description="setup nrmd and its utils"
)

parser.add_argument("-o", "--output", type=pathlib.Path, default=".")
parser.add_argument("-p", "--prefix", type=pathlib.Path, default=".")
parser.add_argument("--dummy", action="store_true")

args = parser.parse_args()
print(args)

nrmd = NRMBinary("nrmd", True)

handler = NRMSetup()
signal.signal(signal.SIGTERM, handler.exit_handler)
signal.signal(signal.SIGINT, handler.exit_handler)
signal.signal(signal.SIGCHLD, handler.child_handler)

tolaunch = []
if args.dummy:
    nrm_dummy_extra = NRMBinary("nrm-dummy-extra", False)
    tolaunch.append(nrm_dummy_extra)

handler.launch(args, nrmd, tolaunch)
handler.wait()
err = handler.cleanup()
exit(err)
