
# chugin name
CHUGIN_NAME=Faust

# all of the c/cpp files that compose this chugin
C_MODULES=
CXX_MODULES=Faust.cpp

# where to find chugin.h
CK_SRC_PATH?=../chuck/include


# ---------------------------------------------------------------------------- #
# you won't generally need to change anything below this line for a new chugin #
# ---------------------------------------------------------------------------- #

# default target: print usage message and quit
current: 
	@echo "[chugin build]: please use one of the following configurations:"
	@echo "   make linux, make mac, make web, or make win32 (for Windows compilation, we recommend using the Visual Studio project)"
	@echo "Options:" 
	@echo "   MAX_INPUTS: the maximum number of inputs of the chugin"
	@echo "   MAX_OUTPUTS: the maximum number of outputs of the chugin"
	@echo "Example:"
	@echo "   make linux MAX_INPUTS=1 MAX_OUTPUTS=4"

help: current

ifneq ($(CK_TARGET),)
.DEFAULT_GOAL:=$(CK_TARGET)
ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS:=$(.DEFAULT_GOAL)
endif
endif

.PHONY: mac osx linux linux-oss linux-jack linux-alsa win32
mac osx linux linux-oss linux-jack linux-alsa: all

win32:
	make -f makefile.win

CC=gcc
CXX=gcc
LD=g++

CHUGIN_PATH=/usr/local/lib/chuck

ifneq (,$(strip $(filter mac osx bin-dist-osx,$(MAKECMDGOALS))))
include makefile.mac
endif

ifneq (,$(strip $(filter linux,$(MAKECMDGOALS))))
include makefile.linux
endif

ifneq (,$(strip $(filter linux-oss,$(MAKECMDGOALS))))
include makefile.linux
endif

ifneq (,$(strip $(filter linux-jack,$(MAKECMDGOALS))))
include makefile.linux
endif

ifneq (,$(strip $(filter linux-alsa,$(MAKECMDGOALS))))
include makefile.linux
endif

ifneq ($(CHUCK_DEBUG),)
FLAGS+= -g
else
FLAGS+= -O3
endif

ifneq ($(CHUCK_STRICT),)
FLAGS+= -Werror
endif

ifneq ($(MAX_INPUTS),)
INS=-DMAX_INPUTS=$(MAX_INPUTS)
endif

ifneq ($(MAX_OUTPUTS),)
OUTS=-DMAX_OUTPUTS=$(MAX_OUTPUTS)
endif

# default: build a dynamic chugin
CK_CHUGIN_STATIC?=0

ifeq ($(CK_CHUGIN_STATIC),0)
SUFFIX=.chug
else
SUFFIX=.schug
FLAGS+= -D__CK_DLL_STATIC__
endif
# webchugin extension
WEBSUFFIX=.wasm

C_OBJECTS=$(addsuffix .o,$(basename $(C_MODULES)))
CXX_OBJECTS=$(addsuffix .o,$(basename $(CXX_MODULES)))

CHUG=$(addsuffix $(SUFFIX),$(CHUGIN_NAME))
WEBCHUG=$(addsuffix $(WEBSUFFIX),$(CHUG))

all: $(CHUG)

$(CHUG): $(C_OBJECTS) $(CXX_OBJECTS)
ifeq ($(CK_CHUGIN_STATIC),0)
	$(LD) -o $@ $^ $(LDFLAGS)
else
	ar rv $@ $^
	ranlib $@
endif

$(C_OBJECTS): %.o: %.c
	$(CC) -c -o $@ $< $(FLAGS) 

$(CXX_OBJECTS): %.o: %.cpp $(CK_SRC_PATH)/chugin.h
	$(CXX) $(INS) $(OUTS) -c -o $@ $< $(FLAGS) 

install: $(CHUG)
	mkdir -p $(CHUGIN_PATH)
	cp $^ $(CHUGIN_PATH)
	chmod 755 $(CHUGIN_PATH)/$(CHUG)

clean: 
	rm -rf $(C_OBJECTS) $(CXX_OBJECTS) $(CHUG) $(WEBCHUG) Release Debug

