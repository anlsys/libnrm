# Copyright 2019 UChicago Argonne, LLC.
# (c.f. AUTHORS, LICENSE)
#
# This file is part of the libnrm project.
# For more info, see https://github.com/anlsys/libnrm
#
# SPDX-License-Identifier: BSD-3-Clause

from ctypes import byref, POINTER, sizeof
from dataclasses import dataclass
from pydantic import BaseModel
from .base import (
    _nrm_get_function,
    _nrm_vector_to_list,
    nrm_client,
    nrm_str,
    nrm_int,
    nrm_uint,
    nrm_vector,
    nrm_sensor,
    nrm_actuator,
    upstream_uri,
    upstream_pub_port,
    upstream_rpc_port,
)

nrm_client_create = _nrm_get_function(
    "nrm_client_create", [POINTER(nrm_client), nrm_str, nrm_uint, nrm_uint]
)
nrm_client_destroy = _nrm_get_function(
    "nrm_client_destroy", [POINTER(nrm_client)], None, None
)

nrm_client_list_actuators = _nrm_get_function(
    "nrm_client_list_actuators", [nrm_client, POINTER(nrm_vector)]
)

nrm_client_list_sensors = _nrm_get_function(
    "nrm_client_list_sensors", [nrm_client, POINTER(nrm_vector)]
)

nrm_client_add_sensor = _nrm_get_function(
    "nrm_client_add_sensor", [nrm_client, nrm_sensor]
)

nrm_client_add_actuator = _nrm_get_function(
    "nrm_client_add_actuator", [nrm_client, nrm_actuator]
)

nrm_actuator_create = _nrm_get_function("nrm_actuator_create", [nrm_str], nrm_actuator, None)

nrm_sensor_create = _nrm_get_function("nrm_sensor_create", [nrm_str], nrm_sensor, None)


class Client:
    """Client class for interacting with NRM C interface.
    Tentative usage:
    ```
    from nrm import Client, Actuator

    my_client = Client("tcp://127.0.0.1", 2345, 3456)
    ...
    my_client.append_actuator(my_actuator)
    # OR???
    my_client.actuators.append(my_actuator)

    """

    def __init__(
        self,
        uri: str = None,
        pub_port: int = None,
        rpc_port: int = None,
    ):
        self.uri = uri if uri else upstream_uri
        self.pub_port = pub_port if pub_port else upstream_pub_port
        self.rpc_port = rpc_port if rpc_port else upstream_rpc_port
        self.client = nrm_client(0)

        if isinstance(self.uri, str):
            self.uri = bytes(self.uri, "utf-8")

        nrm_client_create(
            byref(self.client), self.uri, self.pub_port, self.rpc_port
        )

    @property
    def sensors(self) -> list:
        vector = nrm_vector(0)
        nrm_client_list_sensors(self.client, byref(vector))
        cval_list = _nrm_vector_to_list(vector)
        pyval_list = [Sensor(uuid=i.contents.uuid) for i in cval_list]
        return pyval_list

    def append_new_sensor(self, name: str):
        sensor = nrm_sensor_create(bytes(name, "utf-8"))
        nrm_client_add_sensor(self.client, sensor)

    @property
    def actuators(self) -> list:
        vector = nrm_vector(0)
        nrm_client_list_actuators(self.client, byref(vector))
        cval_list = _nrm_vector_to_list(vector)
        pyval_list = [
            Actuator(
                uuid=i.contents.uuid,
                clientid=str(i.contents.clientid),
                value=i.contents.value,
                choices=i.contents.choices,
            )
            for i in cval_list
        ]

    def append_new_actuator(self, name: str):
        actuator = nrm_actuator_create(bytes(name, "utf-8"))
        nrm_client_add_actuator(self.client, actuator)

    def __del__(self):
        nrm_client_destroy(byref(self.client))


class _NRMComponent(BaseModel):
    uuid: str

class Actuator(_NRMComponent):
    clientid: str
    value: float
    choices: list

class Sensor(_NRMComponent):
    pass

class Scope(_NRMComponent):
    maps: list
