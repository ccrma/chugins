/* ------------------------------------------------------------
author: "RM-CC"
name: "FreeverbStereo"
version: "0.0"
Code generated with Faust 2.74.5. (https://faust.grame.fr)
Compilation options: -a .faust2ck_tmp/FreeverbStereoF.dsp-wrapper.cpp -lang cpp -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __mydsp_H__
#define  __mydsp_H__

#include "chugin.h"
// #include "chuck_dl.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <map>
#include <string>
#include <cmath>
#include <algorithm>

//-------------------------------------------------------------------
// Generic min and max using C++ inline
//-------------------------------------------------------------------

inline int      lsr (int x, int n)          { return int(((unsigned int)x) >> n); }
inline int      int2pow2 (int x)            { int r=0; while ((1<<r)<x) r++; return r; }


/******************************************************************************
 *******************************************************************************
 
 FAUST META DATA
 
 *******************************************************************************
 *******************************************************************************/

struct Meta : std::map<std::string, std::string>
{
    void declare(const char* key, const char* value)
    {
        (*this)[key] = value;
    }
};

/* UI class - do-nothing (from FAUST/minimal.cpp) */

#ifdef WIN32
#ifdef interface
#undef interface
#endif // interface
#endif // WIN32

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

class Soundfile;

class UI
{
    public:
        virtual ~UI() {}
        
        // active widgets
        virtual void addButton(const char* label, FAUSTFLOAT* zone) = 0;
        virtual void addCheckButton(const char* label, FAUSTFLOAT* zone) = 0;
        virtual void addVerticalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) = 0;
        virtual void addHorizontalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) = 0;
        virtual void addNumEntry(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) = 0;
        
        // passive widgets
        virtual void addHorizontalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max) = 0;
        virtual void addVerticalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max) = 0;
        
        // layout widgets
        virtual void openTabBox(const char* label) = 0;
        virtual void openHorizontalBox(const char* label) = 0;
        virtual void openVerticalBox(const char* label) = 0;
        virtual void closeBox() = 0;
        
        // soundfiles
        
        virtual void addSoundfile(const char* label, const char* filename, Soundfile** sf_zone) = 0;
        
        virtual void declare(FAUSTFLOAT* zone, const char* key, const char* value) {}
};

class dsp
{
    public:
        virtual ~dsp() {}

        virtual int getNumInputs() = 0;
        virtual int getNumOutputs() = 0;
        virtual void buildUserInterface(UI* interface) = 0;
        virtual int getSampleRate() = 0;
        virtual void init(int samplingRate) = 0;
        virtual void instanceInit(int sample_rate) = 0;
        virtual void instanceConstants(int sample_rate) = 0;
        virtual void instanceResetUserInterface() = 0;
        virtual void instanceClear() = 0;
        virtual dsp* clone() = 0;
        virtual void metadata(Meta* m) = 0;
        virtual void compute(int len, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) = 0;

        SAMPLE ** ck_frame_in;
        SAMPLE ** ck_frame_out;
};

/*
 * FAUST intrinsic
 */

/*
 * FAUST defines UI values as private, but provides no getters/setters.
 * In our particular case it's way more convenient to access them directly
 * than to set up a complicated UI structure.  Also get rid of everything
 * being "virtual", since it may stop the compiler from inlining properly!
 */
#define private public
#define virtual

/* Rename the class the name of our DSP. */
#define mydsp FreeverbStereo

/*
 * FAUST class
 */
#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS mydsp
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

#if defined(_WIN32)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif


class mydsp : public dsp {
	
 private:
	
