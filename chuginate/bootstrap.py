#!/usr/bin/env python3

import re, sys, base64

marker = dict()
bin_marker = dict()
template = dict()

marker['cpp'] = r"%\(CPP_CODE\)%"
marker['test'] = r"%\(TEST_CK_CODE\)%"
marker['makefile'] = r"%\(MAKEFILE_CODE\)%"
marker['makefile.mac'] = r"%\(MAKEFILEOSX_CODE\)%"
marker['makefile.linux'] = r"%\(MAKEFILELINUX_CODE\)%"
marker['makefile.win'] = r"%\(MAKEFILEWIN32_CODE\)%"
marker['.vcxproj'] = r"%\(VCXPROJ_CODE\)%"

bin_marker['chuck'] = r"%\(CHUCK_B64\)%"

template['cpp'] = "template/ChuGin.cpp"
template['test'] = "template/ChuGin-test.ck"
template['makefile'] = "template/makefile"
template['makefile.mac'] = "template/makefile.mac"
template['makefile.linux'] = "template/makefile.linux"
template['makefile.win'] = "template/makefile.win"
template['.vcxproj'] = "template/ChuGin.vcxproj"
template['chuck'] = "template/chuck.tgz"

code = sys.stdin.read()

for key in marker:
    with open(template[key], 'r') as f:
        code = re.sub(marker[key], lambda match: f.read(), code)

for key in bin_marker:
    with open(template[key], 'rb') as f:
        b64 = base64.b64encode(f.read()).decode()
    code = re.sub(bin_marker[key], lambda match: b64, code)

sys.stdout.write(code)
