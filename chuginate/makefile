
chugerate: chugerate.py bootstrap.py template/ChuGin.cpp template/makefile \
template/makefile.osx template/makefile.linux template/makefile.win32
	python bootstrap.py < chugerate.py > chugerate
	chmod u+x chugerate

clean:
	rm -rf chugerate

install: chugerate
	cp chugerate /usr/bin/
