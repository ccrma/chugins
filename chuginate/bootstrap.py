#!/usr/bin/python

import re
import sys

marker = dict()
template = dict()

marker['cpp'] = "\%\(CPP_CODE\)\%"
marker['makefile'] = "\%\(MAKEFILE_CODE\)\%"
marker['makefile.osx'] = "\%\(MAKEFILEOSX_CODE\)\%"
marker['makefile.linux'] = "\%\(MAKEFILELINUX_CODE\)\%"
marker['makefile.win32'] = "\%\(MAKEFILEWIN32_CODE\)\%"
marker['.dsw'] = "\%\(DSW_CODE\)\%"
marker['.dsp'] = "\%\(DSP_CODE\)\%"

template['cpp'] = "template/ChuGin.cpp"
template['makefile'] = "template/makefile"
template['makefile.osx'] = "template/makefile.osx"
template['makefile.linux'] = "template/makefile.linux"
template['makefile.win32'] = "template/makefile.win32"
template['.dsw'] = "template/ChuGin.dsw"
template['.dsp'] = "template/ChuGin.dsp"

code = sys.stdin.read()

for key in marker:
    f = open(template[key], 'r')
    code = re.sub(marker[key], f.read(), code)

sys.stdout.write(code)
