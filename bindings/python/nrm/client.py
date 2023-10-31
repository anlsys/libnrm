# Copyright 2019 UChicago Argonne, LLC.
# (c.f. AUTHORS, LICENSE)
#
# This file is part of the libnrm project.
# For more info, see https://github.com/anlsys/libnrm
#
# SPDX-License-Identifier: BSD-3-Clause

from ctypes import byref, POINTER
from dataclasses import dataclass
from .base import _nrm_get_function, Error, nrm_client, nrm_str, nrm_int

nrm_client_create = _nrm_get_function(
        "nrm_client_create", [POINTER(nrm_client), nrm_str, nrm_int, nrm_int])
nrm_client_destroy = _nrm_get_function(
        "nrm_client_destroy", [POINTER(nrm_client)], None)


@dataclass
class Client:
    """Client class for interacting with NRM C interface.
    Tentative usage:
    ```
    from nrm import Client, Actuator

    my_client = Client("tcp://127.0.0.1", 2345, 3456)
    ...

    ```
    """

    def __init__(
        self, uri: str = "tcp://127.0.0.1",
        pub_port: int = 2345,
        rpc_port: int = 3456
    ):
        self.uri = uri
        self.pub_port = pub_port
        self.rpc_port = rpc_port
        self.client = nrm_client(0)

        _res = nrm_client_create(
                byref(self.client), bytes(uri, "utf-8"), pub_port, rpc_port
        )
        Error.check(_res)

    def __del__(self):
        nrm_client_destroy(byref(self.client))
