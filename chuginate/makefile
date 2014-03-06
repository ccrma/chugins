
chuginate: chuginate.py bootstrap.py template/ChuGin.cpp template/makefile \
template/makefile.osx template/makefile.linux template/makefile.win32 \
template/ChuGin.dsw template/ChuGin.dsp template/ChuGin.vcxproj
	python bootstrap.py < chuginate.py > chuginate
	chmod u+x chuginate

clean:
	rm -rf chuginate

install: chuginate
	cp chuginate /usr/bin/
