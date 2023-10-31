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
    def check(cls, err):
        if err.value < 0:
            raise cls(err)

# Utils


def _nrm_get_function(method, argtypes=[], restype=Result):
    res = getattr(libnrm, method)
    res.restype = restype
    res.argtypes = argtypes
    return res


nrm_init = _nrm_get_function("nrm_init",
                             [ct.POINTER(nrm_int), ct.POINTER(nrm_str)])
nrm_finalize = _nrm_get_function("nrm_finalize", [])


def _init_and_register_exit():
    import atexit

    atexit.register(nrm_finalize)
    _res = nrm_init(None, None)
    Error.check(_res)


_init_and_register_exit()
del _init_and_register_exit
