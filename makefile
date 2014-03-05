
CHUGINS=ABSaturator AmbPan Bitcrusher KasFilter MagicSine FIR PanN PitchTrack GVerb Mesh2D Spectacle FluidSynth Elliptic Sigmund ExpDelay Overdrive Multicomb
CHUGS=$(foreach CHUG,$(CHUGINS),$(CHUG)/$(CHUG).chug)
CHUGS_CLEAN=$(addsuffix .clean,$(CHUGINS))

# CHUGS_NOT_ON_WIN32=FluidSynth Elliptic Sigmund ExpDelay Overdrive Multicomb
# CHUGS_WIN32=$(addsuffix .win32,$(filter-out $(CHUGS_NOT_ON_WIN32),$(CHUGINS)))

INSTALL_DIR=/usr/lib/chuck

ifneq ($(CK_TARGET),)
.DEFAULT_GOAL:=$(CK_TARGET)
ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS:=$(.DEFAULT_GOAL)
endif
endif

CHUCK_STRICT=1

osx: $(CHUGS)
linux: $(CHUGS)
linux-alsa: $(CHUGS)
linux-jack: $(CHUGS)

win32: 
	msbuild.exe /p:Configuration=Release chugins.sln

$(CHUGS): 
	CHUCK_STRICT=1 make -C $(dir $@) $(MAKECMDGOALS)

clean: $(CHUGS_CLEAN)
.PHONY: $(CHUGS_CLEAN)
$(CHUGS_CLEAN):
	make -C $(basename $@) clean

#.PHONY: $(CHUGS_WIN32)
#$(CHUGS_WIN32): 
#	cd $(basename $@); msbuild.exe /p:Configuration=Release 

install: $(CHUGS)
	mkdir -p $(INSTALL_DIR)
	cp -rf $(CHUGS) $(INSTALL_DIR)

DATE=$(shell date +"%Y-%m-%d")

FIR_EXAMPLES=CCRMAHallShort.wav FIRConvolve.ck FIRConvolveHomer.ck \
FIRFOFTest.ck FIRGaussImpulseTests.ck FIRGaussTests.ck FIRSincBPSweepTest.ck \
FIRSincExplicit.ck FIRSincImpulseTests.ck FIRSincTests.ck \
GreyholeDownUpDecimateDemo.ck

EXAMPLES=Bitcrusher/Bitcrusher-test.ck MagicSine/MagicSine-test.ck \
ABSaturator/ABSaturator-test.ck KasFilter/README-KasFilter.ck \
$(addprefix FIR/examples/,$(FIR_EXAMPLES)) \
PanN/Pan4-test.ck PanN/Pan8-test.ck

bin-dist-osx: osx
	mkdir -p chugins-mac-$(DATE)/chugins/
	mkdir -p chugins-mac-$(DATE)/examples/
	cp -f notes/README-mac.txt chugins-mac-$(DATE)/
	cp -rf $(EXAMPLES) chugins-mac-$(DATE)/examples/
	cp -rf $(CHUGS) chugins-mac-$(DATE)/chugins/
	tar czf chugins-mac-$(DATE).tgz chugins-mac-$(DATE)
	rm -rf chugins-mac-$(DATE)/

WIN_CHUGS=$(foreach CHUG,$(CHUGINS),$(CHUG)/Release/$(CHUG).chug)
    
bin-dist-win32:
	-echo '*** Please ensure you have built all chugins from within Visual Studio ***'
	mkdir -p chugins-windows-$(DATE)/chugins/
	mkdir -p chugins-windows-$(DATE)/examples/
	cp -f notes/README-windows.txt chugins-windows-$(DATE)/
	cp -rf $(EXAMPLES) chugins-windows-$(DATE)/examples/
	cp -rf $(WIN_CHUGS) chugins-windows-$(DATE)/chugins/
	rm -rf chugins-windows-$(DATE).zip
	zip -q -9 -r -m chugins-windows-$(DATE).zip chugins-windows-$(DATE)
	rm -rf chugins-windows-$(DATE)/

src-dist:
	mkdir -p chugins-src-$(DATE)/src/
	mkdir -p chugins-src-$(DATE)/examples/
	cp -rf $(EXAMPLES) chugins-windows-$(DATE)/examples/
	git checkout-index -a -f --prefix=
	tar czf chugins-mac-$(DATE).tgz chugins-mac-$(DATE)
