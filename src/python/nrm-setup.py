#!/usr/bin/env python3
# vim: set ft=python3

import subprocess
import argparse
import pathlib
import signal
import time
import os

class NRMSetup:
    def __init__(self):
        self.returncode = 0

    def launch(self, args):
        nrmd_path = (args.prefix / "nrmd").absolute()
        self.nrmd_stdout = open(args.output / "nrmd-stdout.log", 'w+')
        assert self.nrmd_stdout != None
        self.nrmd_stderr = open(args.output / "nrmd-stderr.log", 'w+')
        assert self.nrmd_stderr != None
        self.nrmd = subprocess.Popen([nrmd_path, '-vvv'], stdin=None,
                                     stdout=self.nrmd_stdout,
                                     stderr=self.nrmd_stderr)
        assert self.nrmd

    def wait(self):
        self.done = False
        while not self.done:
            time.sleep(1)
            if self.nrmd.poll() is not None:
                self.done = True

    def exit_handler(self, *args):
        print("caught signal, exiting")
        if self.nrmd.poll() is None:
            os.kill(self.nrmd.pid, signal.SIGTERM)
        self.done = True

    def child_handler(self, *args):
        # only one child so exit
        print("child terminated, exiting")
        self.done = True

    def cleanup(self):
        ret = self.nrmd.poll()
        self.returncode = self.returncode or ret
        return self.returncode


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

handler = NRMSetup()
signal.signal(signal.SIGTERM, handler.exit_handler)
signal.signal(signal.SIGINT, handler.exit_handler)
signal.signal(signal.SIGCHLD, handler.child_handler)

handler.launch(args)
handler.wait()
err = handler.cleanup()
exit(err)
