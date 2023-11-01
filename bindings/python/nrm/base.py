# Copyright 2019 UChicago Argonne, LLC.
# (c.f. AUTHORS, LICENSE)
#
# This file is part of the libnrm project.
# For more info, see https://github.com/anlsys/libnrm
#
# SPDX-License-Identifier: BSD-3-Clause

import ctypes as ct
import enum
from . import libnrm

# Base types

nrm_int = ct.c_int
nrm_uint = ct.c_uint
nrm_long = ct.c_longlong
nrm_str = ct.c_char_p
nrm_client = ct.c_void_p

# Error types


@enum.unique
class Result(enum.Enum):
    SUCCESS = 0
    FAILURE = -1
    ENOMEM = -2
    EINVAL = -3
    EDOM = -4
    ENOTSUP = -5
    EBUSY = -6
    EPERM = -7


class Error(Exception):
    def __init__(self, code):
        self._code = code
        super().__init__(code)

    @classmethod
    def check(cls, result, func, args):
        ret = Result(result)
        if ret.value < 0:
            raise cls(ret)
        return ret


# Utils


def _nrm_get_checked_function(
    method, argtypes=[], restype=nrm_int, errcheck=Error.check
):
    res = getattr(libnrm, method)
    res.restype = restype
    res.argtypes = argtypes
    res.errcheck = errcheck
    return res


def _nrm_get_void_function(method, argtypes=[]):
    res = getattr(libnrm, method)
    res.restype = None
    return res


nrm_init = _nrm_get_checked_function(
    "nrm_init", [ct.POINTER(nrm_int), ct.POINTER(nrm_str)]
)
nrm_finalize = _nrm_get_void_function("nrm_finalize")


def _init_and_register_exit():
    import atexit

    atexit.register(nrm_finalize)
    nrm_init(None, None)


_init_and_register_exit()
del _init_and_register_exit


# Global Variables

upstream_uri = nrm_str.in_dll(libnrm, "nrm_upstream_uri")
upstream_rpc_port = nrm_uint.in_dll(libnrm, "nrm_upstream_rpc_port")
upstream_pub_port = nrm_uint.in_dll(libnrm, "nrm_upstream_pub_port")
ratelimit = nrm_long.in_dll(libnrm, "nrm_ratelimit")
transmit = nrm_int.in_dll(libnrm, "nrm_transmit")
timeout = nrm_uint.in_dll(libnrm, "nrm_timeout")
