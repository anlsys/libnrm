# Copyright 2019 UChicago Argonne, LLC.
# (c.f. AUTHORS, LICENSE)
#
# This file is part of the libnrm project.
# For more info, see https://github.com/anlsys/libnrm
#
# SPDX-License-Identifier: BSD-3-Clause

from nrm import Client, Setup, Actuator, Sensor
import unittest
import os

options = {"prefix": os.environ.get("ABS_TOP_BUILDDIR")}


class TestClient(unittest.TestCase):
    def test_client_init(self):
        with Setup("nrmd", options=options):
            client = Client()
            del client

    def test_append_sensor(self):
        with Setup("nrmd", options=options):
            client = Client()
            client.append_new_sensor("test_sensor")
            assert len(client.list_sensors())
            assert isinstance(client.list_sensors()[0], Sensor)

    def test_append_actuator(self):
        with Setup("nrmd", options=options):
            client = Client()
            client.append_new_actuator("test_actuator")
            assert len(client.list_actuators())
            assert isinstance(client.list_actuators()[0], Actuator)


if __name__ == "__main__":
    unittest.main()
