#!/usr/bin/python

import sys
import re

if len(sys.argv) != 3:
    print "usage: chugerate [chugin name] [destination directory]"
    sys.exit(-1)

chugin_name = sys.argv[1]
dest_dir = sys.argv[2]

chugin_lcname = chugin_name.lower()
chugin_initials = re.sub('[a-z]', '', chugin_name).lower()

# print "name: %s lc: %s initials: %s" % (chugin_name, chugin_lcname, chugin_initials)

code = dict();
filepath = dict();

code['cpp'] = '''%(CPP_CODE)%'''
code['makefile'] = '''%(MAKEFILE_CODE)%'''
code['makefile.osx'] = '''%(MAKEFILEOSX_CODE)%'''
code['makefile.linux'] = '''%(MAKEFILELINUX_CODE)%'''
code['makefile.win32'] = '''%(MAKEFILEWIN32_CODE)%'''

filepath['cpp'] = "%s/%s.cpp" % (dest_dir, chugin_name)
filepath['makefile'] = "%s/makefile" % (dest_dir)
filepath['makefile.osx'] = "%s/makefile.osx" % (dest_dir)
filepath['makefile.linux'] = "%s/makefile.linux" % (dest_dir)
filepath['makefile.win32'] = "%s/makefile.win32" % (dest_dir)

code['cpp'] = re.sub('\%\(CHUGIN_NAME\)\%', chugin_name, code['cpp'])
code['cpp'] = re.sub('\%\(CHUGIN_LCNAME\)\%', chugin_lcname, code['cpp'])
code['cpp'] = re.sub('\%\(CHUGIN_INITIALS\)\%', chugin_lcname, code['cpp'])

code['makefile'] = re.sub('\%\(CHUGIN_NAME\)\%', chugin_name, code['makefile'])

for key in code:
    f = open(filepath[key], "w")
    f.write(code[key])
    f.close()
