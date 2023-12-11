#!/usr/bin/env python3
"""
Generate a skeleton for a new ChuGin.
"""

import sys, re, os, io, tarfile, base64

if len(sys.argv) != 2 and len(sys.argv) != 3:
    print("usage: chuginate chugin_name [destination_directory]")
    sys.exit(-1)

chugin_name = sys.argv[1]
if len(sys.argv) >= 3:
    dest_dir = sys.argv[2]
else:
    dest_dir = sys.argv[1]
    os.mkdir(dest_dir)

chugin_lcname = chugin_name.lower()
chugin_ucname = chugin_name.upper()
chugin_initials = re.sub('[a-z]', '', chugin_name).lower()
if len(chugin_initials) == 0: chugin_initials = chugin_name[0]
chugin_header_path = 'chuck/include'

USE_EXISTING_CHUCK_HEADERS = False
# if the chuck headers already exist
if os.path.isdir("%s/../chuck/include" % (dest_dir)):
    chugin_header_path = "../chuck/include/"
    USE_EXISTING_CHUCK_HEADERS = True

def substitute(text):
    global chugin_name, chugin_lcname, chugin_ucname, chugin_initials, chugin_header_path
    text = re.sub('\%\(CHUGIN_NAME\)\%', chugin_name, text)
    text = re.sub('\%\(CHUGIN_LCNAME\)\%', chugin_lcname, text)
    text = re.sub('\%\(CHUGIN_UCNAME\)\%', chugin_ucname, text)
    text = re.sub('\%\(CHUGIN_INITIALS\)\%', chugin_initials, text)
    text = re.sub('\%\(CHUGIN_HEADER_PATH\)\%', chugin_header_path, text)
    return text

# print "name: %s lc: %s initials: %s" % (chugin_name, chugin_lcname, chugin_initials)

code = dict()
tgz = dict()
filepath = dict()
newlines = dict()

code['cpp'] = u'''%(CPP_CODE)%'''
code['test'] = u'''%(TEST_CK_CODE)%'''
code['makefile'] = u'''%(MAKEFILE_CODE)%'''
code['makefile.mac'] = u'''%(MAKEFILEOSX_CODE)%'''
code['makefile.linux'] = u'''%(MAKEFILELINUX_CODE)%'''
code['makefile.win'] = u'''%(MAKEFILEWIN32_CODE)%'''
code['.vcxproj'] = u'''%(VCXPROJ_CODE)%'''

if not USE_EXISTING_CHUCK_HEADERS:
    tgz['chuck'] = u'''%(CHUCK_B64)%'''

filepath['cpp'] = "%s/%s.cpp" % (dest_dir, chugin_name)
filepath['test'] = "%s/%s-test.ck" % (dest_dir, chugin_name)
filepath['makefile'] = "%s/makefile" % (dest_dir)
filepath['makefile.mac'] = "%s/makefile.mac" % (dest_dir)
filepath['makefile.linux'] = "%s/makefile.linux" % (dest_dir)
filepath['makefile.win'] = "%s/makefile.win" % (dest_dir)
filepath['.vcxproj'] = "%s/%s.vcxproj" % (dest_dir, chugin_name)
filepath['chuck'] = "%s/chuck.tgz" % (dest_dir)

newlines['.vcxproj'] = '\r\n'

code['cpp'] = substitute(code['cpp'])
code['test'] = substitute(code['test'])
code['makefile'] = substitute(code['makefile'])
code['makefile.win'] = substitute(code['makefile.win'])
code['.vcxproj'] = substitute(code['.vcxproj'])


for key in code:
    if key in newlines:
        nl = newlines[key]
    else:
        nl = '\n'
    with io.open(filepath[key], "wt", newline=nl) as f:
        f.write(code[key])

for key in tgz:
    with io.open(filepath[key], "wb") as f:
        f.write(base64.b64decode(tgz[key]))
    with tarfile.open(filepath[key]) as tar:
        tar.extractall(os.path.dirname(filepath[key]))
    os.unlink(filepath[key])


