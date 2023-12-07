# Copyright 2019 UChicago Argonne, LLC.
# (c.f. AUTHORS, LICENSE)
#
# This file is part of the libnrm project.
# For more info, see https://github.com/anlsys/libnrm
#
# SPDX-License-Identifier: BSD-3-Clause

import os
import logging
import subprocess
from pathlib import Path
from typing import List, Union, Optional
from dataclasses import dataclass
from ctypes import byref, POINTER, c_void_p, c_double

from .base import (
    Error,
    _nrm_get_function,
    _nrm_vector_to_list_by_type,
    nrm_client,
    nrm_str,
    nrm_uuid,
    nrm_double,
    nrm_uint,
    nrm_vector,
    nrm_sensor,
    nrm_actuator,
    nrm_scope,
    nrm_slice,
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

nrm_client_list_scopes = _nrm_get_function(
    "nrm_client_list_scopes", [nrm_client, POINTER(nrm_vector)]
)

nrm_client_list_slices = _nrm_get_function(
    "nrm_client_list_slices", [nrm_client, POINTER(nrm_vector)]
)

nrm_client_add_sensor = _nrm_get_function(
    "nrm_client_add_sensor", [nrm_client, nrm_sensor]
)

nrm_client_add_actuator = _nrm_get_function(
    "nrm_client_add_actuator", [nrm_client, nrm_actuator]
)

nrm_client_add_scope = _nrm_get_function(
    "nrm_client_add_scope", [nrm_client, nrm_scope]
)

nrm_client_add_slice = _nrm_get_function(
    "nrm_client_add_slice", [nrm_client, nrm_slice]
)

nrm_sensor_create = _nrm_get_function(
    "nrm_sensor_create", [nrm_str], nrm_sensor, Error.checkptr
)

nrm_actuator_create = _nrm_get_function(
    "nrm_actuator_create", [nrm_str], nrm_actuator, Error.checkptr
)

nrm_scope_create = _nrm_get_function(
    "nrm_scope_create", [nrm_str], nrm_scope, Error.checkptr
)

nrm_slice_create = _nrm_get_function(
    "nrm_slice_create", [nrm_str], nrm_slice, Error.checkptr
)

nrm_sensor_destroy = _nrm_get_function(
    "nrm_sensor_destroy", [POINTER(nrm_sensor)], None, None
)

nrm_actuator_destroy = _nrm_get_function(
    "nrm_actuator_destroy", [POINTER(nrm_actuator)], None, None
)

nrm_scope_destroy = _nrm_get_function("nrm_scope_destroy", [nrm_scope])

nrm_slice_destroy = _nrm_get_function(
    "nrm_slice_destroy", [POINTER(nrm_slice)], None, None
)

nrm_scope_uuid = _nrm_get_function(
    "nrm_scope_uuid", [nrm_scope], nrm_str, Error.checkptr
)

nrm_sensor_uuid = _nrm_get_function(
    "nrm_sensor_uuid", [nrm_sensor], nrm_str, Error.checkptr
)

nrm_slice_uuid = _nrm_get_function(
    "nrm_slice_uuid", [nrm_slice], nrm_str, Error.checkptr
)

nrm_actuator_uuid = _nrm_get_function(
    "nrm_actuator_uuid", [nrm_actuator], nrm_str, Error.checkptr
)

nrm_uuid_to_string = _nrm_get_function(
    "nrm_uuid_to_string", [POINTER(nrm_uuid)], nrm_str, Error.checkptr
)

nrm_actuator_clientid = _nrm_get_function(
    "nrm_actuator_clientid", [nrm_actuator], POINTER(nrm_uuid), Error.checkptr
)

nrm_actuator_value = _nrm_get_function(
    "nrm_actuator_value", [nrm_actuator], c_double, None
)

nrm_actuator_list_choices = _nrm_get_function(
    "nrm_actuator_list_choices", [nrm_actuator, POINTER(nrm_vector)]
)

_logger = logging.getLogger("nrm")

@dataclass
class _ClientExec:
    cmd: list
    stdout: Path
    stderr: Path
    process: subprocess.Popen = None
    errcode: int = -1
    preloads: str = ""

    def _setup_preloads(self, preloads):
        preloads = ":".join([str(Path(path).absolute()) for path in preloads])
        if len(preloads):
            os.environ["LD_PRELOAD"] = preloads
            self.preloads = preloads

class Client:
    """Client class for interacting with NRM C interface.
    Tentative usage:
    ```
    from nrm import Client, Actuator

    my_client = Client("tcp://127.0.0.1", 2345, 3456)
    ...
    my_client.append_actuator(my_actuator)

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
        self.runs = []

        if isinstance(self.uri, str):
            self.uri = bytes(self.uri, "utf-8")

        nrm_client_create(
            byref(self.client), self.uri, self.pub_port, self.rpc_port
        )

    def run(self, cmd: Union[str, List[str]], preloads: List[Union[str, Path]] = []):
        cmd = [cmd] if isinstance(cmd, str) else cmd
        execcmd = _ClientExec(cmd=cmd, stdout=cmd[0]+".out", stderr=cmd[0]+".err")
        execcmd._setup_preloads(preloads)
        try:
            with open(execcmd.stdout, "w") as stdout, open(execcmd.stderr, "w") as stderr:
                _logger.debug("Launching " + str(cmd))
                execcmd.process = subprocess.Popen(cmd, stdout=stdout, stderr=stderr)
                self.runs.append(execcmd)
        except Exception as e:
            _logger.error("Error on launch: ", e.__class__, e.args)
            raise e

    def papi_run(self, cmd: Union[str, List[str]], events: List[str] = ["PAPI_TOT_INS"], freq: float = 1.0):
        cmd = [cmd] if isinstance(cmd, str) else cmd
        Client._setup_preloads(preloads)
        pass

    def list_sensors(self) -> list:
        vector = nrm_vector(0)
        nrm_client_list_sensors(self.client, byref(vector))
        pylist = _nrm_vector_to_list_by_type(vector, nrm_sensor)
        return [Sensor(i) for i in pylist]

    def list_actuators(self) -> list:
        vector = nrm_vector(0)
        nrm_client_list_actuators(self.client, byref(vector))
        pylist = _nrm_vector_to_list_by_type(vector, nrm_actuator)
        return [Actuator(i) for i in pylist]

    def list_scopes(self) -> list:
        vector = nrm_vector(0)
        nrm_client_list_scopes(self.client, byref(vector))
        pylist = _nrm_vector_to_list_by_type(vector, nrm_sensor)
        return [Scope(i) for i in pylist]

    def list_slices(self) -> list:
        vector = nrm_vector(0)
        nrm_client_list_slices(self.client, byref(vector))
        pylist = _nrm_vector_to_list_by_type(vector, nrm_slice)
        return [Slice(i) for i in pylist]

    def add_sensor(self, name: str):
        sensor = nrm_sensor_create(bytes(name, "utf-8"))
        nrm_client_add_sensor(self.client, sensor)

    def add_actuator(self, name: str):
        actuator = nrm_actuator_create(bytes(name, "utf-8"))
        nrm_client_add_actuator(self.client, actuator)

    def add_scope(self, name: str):
        scope = nrm_scope_create(bytes(name, "utf-8"))
        nrm_client_add_scope(self.client, scope)

    def add_slice(self, name: str):
        nslice = nrm_slice_create(bytes(name, "utf-8"))  # "slice" is builtin
        nrm_client_add_slice(self.client, nslice)

    def __del__(self):
        nrm_client_destroy(byref(self.client))


@dataclass
class _NRMComponent:
    ptr: c_void_p


class Actuator(_NRMComponent):
    def get_uuid(self):
        return nrm_actuator_uuid(self.ptr).decode()

    def get_clientid(self):
        return nrm_uuid_to_string(nrm_actuator_clientid(self.ptr)).decode()

    def get_value(self):
        return nrm_actuator_value(self.ptr)

    def list_choices(self):
        vector = nrm_vector(0)
        nrm_actuator_list_choices(self.ptr, byref(vector))
        cval_list = _nrm_vector_to_list_by_type(vector, nrm_double)
        return [i.value for i in cval_list]

    def __del__(self):
        nrm_actuator_destroy(byref(self.ptr))


class Sensor(_NRMComponent):
    def get_uuid(self):
        return nrm_sensor_uuid(self.ptr).decode()

    def __del__(self):
        nrm_sensor_destroy(byref(self.ptr))


class Scope(_NRMComponent):
    def get_uuid(self):
        return nrm_scope_uuid(self.ptr).decode()

    def __del__(self):
        nrm_scope_destroy(self.ptr)


class Slice(_NRMComponent):
    def get_uuid(self):
        return nrm_slice_uuid(self.ptr).decode()

    def __del__(self):
        nrm_slice_destroy(byref(self.ptr))
