# ---------------------------------------------------------------------------- #
# you won't generally need to change anything below this line for a new chugin #
# ---------------------------------------------------------------------------- #

# Prevent make from trying to rebuild the makefile from makefile.web using implicit rules
makefile: ;

# default target: print usage message and quit
current: 
	@echo "[chugin build]: please use one of the following configurations:"
	@echo "   make linux, make mac, make web, or make win32"

ifneq ($(CK_TARGET),)
.DEFAULT_GOAL:=$(CK_TARGET)
ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS:=$(.DEFAULT_GOAL)
endif
endif

.PHONY: web win32 mac linux clean clean-web

web:
	make -f makefile.web

win32:
	make -f makefile.win

mac:
	make -f makefile.mac

linux:
	make -f makefile.linux

clean-web:
	make -f makefile.web clean

clean:
	rm -rf build package
	rm -rf thirdparty/libsndfile/build thirdparty/libsndfile/install
	rm -f *.chug
	make -f makefile.web clean
