
CHUCK_INCLUDE_ROOT=../
CHUCK_HEADERS=$(wildcard ${CHUCK_INCLUDE_ROOT}/chuck/include/*.h)

TEMPLATE_FILES=template/ChuGin.cpp template/makefile template/makefile.mac \
    template/makefile.linux template/makefile.win template/ChuGin.vcxproj \
    template/ChuGin-test.ck template/chuck.tgz

chuginate: chuginate.py bootstrap.py ${TEMPLATE_FILES}
	python3 bootstrap.py < chuginate.py > chuginate
	chmod u+x chuginate

template/chuck.tgz: ${CHUCK_HEADERS}
	tar czf $@ -C ${CHUCK_INCLUDE_ROOT} chuck

clean:
	rm -rf chuginate template/chuck.tgz

install: chuginate
	cp chuginate /usr/local/bin/
