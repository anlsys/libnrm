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
nrm_actuator = ct.c_void_p
nrm_sensor = ct.c_void_p
nrm_scope = ct.c_void_p
nrm_slice = ct.c_void_p
nrm_vector = ct.c_void_p

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


def _nrm_get_function(method, argtypes=[], restype=nrm_int, errcheck=Error.check):
    res = getattr(libnrm, method)
    res.restype = restype
    res.argtypes = argtypes
    if errcheck is not None:
        res.errcheck = errcheck
    return res


nrm_init = _nrm_get_function("nrm_init", [ct.POINTER(nrm_int), ct.POINTER(nrm_str)])
nrm_finalize = _nrm_get_function("nrm_finalize", [], None, None)

nrm_vector_length = _nrm_get_function(
    "nrm_vector_length", [nrm_vector, ct.POINTER(nrm_int)]
)
nrm_vector_get = _nrm_get_function(
    "nrm_vector_get", [nrm_vector, nrm_int, ct.POINTER(ct.c_void_p)]
)

nrm_vector_destroy = _nrm_get_function(
    "nrm_vector_destroy", [ct.POINTER(nrm_vector), None]
)


def _nrm_vector_to_list(vector_p: nrm_vector) -> list:
    length = nrm_int(0)
    nrm_vector_length(nrm_vector, byref(length))
    pylist = []
    out = ct.c_void_p
    for i in range(length):
        nrm_vector_get(vector_p, i, byref(out))
        pylist[i] = out.value
    nrm_vector_destroy(byref(nrm_vector))
    return pylist


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
