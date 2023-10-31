# Copyright 2019 UChicago Argonne, LLC.
# (c.f. AUTHORS, LICENSE)
#
# This file is part of the libnrm project.
# For more info, see https://github.com/anlsys/libnrm
#
# SPDX-License-Identifier: BSD-3-Clause

import os
import platform
import ctypes as ct

if os.environ.get('LIBNRM_SO_'):
    libnrm = ct.cdll.LoadLibrary(os.environ.get('LIBNRM_SO_'))
else:
    if platform.uname()[0] == 'Linux':
        libnrm = ct.cdll.LoadLibrary('libnrm.so')
    else:
        libnrm = ct.cdll.LoadLibrary('libnrm.dylib')

from .base import Error, Result
from .client import Client
