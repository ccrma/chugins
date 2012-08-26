#!/usr/bin/python

import sys
import re
import os

if len(sys.argv) != 2 and len(sys.argv) != 3:
    print "usage: chugerate chugin_name [destination_directory]"
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
if len(chugin_initials) == 0:
    chugin_initials = chugin_name[0];

def substitute(text):
    global chugin_name, chugin_lcname, chugin_ucname, chugin_initials
    text = re.sub('\%\(CHUGIN_NAME\)\%', chugin_name, text)
    text = re.sub('\%\(CHUGIN_LCNAME\)\%', chugin_lcname, text)
    text = re.sub('\%\(CHUGIN_UCNAME\)\%', chugin_ucname, text)
    text = re.sub('\%\(CHUGIN_INITIALS\)\%', chugin_initials, text)
    return text

# print "name: %s lc: %s initials: %s" % (chugin_name, chugin_lcname, chugin_initials)

code = dict();
filepath = dict();

code['cpp'] = '''%(CPP_CODE)%'''
code['makefile'] = '''%(MAKEFILE_CODE)%'''
code['makefile.osx'] = '''%(MAKEFILEOSX_CODE)%'''
code['makefile.linux'] = '''%(MAKEFILELINUX_CODE)%'''
code['makefile.win32'] = '''%(MAKEFILEWIN32_CODE)%'''
code['.dsw'] = '''%(DSW_CODE)%'''
code['.dsp'] = '''%(DSP_CODE)%'''

filepath['cpp'] = "%s/%s.cpp" % (dest_dir, chugin_name)
filepath['makefile'] = "%s/makefile" % (dest_dir)
filepath['makefile.osx'] = "%s/makefile.osx" % (dest_dir)
filepath['makefile.linux'] = "%s/makefile.linux" % (dest_dir)
filepath['makefile.win32'] = "%s/makefile.win32" % (dest_dir)
filepath['.dsw'] = "%s/%s.dsw" % (dest_dir, chugin_name)
filepath['.dsp'] = "%s/%s.dsp" % (dest_dir, chugin_name)

code['cpp'] = substitute(code['cpp'])
code['makefile'] = substitute(code['makefile'])
code['.dsw'] = substitute(code['.dsw'])
code['.dsp'] = substitute(code['.dsp'])

for key in code:
    f = open(filepath[key], "w")
    f.write(code[key])
    f.close()


    
    