	FAUSTFLOAT fVslider0;
	FAUSTFLOAT fVslider1;
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fRec9[2];
	FAUSTFLOAT fVslider2;
	float fConst2;
	int IOTA0;
	float fVec0[8192];
	int iConst3;
	float fRec8[2];
	float fRec11[2];
	float fVec1[8192];
	int iConst4;
	float fRec10[2];
	float fRec13[2];
	float fVec2[8192];
	int iConst5;
	float fRec12[2];
	float fRec15[2];
	float fVec3[8192];
	int iConst6;
	float fRec14[2];
	float fRec17[2];
	float fVec4[8192];
	int iConst7;
	float fRec16[2];
	float fRec19[2];
	float fVec5[8192];
	int iConst8;
	float fRec18[2];
	float fRec21[2];
	float fVec6[8192];
	int iConst9;
	float fRec20[2];
	float fRec23[2];
	float fVec7[8192];
	int iConst10;
	float fRec22[2];
	float fVec8[2048];
	int iConst11;
	int iConst12;
	float fRec6[2];
	float fVec9[2048];
	int iConst13;
	int iConst14;
	float fRec4[2];
	float fVec10[2048];
	int iConst15;
	int iConst16;
	float fRec2[2];
	float fVec11[1024];
	int iConst17;
	int iConst18;
	float fRec0[2];
	float fRec33[2];
	float fVec12[8192];
	FAUSTFLOAT fVslider3;
	float fConst19;
	float fRec32[2];
	float fRec35[2];
	float fVec13[8192];
	float fRec34[2];
	float fRec37[2];
	float fVec14[8192];
	float fRec36[2];
	float fRec39[2];
	float fVec15[8192];
	float fRec38[2];
	float fRec41[2];
	float fVec16[8192];
	float fRec40[2];
	float fRec43[2];
	float fVec17[8192];
	float fRec42[2];
	float fRec45[2];
	float fVec18[8192];
	float fRec44[2];
	float fRec47[2];
	float fVec19[8192];
	float fRec46[2];
	float fVec20[2048];
	float fRec30[2];
	float fVec21[2048];
	float fRec28[2];
	float fVec22[2048];
	float fRec26[2];
	float fVec23[2048];
	float fRec24[2];
	
 public:
	mydsp() {
	}
	
	void metadata(Meta* m) { 
		m->declare("author", "RM-CC");
		m->declare("compile_options", "-a .faust2ck_tmp/FreeverbStereoF.dsp-wrapper.cpp -lang cpp -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.1.0");
		m->declare("description", "Original freeverb demo application, modified so it doesn't mix inputs to mono");
		m->declare("filename", "FreeverbStereoF.dsp");
		m->declare("filters.lib/allpass_comb:author", "Julius O. Smith III");
		m->declare("filters.lib/allpass_comb:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/allpass_comb:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/version", "1.3.0");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.8.0");
		m->declare("name", "FreeverbStereo");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("version", "0.0");
	}

