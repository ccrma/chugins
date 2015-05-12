#!/usr/bin/python

import sys, re, os, io, tarfile, base64

if len(sys.argv) != 2 and len(sys.argv) != 3:
    print "usage: chuginate chugin_name [destination_directory]"
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
if len(chugin_initials) == 0: chugin_initials = chugin_name[0];
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
code['makefile'] = u'''%(MAKEFILE_CODE)%'''
code['makefile.osx'] = u'''%(MAKEFILEOSX_CODE)%'''
code['makefile.linux'] = u'''%(MAKEFILELINUX_CODE)%'''
code['makefile.win32'] = u'''%(MAKEFILEWIN32_CODE)%'''
code['.dsw'] = u'''%(DSW_CODE)%'''
code['.dsp'] = u'''%(DSP_CODE)%'''
code['.vcxproj'] = u'''%(VCXPROJ_CODE)%'''

if not USE_EXISTING_CHUCK_HEADERS:
    tgz['chuck'] = u'''%(CHUCK_B64)%'''

filepath['cpp'] = "%s/%s.cpp" % (dest_dir, chugin_name)
filepath['makefile'] = "%s/makefile" % (dest_dir)
filepath['makefile.osx'] = "%s/makefile.osx" % (dest_dir)
filepath['makefile.linux'] = "%s/makefile.linux" % (dest_dir)
filepath['makefile.win32'] = "%s/makefile.win32" % (dest_dir)
filepath['.dsw'] = "%s/%s.dsw" % (dest_dir, chugin_name)
filepath['.dsp'] = "%s/%s.dsp" % (dest_dir, chugin_name)
filepath['.vcxproj'] = "%s/%s.vcxproj" % (dest_dir, chugin_name)
filepath['chuck'] = "%s/chuck.tgz" % (dest_dir)

newlines['.dsw'] = '\r\n'
newlines['.dsp'] = '\r\n'
newlines['.vcxproj'] = '\r\n'

code['cpp'] = substitute(code['cpp'])
code['makefile'] = substitute(code['makefile'])
code['makefile.win32'] = substitute(code['makefile.win32'])
code['.dsw'] = substitute(code['.dsw'])
code['.dsp'] = substitute(code['.dsp'])
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


