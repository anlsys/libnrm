# Copyright 2019 UChicago Argonne, LLC.
# (c.f. AUTHORS, LICENSE)
#
# This file is part of the libnrm project.
# For more info, see https://github.com/anlsys/libnrm
#
# SPDX-License-Identifier: BSD-3-Clause

from nrm import Client, Setup, Actuator, Sensor, Scope, Slice, nrm_time_fromns
import unittest
from unittest.mock import Mock
import os
import time

options = {"prefix": os.environ.get("ABS_TOP_BUILDDIR")}


class TestClient(unittest.TestCase):
    def test_client_init(self):
        with Setup("nrmd", options=options):
            client = Client()
            del client

    def test_append_list_del_sensor(self):
        with Setup("nrmd", options=options):
            client = Client()
            client.add_sensor("test_sensor")
            sensors = client.list_sensors()
            assert len(sensors)
            assert isinstance(sensors[0], Sensor)
            assert sensors[0].get_uuid() == "test_sensor"

    def test_append_list_del_actuator(self):
        with Setup("nrmd", options=options):
            client = Client()
            client.add_actuator("test_actuator")
            actuators = client.list_actuators()
            assert len(actuators)
            assert isinstance(actuators[0], Actuator)
            assert actuators[0].get_uuid() == "test_actuator"

    def test_append_list_del_scope(self):
        with Setup("nrmd", options=options):
            client = Client()
            client.add_scope("test_scope")
            scopes = client.list_scopes()
            assert len(scopes)
            assert all([isinstance(scope, Scope) for scope in scopes])
            assert scopes[0].get_uuid() == "nrm.hwloc.Machine.0"
            assert scopes[-1].get_uuid() == "test_scope"

    def test_append_list_del_slice(self):
        with Setup("nrmd", options=options):
            client = Client()
            client.add_slice("test_slice")
            slices = client.list_slices()
            assert len(slices)
            assert isinstance(slices[0], Slice)
            assert slices[0].get_uuid() == "test_slice"

    def test_actuator_values_from_extra(self):
        with Setup("nrmd", options=options), Setup(
            "nrm-dummy-extra", options=options
        ):
            client = Client()
            actuators = client.list_actuators()
            assert len(actuators)
            dummy_act = actuators[0]
            assert dummy_act.get_uuid() == "nrm-dummy-extra-actuator"
            assert dummy_act.get_value() == 0.0
            assert dummy_act.list_choices() == [0.0, 1.0]
            assert len(dummy_act.get_clientid())

    def test_listen(self):
        with Setup("nrmd", options=options), Setup(
            "nrm-dummy-extra", options=options
        ):
            client = Client()

            mock = Mock(return_value=None)
            client.set_event_listener(mock)
            client.start_event_listener("")
            time.sleep(1)
            mock.assert_called()

    def test_send_event(self):
        with Setup("nrmd", options=options):
            client = Client()
            sensor = client.add_sensor("test_sensor")
            scope = client.add_scope("test_scope")
            now = nrm_time_fromns(time.time_ns())
            client.send_event(now, sensor, scope, 1.0)

    def test_actuate(self):
        with Setup("nrmd", options=options), Setup(
            "nrm-dummy-extra", options=options
        ):
            client = Client()
            act = client.list_actuators()[0]
            newval = act.list_choices()[-1]
            client.actuate(act, newval)
            assert client.list_actuators()[0].get_value() == newval
            oldval = act.list_choices()[0]
            client.actuate(act, oldval)
            assert client.list_actuators()[0].get_value() == oldval


if __name__ == "__main__":
    unittest.main()
