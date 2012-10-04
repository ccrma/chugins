
CHUGINS=ABSaturator Bitcrusher MagicSine

ifneq ($(CK_TARGET),)
.DEFAULT_GOAL:=$(CK_TARGET)
ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS:=$(.DEFAULT_GOAL)
endif
endif

osx: $(CHUGINS)
linux: $(CHUGINS)
win32: $(CHUGINS)
clean: $(CHUGINS)

$(CHUGINS):
	make -C $@ $(MAKECMDGOALS)

