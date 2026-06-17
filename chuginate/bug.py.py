#!/usr/bin/python
"""
Generate a skeleton for a new ChuGin.
"""

import sys, re, os, io, tarfile, base64

with tarfile.open("template/chuck.tgz") as tar:
    tar.extractall("/tmp/yyy")


