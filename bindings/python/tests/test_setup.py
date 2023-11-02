# Copyright 2019 UChicago Argonne, LLC.
# (c.f. AUTHORS, LICENSE)
#
# This file is part of the libnrm project.
# For more info, see https://github.com/anlsys/libnrm
#
# SPDX-License-Identifier: BSD-3-Clause

from nrm import Setup
import unittest
import os

options = {'prefix': os.environ.get('ABS_TOP_BUILDDIR')}


class TestSetup(unittest.TestCase):
    def test_setup_init(self):
        with Setup("nrmd", options=options):
            pass


if __name__ == "__main__":
    unittest.main()