	virtual int getNumInputs() {
		return 2;
	}
	virtual int getNumOutputs() {
		return 2;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, float(fSampleRate)));
		fConst1 = 1.764e+04f / fConst0;
		fConst2 = 12348.0f / fConst0;
		iConst3 = int(0.036666665f * fConst0);
		iConst4 = int(0.035306122f * fConst0);
		iConst5 = int(0.033809524f * fConst0);
		iConst6 = int(0.0322449f * fConst0);
		iConst7 = int(0.030748298f * fConst0);
		iConst8 = int(0.028956916f * fConst0);
		iConst9 = int(0.026938776f * fConst0);
		iConst10 = int(0.025306122f * fConst0);
		iConst11 = int(0.0126077095f * fConst0);
		iConst12 = std::min<int>(1024, std::max<int>(0, iConst11 + -1));
		iConst13 = int(0.01f * fConst0);
		iConst14 = std::min<int>(1024, std::max<int>(0, iConst13 + -1));
		iConst15 = int(0.0077324263f * fConst0);
		iConst16 = std::min<int>(1024, std::max<int>(0, iConst15 + -1));
		iConst17 = int(0.0051020407f * fConst0);
		iConst18 = std::min<int>(1024, std::max<int>(0, iConst17 + -1));
		fConst19 = 0.0010430838f * fConst0;
	}
	
	virtual void instanceResetUserInterface() {
		fVslider0 = FAUSTFLOAT(0.12f);
		fVslider1 = FAUSTFLOAT(0.5f);
		fVslider2 = FAUSTFLOAT(0.5f);
		fVslider3 = FAUSTFLOAT(0.5f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fRec9[l0] = 0.0f;
		}
		IOTA0 = 0;
		for (int l1 = 0; l1 < 8192; l1 = l1 + 1) {
			fVec0[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec8[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec11[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 8192; l4 = l4 + 1) {
			fVec1[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fRec10[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fRec13[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 8192; l7 = l7 + 1) {
			fVec2[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
			fRec12[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
			fRec15[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 8192; l10 = l10 + 1) {
			fVec3[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fRec14[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			fRec17[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 8192; l13 = l13 + 1) {
			fVec4[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
			fRec16[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fRec19[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 8192; l16 = l16 + 1) {
			fVec5[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2; l17 = l17 + 1) {
			fRec18[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec21[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 8192; l19 = l19 + 1) {
			fVec6[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 2; l20 = l20 + 1) {
			fRec20[l20] = 0.0f;
		}
		for (int l21 = 0; l21 < 2; l21 = l21 + 1) {
			fRec23[l21] = 0.0f;
		}
		for (int l22 = 0; l22 < 8192; l22 = l22 + 1) {
			fVec7[l22] = 0.0f;
		}
		for (int l23 = 0; l23 < 2; l23 = l23 + 1) {
			fRec22[l23] = 0.0f;
		}
		for (int l24 = 0; l24 < 2048; l24 = l24 + 1) {
			fVec8[l24] = 0.0f;
		}
		for (int l25 = 0; l25 < 2; l25 = l25 + 1) {
			fRec6[l25] = 0.0f;
		}
		for (int l26 = 0; l26 < 2048; l26 = l26 + 1) {
			fVec9[l26] = 0.0f;
		}
		for (int l27 = 0; l27 < 2; l27 = l27 + 1) {
			fRec4[l27] = 0.0f;
		}
		for (int l28 = 0; l28 < 2048; l28 = l28 + 1) {
			fVec10[l28] = 0.0f;
		}
		for (int l29 = 0; l29 < 2; l29 = l29 + 1) {
			fRec2[l29] = 0.0f;
		}
		for (int l30 = 0; l30 < 1024; l30 = l30 + 1) {
			fVec11[l30] = 0.0f;
		}
		for (int l31 = 0; l31 < 2; l31 = l31 + 1) {
			fRec0[l31] = 0.0f;
		}
		for (int l32 = 0; l32 < 2; l32 = l32 + 1) {
			fRec33[l32] = 0.0f;
		}
		for (int l33 = 0; l33 < 8192; l33 = l33 + 1) {
			fVec12[l33] = 0.0f;
		}
		for (int l34 = 0; l34 < 2; l34 = l34 + 1) {
			fRec32[l34] = 0.0f;
		}
		for (int l35 = 0; l35 < 2; l35 = l35 + 1) {
			fRec35[l35] = 0.0f;
		}
		for (int l36 = 0; l36 < 8192; l36 = l36 + 1) {
			fVec13[l36] = 0.0f;
		}
		for (int l37 = 0; l37 < 2; l37 = l37 + 1) {
			fRec34[l37] = 0.0f;
		}
		for (int l38 = 0; l38 < 2; l38 = l38 + 1) {
			fRec37[l38] = 0.0f;
		}
		for (int l39 = 0; l39 < 8192; l39 = l39 + 1) {
			fVec14[l39] = 0.0f;
		}
		for (int l40 = 0; l40 < 2; l40 = l40 + 1) {
			fRec36[l40] = 0.0f;
		}
		for (int l41 = 0; l41 < 2; l41 = l41 + 1) {
			fRec39[l41] = 0.0f;
		}
		for (int l42 = 0; l42 < 8192; l42 = l42 + 1) {
			fVec15[l42] = 0.0f;
		}
		for (int l43 = 0; l43 < 2; l43 = l43 + 1) {
			fRec38[l43] = 0.0f;
		}
		for (int l44 = 0; l44 < 2; l44 = l44 + 1) {
			fRec41[l44] = 0.0f;
		}
		for (int l45 = 0; l45 < 8192; l45 = l45 + 1) {
			fVec16[l45] = 0.0f;
		}
		for (int l46 = 0; l46 < 2; l46 = l46 + 1) {
			fRec40[l46] = 0.0f;
		}
		for (int l47 = 0; l47 < 2; l47 = l47 + 1) {
			fRec43[l47] = 0.0f;
		}
		for (int l48 = 0; l48 < 8192; l48 = l48 + 1) {
			fVec17[l48] = 0.0f;
		}
		for (int l49 = 0; l49 < 2; l49 = l49 + 1) {
			fRec42[l49] = 0.0f;
		}
		for (int l50 = 0; l50 < 2; l50 = l50 + 1) {
			fRec45[l50] = 0.0f;
		}
		for (int l51 = 0; l51 < 8192; l51 = l51 + 1) {
			fVec18[l51] = 0.0f;
		}
		for (int l52 = 0; l52 < 2; l52 = l52 + 1) {
			fRec44[l52] = 0.0f;
		}
		for (int l53 = 0; l53 < 2; l53 = l53 + 1) {
			fRec47[l53] = 0.0f;
		}
		for (int l54 = 0; l54 < 8192; l54 = l54 + 1) {
			fVec19[l54] = 0.0f;
		}
		for (int l55 = 0; l55 < 2; l55 = l55 + 1) {
			fRec46[l55] = 0.0f;
		}
		for (int l56 = 0; l56 < 2048; l56 = l56 + 1) {
			fVec20[l56] = 0.0f;
		}
		for (int l57 = 0; l57 < 2; l57 = l57 + 1) {
			fRec30[l57] = 0.0f;
		}
		for (int l58 = 0; l58 < 2048; l58 = l58 + 1) {
			fVec21[l58] = 0.0f;
		}
		for (int l59 = 0; l59 < 2; l59 = l59 + 1) {
			fRec28[l59] = 0.0f;
		}
		for (int l60 = 0; l60 < 2048; l60 = l60 + 1) {
			fVec22[l60] = 0.0f;
		}
		for (int l61 = 0; l61 < 2; l61 = l61 + 1) {
			fRec26[l61] = 0.0f;
		}
		for (int l62 = 0; l62 < 2048; l62 = l62 + 1) {
			fVec23[l62] = 0.0f;
		}
		for (int l63 = 0; l63 < 2; l63 = l63 + 1) {
			fRec24[l63] = 0.0f;
		}
	}
	
	virtual void init(int sample_rate) {
		classInit(sample_rate);
		instanceInit(sample_rate);
	}
	
	virtual void instanceInit(int sample_rate) {
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
	}
	
	virtual mydsp* clone() {
		return new mydsp();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openHorizontalBox("Freeverb");
		ui_interface->declare(0, "0", "");
		ui_interface->openVerticalBox("0x00");
		ui_interface->declare(&fVslider1, "0", "");
		ui_interface->declare(&fVslider1, "style", "knob");
		ui_interface->declare(&fVslider1, "tooltip", "Somehow control the   density of the reverb.");
		ui_interface->addVerticalSlider("Damp", &fVslider1, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.025f));
		ui_interface->declare(&fVslider2, "1", "");
		ui_interface->declare(&fVslider2, "style", "knob");
		ui_interface->declare(&fVslider2, "tooltip", "The room size   between 0 and 1 with 1 for the largest room.");
		ui_interface->addVerticalSlider("RoomSize", &fVslider2, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.025f));
		ui_interface->declare(&fVslider3, "2", "");
		ui_interface->declare(&fVslider3, "style", "knob");
		ui_interface->declare(&fVslider3, "tooltip", "Spatial   spread between 0 and 1 with 1 for maximum spread.");
		ui_interface->addVerticalSlider("Stereo Spread", &fVslider3, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->declare(&fVslider0, "1", "");
		ui_interface->declare(&fVslider0, "tooltip", "The amount of reverb applied to the signal   between 0 and 1 with 1 for the maximum amount of reverb.");
		ui_interface->addVerticalSlider("Wet", &fVslider0, FAUSTFLOAT(0.12f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.025f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* input1 = inputs[1];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = float(fVslider0);
		float fSlow1 = 1.0f - fSlow0;
		float fSlow2 = fConst1 * float(fVslider1);
		float fSlow3 = 1.0f - fSlow2;
		float fSlow4 = fConst2 * float(fVslider2) + 0.7f;
		int iSlow5 = int(fConst19 * float(fVslider3));
		int iSlow6 = iConst3 + iSlow5;
		int iSlow7 = iConst4 + iSlow5;
		int iSlow8 = iConst5 + iSlow5;
		int iSlow9 = iConst6 + iSlow5;
		int iSlow10 = iConst7 + iSlow5;
		int iSlow11 = iConst8 + iSlow5;
		int iSlow12 = iConst9 + iSlow5;
		int iSlow13 = iConst10 + iSlow5;
		int iSlow14 = iSlow5 + -1;
		int iSlow15 = std::min<int>(1024, std::max<int>(0, iConst11 + iSlow14));
		int iSlow16 = std::min<int>(1024, std::max<int>(0, iConst13 + iSlow14));
		int iSlow17 = std::min<int>(1024, std::max<int>(0, iConst15 + iSlow14));
		int iSlow18 = std::min<int>(1024, std::max<int>(0, iConst17 + iSlow14));
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			float fTemp0 = float(input0[i0]);
			fRec9[0] = fSlow2 * fRec9[1] + fSlow3 * fRec8[1];
			float fTemp1 = fSlow0 * fTemp0;
			fVec0[IOTA0 & 8191] = fTemp1 + fSlow4 * fRec9[0];
			fRec8[0] = fVec0[(IOTA0 - iConst3) & 8191];
			fRec11[0] = fSlow2 * fRec11[1] + fSlow3 * fRec10[1];
			fVec1[IOTA0 & 8191] = fTemp1 + fSlow4 * fRec11[0];
			fRec10[0] = fVec1[(IOTA0 - iConst4) & 8191];
			fRec13[0] = fSlow2 * fRec13[1] + fSlow3 * fRec12[1];
			fVec2[IOTA0 & 8191] = fTemp1 + fSlow4 * fRec13[0];
			fRec12[0] = fVec2[(IOTA0 - iConst5) & 8191];
			fRec15[0] = fSlow2 * fRec15[1] + fSlow3 * fRec14[1];
			fVec3[IOTA0 & 8191] = fTemp1 + fSlow4 * fRec15[0];
			fRec14[0] = fVec3[(IOTA0 - iConst6) & 8191];
			fRec17[0] = fSlow2 * fRec17[1] + fSlow3 * fRec16[1];
			fVec4[IOTA0 & 8191] = fTemp1 + fSlow4 * fRec17[0];
			fRec16[0] = fVec4[(IOTA0 - iConst7) & 8191];
			fRec19[0] = fSlow2 * fRec19[1] + fSlow3 * fRec18[1];
			fVec5[IOTA0 & 8191] = fTemp1 + fSlow4 * fRec19[0];
			fRec18[0] = fVec5[(IOTA0 - iConst8) & 8191];
			fRec21[0] = fSlow2 * fRec21[1] + fSlow3 * fRec20[1];
			fVec6[IOTA0 & 8191] = fTemp1 + fSlow4 * fRec21[0];
			fRec20[0] = fVec6[(IOTA0 - iConst9) & 8191];
			fRec23[0] = fSlow2 * fRec23[1] + fSlow3 * fRec22[1];
			fVec7[IOTA0 & 8191] = fSlow4 * fRec23[0] + fTemp1;
			fRec22[0] = fVec7[(IOTA0 - iConst10) & 8191];
			float fTemp2 = fRec22[0] + fRec20[0] + fRec18[0] + fRec16[0] + fRec14[0] + fRec12[0] + fRec10[0] + fRec8[0] + 0.5f * fRec6[1];
			fVec8[IOTA0 & 2047] = fTemp2;
			fRec6[0] = fVec8[(IOTA0 - iConst12) & 2047];
			float fRec7 = -(0.5f * fTemp2);
			float fTemp3 = fRec6[1] + fRec7 + 0.5f * fRec4[1];
			fVec9[IOTA0 & 2047] = fTemp3;
			fRec4[0] = fVec9[(IOTA0 - iConst14) & 2047];
			float fRec5 = -(0.5f * fTemp3);
			float fTemp4 = fRec4[1] + fRec5 + 0.5f * fRec2[1];
			fVec10[IOTA0 & 2047] = fTemp4;
			fRec2[0] = fVec10[(IOTA0 - iConst16) & 2047];
			float fRec3 = -(0.5f * fTemp4);
			float fTemp5 = fRec2[1] + fRec3 + 0.5f * fRec0[1];
			fVec11[IOTA0 & 1023] = fTemp5;
			fRec0[0] = fVec11[(IOTA0 - iConst18) & 1023];
			float fRec1 = -(0.5f * fTemp5);
			output0[i0] = FAUSTFLOAT(fRec1 + fRec0[1] + fSlow1 * fTemp0);
			float fTemp6 = float(input1[i0]);
			fRec33[0] = fSlow2 * fRec33[1] + fSlow3 * fRec32[1];
			float fTemp7 = fSlow0 * fTemp6;
			fVec12[IOTA0 & 8191] = fTemp7 + fSlow4 * fRec33[0];
			fRec32[0] = fVec12[(IOTA0 - iSlow6) & 8191];
			fRec35[0] = fSlow2 * fRec35[1] + fSlow3 * fRec34[1];
			fVec13[IOTA0 & 8191] = fTemp7 + fSlow4 * fRec35[0];
			fRec34[0] = fVec13[(IOTA0 - iSlow7) & 8191];
			fRec37[0] = fSlow2 * fRec37[1] + fSlow3 * fRec36[1];
			fVec14[IOTA0 & 8191] = fTemp7 + fSlow4 * fRec37[0];
			fRec36[0] = fVec14[(IOTA0 - iSlow8) & 8191];
			fRec39[0] = fSlow2 * fRec39[1] + fSlow3 * fRec38[1];
			fVec15[IOTA0 & 8191] = fTemp7 + fSlow4 * fRec39[0];
			fRec38[0] = fVec15[(IOTA0 - iSlow9) & 8191];
			fRec41[0] = fSlow2 * fRec41[1] + fSlow3 * fRec40[1];
			fVec16[IOTA0 & 8191] = fTemp7 + fSlow4 * fRec41[0];
			fRec40[0] = fVec16[(IOTA0 - iSlow10) & 8191];
			fRec43[0] = fSlow2 * fRec43[1] + fSlow3 * fRec42[1];
			fVec17[IOTA0 & 8191] = fTemp7 + fSlow4 * fRec43[0];
			fRec42[0] = fVec17[(IOTA0 - iSlow11) & 8191];
			fRec45[0] = fSlow2 * fRec45[1] + fSlow3 * fRec44[1];
			fVec18[IOTA0 & 8191] = fTemp7 + fSlow4 * fRec45[0];
			fRec44[0] = fVec18[(IOTA0 - iSlow12) & 8191];
			fRec47[0] = fSlow2 * fRec47[1] + fSlow3 * fRec46[1];
			fVec19[IOTA0 & 8191] = fSlow4 * fRec47[0] + fTemp7;
			fRec46[0] = fVec19[(IOTA0 - iSlow13) & 8191];
			float fTemp8 = fRec46[0] + fRec44[0] + fRec42[0] + fRec40[0] + fRec38[0] + fRec36[0] + fRec34[0] + fRec32[0] + 0.5f * fRec30[1];
			fVec20[IOTA0 & 2047] = fTemp8;
			fRec30[0] = fVec20[(IOTA0 - iSlow15) & 2047];
			float fRec31 = -(0.5f * fTemp8);
			float fTemp9 = fRec30[1] + fRec31 + 0.5f * fRec28[1];
			fVec21[IOTA0 & 2047] = fTemp9;
			fRec28[0] = fVec21[(IOTA0 - iSlow16) & 2047];
			float fRec29 = -(0.5f * fTemp9);
			float fTemp10 = fRec28[1] + fRec29 + 0.5f * fRec26[1];
			fVec22[IOTA0 & 2047] = fTemp10;
			fRec26[0] = fVec22[(IOTA0 - iSlow17) & 2047];
			float fRec27 = -(0.5f * fTemp10);
			float fTemp11 = fRec26[1] + fRec27 + 0.5f * fRec24[1];
			fVec23[IOTA0 & 2047] = fTemp11;
			fRec24[0] = fVec23[(IOTA0 - iSlow18) & 2047];
			float fRec25 = -(0.5f * fTemp11);
			output1[i0] = FAUSTFLOAT(fRec25 + fRec24[1] + fSlow1 * fTemp6);
			fRec9[1] = fRec9[0];
			IOTA0 = IOTA0 + 1;
			fRec8[1] = fRec8[0];
			fRec11[1] = fRec11[0];
			fRec10[1] = fRec10[0];
			fRec13[1] = fRec13[0];
			fRec12[1] = fRec12[0];
			fRec15[1] = fRec15[0];
			fRec14[1] = fRec14[0];
			fRec17[1] = fRec17[0];
			fRec16[1] = fRec16[0];
			fRec19[1] = fRec19[0];
			fRec18[1] = fRec18[0];
			fRec21[1] = fRec21[0];
			fRec20[1] = fRec20[0];
			fRec23[1] = fRec23[0];
			fRec22[1] = fRec22[0];
			fRec6[1] = fRec6[0];
			fRec4[1] = fRec4[0];
			fRec2[1] = fRec2[0];
			fRec0[1] = fRec0[0];
			fRec33[1] = fRec33[0];
			fRec32[1] = fRec32[0];
			fRec35[1] = fRec35[0];
			fRec34[1] = fRec34[0];
			fRec37[1] = fRec37[0];
			fRec36[1] = fRec36[0];
			fRec39[1] = fRec39[0];
			fRec38[1] = fRec38[0];
			fRec41[1] = fRec41[0];
			fRec40[1] = fRec40[0];
			fRec43[1] = fRec43[0];
			fRec42[1] = fRec42[0];
			fRec45[1] = fRec45[0];
			fRec44[1] = fRec44[0];
			fRec47[1] = fRec47[0];
			fRec46[1] = fRec46[0];
			fRec30[1] = fRec30[0];
			fRec28[1] = fRec28[0];
			fRec26[1] = fRec26[0];
			fRec24[1] = fRec24[0];
		}
	}

};

#undef private
#undef virtual
#undef mydsp

/*
 * ChucK glue code
 */
static t_CKUINT FreeverbStereo_offset_data = 0;
static int g_sr = 44100;
static int g_nChans = 1;

CK_DLL_CTOR(FreeverbStereo_ctor)
{
    // return data to be used later
    FreeverbStereo *d = new FreeverbStereo;
    OBJ_MEMBER_UINT(SELF, FreeverbStereo_offset_data) = (t_CKUINT)d;
    d->init(g_sr);
    d->ck_frame_in = new SAMPLE*[g_nChans];
    d->ck_frame_out = new SAMPLE*[g_nChans];
}

CK_DLL_DTOR(FreeverbStereo_dtor)
{
    FreeverbStereo *d = (FreeverbStereo*)OBJ_MEMBER_UINT(SELF, FreeverbStereo_offset_data);

    delete[] d->ck_frame_in;
    delete[] d->ck_frame_out;
    
    delete d;
    
    OBJ_MEMBER_UINT(SELF, FreeverbStereo_offset_data) = 0;
}

// mono tick
CK_DLL_TICK(FreeverbStereo_tick)
{
    FreeverbStereo *d = (FreeverbStereo*)OBJ_MEMBER_UINT(SELF, FreeverbStereo_offset_data);
    
    d->ck_frame_in[0] = &in;
    d->ck_frame_out[0] = out;

    d->compute(1, d->ck_frame_in, d->ck_frame_out);
    
    return TRUE;
}

// multichannel tick
CK_DLL_TICKF(FreeverbStereo_tickf)
{
    FreeverbStereo *d = (FreeverbStereo*)OBJ_MEMBER_UINT(SELF, FreeverbStereo_offset_data);
    
    for(int f = 0; f < nframes; f++)
    {
        // fake-deinterleave
        for(int c = 0; c < g_nChans; c++)
        {
            d->ck_frame_in[c] = &in[f*g_nChans+c];
            d->ck_frame_out[c] = &out[f*g_nChans+c];
        }
        
        d->compute(1, d->ck_frame_in, d->ck_frame_out);
    }
    
    return TRUE;
}

CK_DLL_MFUN(FreeverbStereo_ctrl_fVslider1)
{
    FreeverbStereo *d = (FreeverbStereo*)OBJ_MEMBER_UINT(SELF, FreeverbStereo_offset_data);
    d->fVslider1 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fVslider1);
}

CK_DLL_MFUN(FreeverbStereo_cget_fVslider1)
{
    FreeverbStereo *d = (FreeverbStereo*)OBJ_MEMBER_UINT(SELF, FreeverbStereo_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fVslider1);
}


CK_DLL_MFUN(FreeverbStereo_ctrl_fVslider2)
{
    FreeverbStereo *d = (FreeverbStereo*)OBJ_MEMBER_UINT(SELF, FreeverbStereo_offset_data);
    d->fVslider2 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fVslider2);
}

CK_DLL_MFUN(FreeverbStereo_cget_fVslider2)
{
    FreeverbStereo *d = (FreeverbStereo*)OBJ_MEMBER_UINT(SELF, FreeverbStereo_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fVslider2);
}


CK_DLL_MFUN(FreeverbStereo_ctrl_fVslider3)
{
    FreeverbStereo *d = (FreeverbStereo*)OBJ_MEMBER_UINT(SELF, FreeverbStereo_offset_data);
    d->fVslider3 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fVslider3);
}

CK_DLL_MFUN(FreeverbStereo_cget_fVslider3)
{
    FreeverbStereo *d = (FreeverbStereo*)OBJ_MEMBER_UINT(SELF, FreeverbStereo_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fVslider3);
}


CK_DLL_MFUN(FreeverbStereo_ctrl_fVslider0)
{
    FreeverbStereo *d = (FreeverbStereo*)OBJ_MEMBER_UINT(SELF, FreeverbStereo_offset_data);
    d->fVslider0 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fVslider0);
}

CK_DLL_MFUN(FreeverbStereo_cget_fVslider0)
{
    FreeverbStereo *d = (FreeverbStereo*)OBJ_MEMBER_UINT(SELF, FreeverbStereo_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fVslider0);
}




CK_DLL_QUERY(FreeverbStereo_query)
{
    g_sr = QUERY->srate;

	FreeverbStereo temp; // needed to get IO channel count

    QUERY->setname(QUERY, "FreeverbStereo");
    
    QUERY->begin_class(QUERY, "FreeverbStereo", "UGen");
    QUERY->doc_class(QUERY, "FreeverbStereo");
    QUERY->add_ex(QUERY, "FreeverbStereo-test.ck");
    
    QUERY->add_ctor(QUERY, FreeverbStereo_ctor);
    QUERY->add_dtor(QUERY, FreeverbStereo_dtor);
    
    g_nChans = std::max(temp.getNumInputs(), temp.getNumOutputs());
    
    if(g_nChans == 1)
        QUERY->add_ugen_func(QUERY, FreeverbStereo_tick, NULL, g_nChans, g_nChans);
    else
        QUERY->add_ugen_funcf(QUERY, FreeverbStereo_tickf, NULL, g_nChans, g_nChans);

    // add member variable
    FreeverbStereo_offset_data = QUERY->add_mvar( QUERY, "int", "@FreeverbStereo_data", FALSE );
    if( FreeverbStereo_offset_data == CK_INVALID_OFFSET ) goto error;

    
    QUERY->add_mfun( QUERY, FreeverbStereo_cget_fVslider1 , "float", "Damp" );
    
    QUERY->add_mfun( QUERY, FreeverbStereo_ctrl_fVslider1 , "float", "Damp" );
    QUERY->add_arg( QUERY, "float", "Damp" );
    QUERY->doc_func(QUERY, "float value controls Damp" );
    

    QUERY->add_mfun( QUERY, FreeverbStereo_cget_fVslider2 , "float", "RoomSize" );
    
    QUERY->add_mfun( QUERY, FreeverbStereo_ctrl_fVslider2 , "float", "RoomSize" );
    QUERY->add_arg( QUERY, "float", "RoomSize" );
    QUERY->doc_func(QUERY, "float value controls RoomSize" );
    

    QUERY->add_mfun( QUERY, FreeverbStereo_cget_fVslider3 , "float", "Stereo_Spread" );
    
    QUERY->add_mfun( QUERY, FreeverbStereo_ctrl_fVslider3 , "float", "Stereo_Spread" );
    QUERY->add_arg( QUERY, "float", "Stereo_Spread" );
    QUERY->doc_func(QUERY, "float value controls Stereo_Spread" );
    

    QUERY->add_mfun( QUERY, FreeverbStereo_cget_fVslider0 , "float", "Wet" );
    
    QUERY->add_mfun( QUERY, FreeverbStereo_ctrl_fVslider0 , "float", "Wet" );
    QUERY->add_arg( QUERY, "float", "Wet" );
    QUERY->doc_func(QUERY, "float value controls Wet" );
    


    // end import
	QUERY->end_class(QUERY);
	
    return TRUE;

error:
    // end import
	QUERY->end_class(QUERY);

    return FALSE;
}

#endif
