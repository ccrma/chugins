
chuginate: chuginate.py bootstrap.py template/ChuGin.cpp template/makefile \
template/makefile.osx template/makefile.linux template/makefile.win32 \
template/ChuGin.dsw template/ChuGin.dsp template/ChuGin.vcxproj \
template/chuck.tgz
	python bootstrap.py < chuginate.py > chuginate
	chmod u+x chuginate

template/chuck.tgz: $(wildcard template/chuck/*.h)
	tar czf $@ -C template chuck

clean:
	rm -rf chuginate template/chuck.tgz

install: chuginate
	cp chuginate /usr/bin/

