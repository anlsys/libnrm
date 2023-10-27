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

from setuptools import setup, find_packages

setup(
    name="nrm",
    version="0.7.0",
    description="Argo Node Resource Manager Python Bindings",
    author="Swann Perarnau",
    author_email="swann@anl.gov",
    url="https://github.com/anlsys/libnrm",
    license="BSD3",
    classifiers=[
        "Development Status :: 3 - Alpha",
        "License :: OSI Approved :: BSD License",
        "Programming Language :: Python :: 3.10",
    ],
    packages=["nrm"],
    package_dir={"nrm": "nrm"},
)
