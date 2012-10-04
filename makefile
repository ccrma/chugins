
CHUGINS=ABSaturator Bitcrusher MagicSine
CHUGS=$(foreach CHUG,$(CHUGINS),$(CHUG)/$(CHUG).chug)

INSTALL_DIR=/usr/lib/chuck

ifneq ($(CK_TARGET),)
.DEFAULT_GOAL:=$(CK_TARGET)
ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS:=$(.DEFAULT_GOAL)
endif
endif

osx: $(CHUGS)
linux: $(CHUGS)
win32: $(CHUGS)

$(CHUGS): 
	make -C $(dir $@) $(MAKECMDGOALS)

clean:
	rm -rf $(addsuffix /*.o,$(CHUGINS)) $(CHUGS)

install: $(CHUGS)
	mkdir -p $(INSTALL_DIR)
	cp -rf $(CHUGS) $(INSTALL_DIR)