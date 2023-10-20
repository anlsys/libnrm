from loguru import logger
from ctypes import byref
from dataclasses import dataclass
from bindings import NRM, nrm_client


@dataclass
class Client:
    """Client class for interacting with NRM C interface. Use as a context switcher.
    Tentative usage:
    ```
    from nrm.api import Client, Actuator
    with Client("tcp://127.0.0.1", 2345, 3456) as nrmc:
        ...
        nrmc.scopes["uuid"] = my_scope

        ...
        nrmc.send_event(sensor, scope, 1234)
        ...

    ```
    """

    def __enter__(
        self, uri: str = "tcp://127.0.0.1", pub_port: int = 2345, rpc_port: int = 3456
    ):
        self.uri = uri
        self.pub_port = pub_port
        self.rpc_port = rpc_port
        self.client = nrm_client(0)
        assert not NRM.nrm_init(
            None, None
        ), "NRM library did not initialize successfully"
        logger.debug("NRM initialized")

        assert not NRM.nrm_client_create(
            byref(self.client), bytes(uri, "utf-8"), pub_port, rpc_port
        ), "Unable to instantiate an NRM client"
        logger.debug("NRM client created")

    def __exit__(self, exc_type, exc_value, traceback):
        logger.debug("Destroying Client instance")
        NRM.nrm_client_destroy(byref(self.client))
        assert not NRM.nrm_finalize(), "Unable to exit NRM"
