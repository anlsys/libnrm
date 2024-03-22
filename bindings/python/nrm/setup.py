# Copyright 2019 UChicago Argonne, LLC.
# (c.f. AUTHORS, LICENSE)
#
# This file is part of the libnrm project.
# For more info, see https://github.com/anlsys/libnrm
#
# SPDX-License-Identifier: BSD-3-Clause

import os
import time
import signal
import subprocess
from nrm import Client


class NRMBinary:
    def __init__(self, name, mustkill=False, waitready=None):
        self.name = name
        self.mustkill = mustkill
        self.waitready = waitready

    def launch(self, options, args):
        self.abspath = (options["prefix"] / self.name).absolute()
        self.stdout = open(
            options["output"] / (self.name + "-stdout.log"), "w+"
        )
        self.stderr = open(
            options["output"] / (self.name + "-stderr.log"), "w+"
        )
        assert self.stdout is not None
        assert self.stderr is not None
        popencmd = []
        # support for LOG_COMPILER and LOG_FLAGS
        popencmd += os.environ.get("LOG_COMPILER", "").split()
        popencmd += os.environ.get("LOG_FLAGS", "").split()
        popencmd += [self.abspath]
        # add the rest of the command line arguments
        popencmd += args
        self.proc = subprocess.Popen(
            popencmd, stdin=None, stdout=self.stdout, stderr=self.stderr
        )
        assert self.proc

        # in most cases, we should wait before launching something else
        if self.waitready:
            self.waitready(options)

    def isdead(self):
        return self.proc.poll() is not None

    def kill(self):
        if self.mustkill:
            os.kill(self.proc.pid, signal.SIGKILL)
        else:
            os.kill(self.proc.pid, signal.SIGTERM)
        self.proc.wait()
        self.stdout.close()
        self.stderr.close()


def waitretry(func, retries=5, sleeptime=1):
    def decorator(*args, **kwargs):
        for i in range(retries):
            if func(*args, **kwargs):
                break
            time.sleep(sleeptime)
        else:
            raise TimeoutError(
                f"Unable to verify start of NRM binary after {retries} seconds"
            )

    return decorator


def nrmd_waitready(options):
    nrmc_cmd = [(options["prefix"] / "nrmc").absolute(), "-q", "connect"]
    subprocess.run(nrmc_cmd, check=True)


@waitretry
def dummy_connect(*args):
    actuators = Client().list_actuators()
    return "nrm-dummy-extra-actuator" in [act.get_uuid() for act in actuators]


@waitretry
def geopm_connect(*args):
    actuators = Client().list_actuators()
    return "nrm.geopm.cpu.power" in [act.get_uuid() for act in actuators]


class Setup:
    binaries = {
        "nrmd": NRMBinary("nrmd", False, nrmd_waitready),
        "nrm-dummy-extra": NRMBinary("nrm-dummy-extra", False, dummy_connect),
        "nrm-geopm": NRMBinary("nrm-geopm", False, geopm_connect),
    }

    def __init__(self, name, args=[], options={}):
        assert name in self.binaries
        self.binary = self.binaries[name]
        import pathlib

        self.options = {}
        self.options["prefix"] = pathlib.Path(options.get("prefix", ""))
        self.options["output"] = pathlib.Path(options.get("output", ""))
        self.args = args

    def __enter__(self):
        self.binary.launch(self.options, self.args)
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if not self.binary.isdead():
            self.binary.kill()
