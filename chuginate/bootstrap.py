#!/usr/bin/python

import re, sys, base64

marker = dict()
bin_marker = dict()
template = dict()

marker['cpp'] = "\%\(CPP_CODE\)\%"
marker['makefile'] = "\%\(MAKEFILE_CODE\)\%"
marker['makefile.osx'] = "\%\(MAKEFILEOSX_CODE\)\%"
marker['makefile.linux'] = "\%\(MAKEFILELINUX_CODE\)\%"
marker['makefile.win32'] = "\%\(MAKEFILEWIN32_CODE\)\%"
marker['.dsw'] = "\%\(DSW_CODE\)\%"
marker['.dsp'] = "\%\(DSP_CODE\)\%"
marker['.vcxproj'] = "\%\(VCXPROJ_CODE\)\%"

bin_marker['chuck'] = "\%\(CHUCK_B64\)\%"

template['cpp'] = "template/ChuGin.cpp"
template['makefile'] = "template/makefile"
template['makefile.osx'] = "template/makefile.osx"
template['makefile.linux'] = "template/makefile.linux"
template['makefile.win32'] = "template/makefile.win32"
template['.dsw'] = "template/ChuGin.dsw"
template['.dsp'] = "template/ChuGin.dsp"
template['.vcxproj'] = "template/ChuGin.vcxproj"
template['chuck'] = "template/chuck.tgz"

code = sys.stdin.read()

for key in marker:
    f = open(template[key], 'r')
    code = re.sub(marker[key], f.read(), code)

for key in bin_marker:
    f = open(template[key], 'r')
    b64 = base64.b64encode(f.read())
    code = re.sub(bin_marker[key], b64, code)

sys.stdout.write(code)
