"""
/*******************************************************************************
 * Copyright 2023 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/
"""

import sys
import logging as logger
from ctypes import byref
from dataclasses import dataclass
from .bindings import nrm_client_create, nrm_client_destroy, NRM_SUCCESS, NRM_EINVAL, NRM_ENOMEM

def _determine_status(flag, good_message="Success"):
    if flag == NRM_SUCCESS:
        logger.debug(good_message)
        return
    elif flag == -NRM_EINVAL:
        logger.error("Invalid argument")
    elif flag == -NRM_ENOMEM:
        logger.error("Not enough memory")
    sys.exit(flag)

@dataclass
class Client:
    """Client class for interacting with NRM C interface. Tentative usage:
    ```
    from nrm import Client, Actuator

    my_client = Client("tcp://127.0.0.1", 2345, 3456)
    with Client("tcp://127.0.0.1", 2345, 3456) as nrmc:
        ...
        nrmc.scopes["uuid"] = my_scope

        ...
        nrmc.send_event(sensor, scope, 1234)
        ...

    ```
    """

    def __init__(
        self, uri: str = "tcp://127.0.0.1", pub_port: int = 2345, rpc_port: int = 3456
    ):
        self.uri = uri
        self.pub_port = pub_port
        self.rpc_port = rpc_port
        self.client = nrm_client(0)

        flag =  NRM.nrm_client_create(
            byref(self.client), bytes(uri, "utf-8"), pub_port, rpc_port
        )
        _determine_status(flag, "NRM Client created")


    def __exit__(self, exc_type, exc_value, traceback):
        logger.debug("Destroying Client instance")
        NRM.nrm_client_destroy(byref(self.client))
