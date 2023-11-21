# Copyright 2019 UChicago Argonne, LLC.
# (c.f. AUTHORS, LICENSE)
#
# This file is part of the libnrm project.
# For more info, see https://github.com/anlsys/libnrm
#
# SPDX-License-Identifier: BSD-3-Clause

import nrm
import unittest


class TestInit(unittest.TestCase):
    def test_init(self):
        pass

    def test_vars(self):
        assert nrm.upstream_uri is not None
        assert nrm.upstream_rpc_port is not None
        assert nrm.upstream_pub_port is not None
        assert nrm.ratelimit is not None
        assert nrm.transmit is not None
        assert nrm.timeout is not None


if __name__ == "__main__":
    unittest.main()
