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

import os
import atexit
from ctypes import *
import logging as logger

if os.environ.get('LIBNRM_SO_'):
  libnrm = cdll.LoadLibrary(os.environ.get('LIBNRM_SO_'))
else:
  libnrm = cdll.LoadLibrary('libnrm.so.0.0.0')

########## types ##########

nrm_string = c_char_p
nrm_uuid = c_char_p
nrm_scope = c_void_p
nrm_client = c_void_p


class nrm_time(Structure):
    _fields_ = [("tv_nsec", c_long), ("tv_sec", c_int)]


nrm_client_event_listener_fn = CFUNCTYPE(
    c_int, nrm_string, nrm_time, nrm_scope, c_double
)

nrm_client_actuate_listener_fn = CFUNCTYPE(c_int, nrm_uuid, c_double)

##### utils #####

def _nrm_get_function(method, argtypes=[], restype=c_int):
    res = getattr(libnrm, method)
    res.restype = restype
    res.argtypes = argtypes
    return res

nrm_init = _nrm_get_function("nrm_init", [POINTER(c_int), POINTER(c_char_p)], c_int)
nrm_finalize = _nrm_get_function("nrm_finalize", [], c_int)
nrm_client_create = _nrm_get_function("nrm_client_create", [POINTER(nrm_client), c_char_p, c_int, c_int], c_int)
nrm_client_destroy = _nrm_get_function("nrm_client_destroy", [POINTER(nrm_client)], None)

##### errors #####

NRM_SUCCESS = 0
NRM_FAILURE = 1
NRM_ENOMEM = 2
NRM_EINVAL = 3
NRM_EDOM = 4
NRM_EBUSY = 6
NRM_EPERM = 7
NRM_ERROR_MAX = 8

##### setup #####

assert not nrm_init(
    None, None
), "NRM library did not initialize successfully"
logger.debug("NRM initialized")

##### teardown #####

atexit.register(nrm_finalize)