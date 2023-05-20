
CHUGINS=ABSaturator AmbPan Bitcrusher KasFilter MagicSine FIR FoldbackSaturator \
	PanN PitchTrack GVerb Mesh2D Spectacle Elliptic Sigmund ExpDelay Overdrive \
	Multicomb PowerADSR WinFuncEnv WPDiodeLadder WPKorg35 \
	Binaural ExpEnv Perlin Random RegEx Wavetable

CHUGS_NOT_ON_WIN32=FluidSynth
CHUGINS_WIN32=$(filter-out $(CHUGS_NOT_ON_WIN32),$(CHUGINS))

CHUGS=$(foreach CHUG,$(CHUGINS),$(CHUG)/$(CHUG).chug)
CHUGS_WIN32=$(foreach CHUG,$(CHUGINS_WIN32),$(CHUG)/$(CHUG).chug)
CHUGS_RELEASE=$(foreach CHUG,$(CHUGINS_WIN32),$(CHUG)/Release/$(CHUG).chug)
CHUGS_CLEAN=$(addsuffix .clean,$(CHUGINS))


DESTDIR?=/usr/local
INSTALL_DIR=$(DESTDIR)/lib/chuck
INSTALL_DIR_WIN32="C:/Program Files/ChucK/chugins"

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
win32: $(CHUGS_WIN32)

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

install-win32: $(CHUGS_RELEASE)
	mkdir -p $(INSTALL_DIR_WIN32)
	cp -rf $(CHUGS_RELEASE) $(INSTALL_DIR_WIN32)

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
	cp -rf $(EXAMPLES) chugins-src-$(DATE)/examples/
	git archive HEAD . | tar -x -C chugins-src-$(DATE)/src
	tar czf chugins-src-$(DATE).tgz chugins-src-$(DATE)


PPA_CHUG_VERSION?=1.3.5.0a
PPA_DEB_VERSION?=1.3.5.0a-ppa1

PPA_CHUG_DIR=chugins_$(PPA_CHUG_VERSION)
PPA_CHUG_TGZ=../chugins_$(PPA_CHUG_VERSION).orig.tar.gz

FORCE:

ppa-tgz: $(PPA_CHUG_TGZ)

ppa-source: $(PPA_CHUG_TGZ) ppa-clean
	debuild -S

$(PPA_CHUG_TGZ): FORCE
	rm -rf $(PPA_CHUG_DIR)
	mkdir -p $(PPA_CHUG_DIR)
	git archive HEAD . | tar -x -C $(PPA_CHUG_DIR)
	find $(PPA_CHUG_DIR)/ -type f -exec chmod a-x {} +
	tar czf $(PPA_CHUG_TGZ) $(PPA_CHUG_DIR)
	rm -rf $(PPA_CHUG_DIR)

ppa-binary: $(PPA_CHUG_TGZ) ppa-clean
	debuild -uc -us

ppa-upload:
	dput ppa:t-spencer/chuck ../chugins_$(PPA_DEB_VERSION)_source.changes

ppa-clean:
	debian/rules clean
	rm -rf debian/chuck
