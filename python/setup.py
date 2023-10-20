"""Argo Node Resource Manager
"""

from setuptools import setup, find_packages

setup(
    name="nrm",
    version="0.7.0",
    description="Argo Node Resource Manager",
    author="Swann Perarnau",
    author_email="swann@anl.gov",
    url="https://github.com/anlsys/libnrm",
    license="BSD3",
    classifiers=[
        "Development Status :: 3 - Alpha",
        "License :: OSI Approved :: BSD License",
        "Programming Language :: Python :: 3.10",
    ],
    packages=find_packages(),
    install_requires=["loguru"],
)
