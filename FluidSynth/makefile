
# chugin name
CHUGIN_NAME=FluidSynth

# all of the c/cpp files that compose this chugin
C_MODULES=
CXX_MODULES=FluidSynth.cpp
LIBS=fluidsynth
# where to find chugin.h
CK_SRC_PATH?=../chuck/include/


# ---------------------------------------------------------------------------- #
# you won't generally need to change anything below this line for a new chugin #
# ---------------------------------------------------------------------------- #

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
	$(LD) $(LDFLAGS) -o $@ $^ $(addprefix -l,$(LIBS))
else
	ar rv $@ $^
	ranlib $@
endif

$(C_OBJECTS): %.o: %.c
	$(CC) $(FLAGS) -c -o $@ $<

$(CXX_OBJECTS): %.o: %.cpp $(CK_SRC_PATH)/chugin.h
	$(CXX) $(FLAGS) -c -o $@ $<

# build as webchugin
web:
	emcc -O3 -s SIDE_MODULE=1 -s DISABLE_EXCEPTION_CATCHING=0 -fPIC -Wformat=0 \
	-I ../chuck/include/ $(CXX_MODULES) $(C_MODULES) -o $(WEBCHUG)

install: $(CHUG)
	mkdir -p $(CHUGIN_PATH)
	cp $^ $(CHUGIN_PATH)
	chmod 755 $(CHUGIN_PATH)/$(CHUG)

clean: 
	rm -rf $(C_OBJECTS) $(CXX_OBJECTS) $(CHUG) $(WEBCHUG) Release Debug builddir-x86_64 builddir-arm64 builddir-win builddir-linux


#-------------------------------------------------------------------
# meson-build system
# for cross-platform building including for Chump package management
#-------------------------------------------------------------------

#-----------  setting up the build -----------
setup-mac-x86_64:
	meson setup --buildtype=release --wrap-mode=forcefallback --cross-file cross/x86_64-macos.txt builddir-x86_64

setup-mac-arm64:
	meson setup --buildtype=release --cross-file cross/arm64-macos.txt builddir-arm64

setup-win:
	meson setup --buildtype=release builddir-win --backend vs

setup-linux:
	meson setup --buildtype=release builddir-linux


#-----------  building -----------
.PHONY: build-mac
build-mac: build-mac-x86_64 build-mac-arm64

build-mac-x86_64: setup-mac-x86_64
	meson compile -C builddir-x86_64
	cp builddir-x86_64/FluidSynth.chug FluidSynth-x86_64.chug

build-mac-arm64: setup-mac-arm64
	meson compile -C builddir-arm64
	cp builddir-arm64/FluidSynth.chug FluidSynth-arm64.chug

build-win: setup-win
	meson compile -C builddir-win

build-linux: setup-linux
	meson compile -C builddir-linux

# note: presently, Nick and Ge are able to build on windows using a combination of
# cygwin (which make and can install meson, and can setup BUT not build) with
# x64 Native Tools Command Prompt for VS 2022 (no 'make' at least on Ge's machine)
# but can run the meson compile command directly
