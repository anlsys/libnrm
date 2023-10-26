from ctypes import *
from ctypes.util import find_library
from loguru import logger

NRMH_PATH = find_library("nrm")

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

libnrm = CDLL(NRMH_PATH)

nrm_init = _nrm_get_function("nrm_init", [POINTER(c_int), POINTER(c_char_p)], c_int)
nrm_finalize = _nrm_get_function("nrm_finalize", [], c_int)
nrm_client_create = _nrm_get_function("nrm_client_create", [POINTER(nrm_client), c_char_p, c_int, c_int], c_int)
nrm_client_destroy = _nrm_get_function("nrm_client_destroy", [POINTER(nrm_client)], None)


##### setup #####

assert not nrm_init(
    None, None
), "NRM library did not initialize successfully"
logger.debug("NRM initialized")

##### teardown #####

assert not nrm_finalize(), "Unable to exit NRM"
