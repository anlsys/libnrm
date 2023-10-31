# Copyright 2019 UChicago Argonne, LLC.
# (c.f. AUTHORS, LICENSE)
#
# This file is part of the libnrm project.
# For more info, see https://github.com/anlsys/libnrm
#
# SPDX-License-Identifier: BSD-3-Clause

from nrm import Client
import unittest


class TestClient(unittest.TestCase):

    def test_client_init(self):
        client = Client()
        del client


if __name__ == "__main__":
    unittest.main()
