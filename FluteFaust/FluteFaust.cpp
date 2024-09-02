/* ------------------------------------------------------------
copyright: "(c)Romain Michon, CCRMA (Stanford University), GRAME"
license: "MIT"
name: "FluteFaust"
Code generated with Faust 2.74.5. (https://faust.grame.fr)
Compilation options: -a .faust2ck_tmp/FluteFaustF.dsp-wrapper.cpp -lang cpp -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
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
#define mydsp FluteFaust

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

class mydspSIG0 {
	
  private:
	
	int iVec2[2];
	int iRec29[2];
	
  public:
	
	int getNumInputsmydspSIG0() {
		return 0;
	}
	int getNumOutputsmydspSIG0() {
		return 1;
	}
	
	void instanceInitmydspSIG0(int sample_rate) {
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			iVec2[l11] = 0;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			iRec29[l12] = 0;
		}
	}
	
	void fillmydspSIG0(int count, float* table) {
		for (int i1 = 0; i1 < count; i1 = i1 + 1) {
			iVec2[0] = 1;
			iRec29[0] = (iVec2[1] + iRec29[1]) % 65536;
			table[i1] = std::sin(9.58738e-05f * float(iRec29[0]));
			iVec2[1] = iVec2[0];
			iRec29[1] = iRec29[0];
		}
	}

};

static mydspSIG0* newmydspSIG0() { return (mydspSIG0*)new mydspSIG0(); }
static void deletemydspSIG0(mydspSIG0* dsp) { delete dsp; }

static float mydsp_faustpower2_f(float value) {
	return value * value;
}
static float ftbl0mydspSIG0[65536];

class mydsp : public dsp {
	
 private:
	
	int iRec15[2];
	int iVec0[2];
	float fRec21[2];
	float fRec22[2];
	int IOTA0;
	float fRec23[2048];
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	float fConst3;
	FAUSTFLOAT fHslider0;
	float fRec24[2];
	FAUSTFLOAT fHslider1;
	float fRec25[2];
	float fConst4;
	float fVec1[2];
	FAUSTFLOAT fHslider2;
	float fConst5;
	int iRec27[2];
	float fRec26[3];
	FAUSTFLOAT fHslider3;
	FAUSTFLOAT fHslider4;
	float fRec28[2];
	FAUSTFLOAT fHslider5;
	float fConst6;
	float fRec30[2];
	FAUSTFLOAT fHslider6;
	float fVec3[2048];
	float fVec4[2];
	float fRec20[2];
	float fRec11[2048];
	float fRec7[2];
	float fRec3[2048];
	float fRec1[2];
	float fRec2[2];
	float fRec0[2];
	FAUSTFLOAT fHslider7;
	
 public:
	mydsp() {
	}
	
	void metadata(Meta* m) { 
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/tabulateNd", "Copyright (C) 2023 Bart Brouns <bart@magnetophon.nl>");
		m->declare("basics.lib/version", "1.17.1");
		m->declare("compile_options", "-a .faust2ck_tmp/FluteFaustF.dsp-wrapper.cpp -lang cpp -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("copyright", "(c)Romain Michon, CCRMA (Stanford University), GRAME");
		m->declare("delays.lib/fdelay4:author", "Julius O. Smith III");
		m->declare("delays.lib/fdelayltv:author", "Julius O. Smith III");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.1.0");
		m->declare("description", "Simple flute physical model with physical parameters.");
		m->declare("filename", "FluteFaustF.dsp");
		m->declare("filters.lib/dcblocker:author", "Julius O. Smith III");
		m->declare("filters.lib/dcblocker:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/dcblocker:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/fir:author", "Julius O. Smith III");
		m->declare("filters.lib/fir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/fir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/iir:author", "Julius O. Smith III");
		m->declare("filters.lib/iir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/iir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/pole:author", "Julius O. Smith III");
		m->declare("filters.lib/pole:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/pole:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf2:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf2s:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2s:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2s:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/version", "1.3.0");
		m->declare("filters.lib/zero:author", "Julius O. Smith III");
		m->declare("filters.lib/zero:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/zero:license", "MIT-style STK-4.3 license");
		m->declare("license", "MIT");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.8.0");
		m->declare("name", "FluteFaust");
		m->declare("noises.lib/name", "Faust Noise Generator Library");
		m->declare("noises.lib/version", "1.4.1");
		m->declare("oscillators.lib/name", "Faust Oscillator Library");
		m->declare("oscillators.lib/version", "1.5.1");
		m->declare("physmodels.lib/name", "Faust Physical Models Library");
		m->declare("physmodels.lib/version", "1.1.0");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("routes.lib/name", "Faust Signal Routing Library");
		m->declare("routes.lib/version", "1.2.0");
		m->declare("signals.lib/name", "Faust Signal Routing Library");
		m->declare("signals.lib/version", "1.5.0");
	}

	virtual int getNumInputs() {
		return 0;
	}
	virtual int getNumOutputs() {
		return 2;
	}
	
	static void classInit(int sample_rate) {
		mydspSIG0* sig0 = newmydspSIG0();
		sig0->instanceInitmydspSIG0(sample_rate);
		sig0->fillmydspSIG0(65536, ftbl0mydspSIG0);
		deletemydspSIG0(sig0);
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, float(fSampleRate)));
		fConst1 = 0.00882353f * fConst0;
		fConst2 = 44.1f / fConst0;
		fConst3 = 1.0f - fConst2;
		fConst4 = 0.0014705883f * fConst0;
		fConst5 = 3.1415927f / fConst0;
		fConst6 = 1.0f / fConst0;
	}
	
	virtual void instanceResetUserInterface() {
		fHslider0 = FAUSTFLOAT(0.5f);
		fHslider1 = FAUSTFLOAT(0.8f);
		fHslider2 = FAUSTFLOAT(2e+03f);
		fHslider3 = FAUSTFLOAT(0.1f);
		fHslider4 = FAUSTFLOAT(0.0f);
		fHslider5 = FAUSTFLOAT(5.0f);
		fHslider6 = FAUSTFLOAT(0.25f);
		fHslider7 = FAUSTFLOAT(0.5f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iRec15[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			iVec0[l1] = 0;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec21[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec22[l3] = 0.0f;
		}
		IOTA0 = 0;
		for (int l4 = 0; l4 < 2048; l4 = l4 + 1) {
			fRec23[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fRec24[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fRec25[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			fVec1[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
			iRec27[l8] = 0;
		}
		for (int l9 = 0; l9 < 3; l9 = l9 + 1) {
			fRec26[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			fRec28[l10] = 0.0f;
		}
		for (int l13 = 0; l13 < 2; l13 = l13 + 1) {
			fRec30[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 2048; l14 = l14 + 1) {
			fVec3[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fVec4[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 2; l16 = l16 + 1) {
			fRec20[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2048; l17 = l17 + 1) {
			fRec11[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec7[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 2048; l19 = l19 + 1) {
			fRec3[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 2; l20 = l20 + 1) {
			fRec1[l20] = 0.0f;
		}
		for (int l21 = 0; l21 < 2; l21 = l21 + 1) {
			fRec2[l21] = 0.0f;
		}
		for (int l22 = 0; l22 < 2; l22 = l22 + 1) {
			fRec0[l22] = 0.0f;
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
		ui_interface->openHorizontalBox("flute");
		ui_interface->openVerticalBox("blower");
		ui_interface->declare(&fHslider4, "0", "");
		ui_interface->addHorizontalSlider("pressure", &fHslider4, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider3, "1", "");
		ui_interface->addHorizontalSlider("breathGain", &fHslider3, FAUSTFLOAT(0.1f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider2, "2", "");
		ui_interface->addHorizontalSlider("breathCutoff", &fHslider2, FAUSTFLOAT(2e+03f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(2e+04f), FAUSTFLOAT(0.1f));
		ui_interface->declare(&fHslider5, "3", "");
		ui_interface->addHorizontalSlider("vibratoFreq", &fHslider5, FAUSTFLOAT(5.0f), FAUSTFLOAT(0.1f), FAUSTFLOAT(1e+01f), FAUSTFLOAT(0.1f));
		ui_interface->declare(&fHslider6, "4", "");
		ui_interface->addHorizontalSlider("vibratoGain", &fHslider6, FAUSTFLOAT(0.25f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->openVerticalBox("fluteModel");
		ui_interface->declare(&fHslider1, "0", "");
		ui_interface->addHorizontalSlider("tubeLength", &fHslider1, FAUSTFLOAT(0.8f), FAUSTFLOAT(0.01f), FAUSTFLOAT(3.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider0, "1", "");
		ui_interface->addHorizontalSlider("mouthPosition", &fHslider0, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider7, "2", "");
		ui_interface->addHorizontalSlider("outGain", &fHslider7, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = fConst2 * float(fHslider0);
		float fSlow1 = fConst2 * float(fHslider1);
		float fSlow2 = std::tan(fConst5 * float(fHslider2));
		float fSlow3 = 2.0f * (1.0f - 1.0f / mydsp_faustpower2_f(fSlow2));
		float fSlow4 = 1.0f / fSlow2;
		float fSlow5 = (fSlow4 + -1.4142135f) / fSlow2 + 1.0f;
		float fSlow6 = (fSlow4 + 1.4142135f) / fSlow2 + 1.0f;
		float fSlow7 = 1.0f / fSlow6;
		float fSlow8 = 0.05f * (float(fHslider3) / fSlow6);
		float fSlow9 = fConst2 * float(fHslider4);
		float fSlow10 = fConst6 * float(fHslider5);
		float fSlow11 = 0.03f * float(fHslider6);
		float fSlow12 = float(fHslider7);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iRec15[0] = 0;
			iVec0[0] = 1;
			fRec21[0] = 0.285f * fRec20[1] + 0.7f * fRec21[1];
			float fRec19 = fRec21[0] + float(iRec15[1]);
			fRec22[0] = fRec1[1];
			fRec23[IOTA0 & 2047] = 0.95f * fRec22[1];
			fRec24[0] = fSlow0 + fConst3 * fRec24[1];
			float fTemp0 = 0.4f * (fRec24[0] + -0.5f);
			fRec25[0] = fSlow1 + fConst3 * fRec25[1];
			float fTemp1 = fRec25[0] + 0.27f;
			float fTemp2 = fConst4 * fTemp1 * (fTemp0 + 0.27f);
			float fTemp3 = fTemp2 + -1.499995f;
			int iTemp4 = int(fTemp3);
			int iTemp5 = int(std::min<float>(fConst1, float(std::max<int>(0, iTemp4 + 4)))) + 1;
			float fTemp6 = std::floor(fTemp3);
			float fTemp7 = fTemp2 + (-3.0f - fTemp6);
			float fTemp8 = fTemp2 + (-2.0f - fTemp6);
			float fTemp9 = fTemp2 + (-1.0f - fTemp6);
			float fTemp10 = fTemp2 - fTemp6;
			float fTemp11 = fTemp10 * fTemp9;
			float fTemp12 = fTemp11 * fTemp8;
			float fTemp13 = fTemp12 * fTemp7;
			int iTemp14 = int(std::min<float>(fConst1, float(std::max<int>(0, iTemp4 + 3)))) + 1;
			int iTemp15 = int(std::min<float>(fConst1, float(std::max<int>(0, iTemp4 + 2)))) + 1;
			int iTemp16 = int(std::min<float>(fConst1, float(std::max<int>(0, iTemp4 + 1)))) + 1;
			int iTemp17 = int(std::min<float>(fConst1, float(std::max<int>(0, iTemp4)))) + 1;
			float fTemp18 = fTemp2 + (-4.0f - fTemp6);
			fVec1[0] = fTemp18 * (fTemp7 * (fTemp8 * (0.041666668f * fRec23[(IOTA0 - iTemp17) & 2047] * fTemp9 - 0.16666667f * fTemp10 * fRec23[(IOTA0 - iTemp16) & 2047]) + 0.25f * fTemp11 * fRec23[(IOTA0 - iTemp15) & 2047]) - 0.16666667f * fTemp12 * fRec23[(IOTA0 - iTemp14) & 2047]) + 0.041666668f * fTemp13 * fRec23[(IOTA0 - iTemp5) & 2047];
			iRec27[0] = 1103515245 * iRec27[1] + 12345;
			fRec26[0] = 4.656613e-10f * float(iRec27[0]) - fSlow7 * (fSlow5 * fRec26[2] + fSlow3 * fRec26[1]);
			fRec28[0] = fSlow9 + fConst3 * fRec28[1];
			float fTemp19 = ((1 - iVec0[1]) ? 0.0f : fSlow10 + fRec30[1]);
			fRec30[0] = fTemp19 - std::floor(fTemp19);
			float fTemp20 = fSlow11 * ftbl0mydspSIG0[std::max<int>(0, std::min<int>(int(65536.0f * fRec30[0]), 65535))] + fRec28[0] * (fSlow8 * (fRec26[2] + fRec26[0] + 2.0f * fRec26[1]) + 1.0f) - 0.5f * fVec1[1];
			float fTemp21 = 0.5f * fRec7[1] + std::max<float>(-1.0f, std::min<float>(1.0f, fTemp20 * (mydsp_faustpower2_f(fTemp20) + -1.0f)));
			fVec3[IOTA0 & 2047] = fTemp21;
			float fTemp22 = fConst4 * fTemp1 * (0.73f - fTemp0);
			float fTemp23 = fTemp22 + -1.499995f;
			int iTemp24 = int(fTemp23);
			int iTemp25 = int(std::min<float>(fConst1, float(std::max<int>(0, iTemp24 + 4)))) + 1;
			float fTemp26 = std::floor(fTemp23);
			float fTemp27 = fTemp22 + (-3.0f - fTemp26);
			float fTemp28 = fTemp22 + (-2.0f - fTemp26);
			float fTemp29 = fTemp22 + (-1.0f - fTemp26);
			float fTemp30 = fTemp22 - fTemp26;
			float fTemp31 = fTemp30 * fTemp29;
			float fTemp32 = fTemp31 * fTemp28;
			float fTemp33 = fTemp32 * fTemp27;
			int iTemp34 = int(std::min<float>(fConst1, float(std::max<int>(0, iTemp24 + 3)))) + 1;
			int iTemp35 = int(std::min<float>(fConst1, float(std::max<int>(0, iTemp24 + 2)))) + 1;
			int iTemp36 = int(std::min<float>(fConst1, float(std::max<int>(0, iTemp24 + 1)))) + 1;
			int iTemp37 = int(std::min<float>(fConst1, float(std::max<int>(0, iTemp24)))) + 1;
			float fTemp38 = fTemp22 + (-4.0f - fTemp26);
			fVec4[0] = fTemp38 * (fTemp27 * (fTemp28 * (0.041666668f * fVec3[(IOTA0 - iTemp37) & 2047] * fTemp29 - 0.16666667f * fTemp30 * fVec3[(IOTA0 - iTemp36) & 2047]) + 0.25f * fTemp31 * fVec3[(IOTA0 - iTemp35) & 2047]) - 0.16666667f * fTemp32 * fVec3[(IOTA0 - iTemp34) & 2047]) + 0.041666668f * fTemp33 * fVec3[(IOTA0 - iTemp25) & 2047];
			fRec20[0] = fVec4[1];
			float fRec16 = fRec19;
			float fRec17 = fRec20[0];
			float fRec18 = fRec20[0];
			fRec11[IOTA0 & 2047] = fRec16;
			float fRec12 = fTemp38 * (fTemp27 * (fTemp28 * (0.041666668f * fTemp29 * fRec11[(IOTA0 - iTemp37) & 2047] - 0.16666667f * fTemp30 * fRec11[(IOTA0 - iTemp36) & 2047]) + 0.25f * fTemp31 * fRec11[(IOTA0 - iTemp35) & 2047]) - 0.16666667f * fTemp32 * fRec11[(IOTA0 - iTemp34) & 2047]) + 0.041666668f * fTemp33 * fRec11[(IOTA0 - iTemp25) & 2047];
			float fRec13 = fRec17;
			float fRec14 = fRec18;
			fRec7[0] = fRec12;
			float fRec8 = fRec7[1];
			float fRec9 = fRec13;
			float fRec10 = fRec14;
			fRec3[IOTA0 & 2047] = fRec8;
			float fRec4 = fTemp18 * (fTemp7 * (fTemp8 * (0.041666668f * fTemp9 * fRec3[(IOTA0 - iTemp17) & 2047] - 0.16666667f * fTemp10 * fRec3[(IOTA0 - iTemp16) & 2047]) + 0.25f * fTemp11 * fRec3[(IOTA0 - iTemp15) & 2047]) - 0.16666667f * fTemp12 * fRec3[(IOTA0 - iTemp14) & 2047]) + 0.041666668f * fTemp13 * fRec3[(IOTA0 - iTemp5) & 2047];
			float fRec5 = fRec9;
			float fRec6 = fRec10;
			fRec1[0] = fRec4;
			fRec2[0] = fRec6;
			fRec0[0] = fRec2[0] + 0.995f * fRec0[1] - fRec2[1];
			float fTemp39 = fSlow12 * fRec0[0];
			output0[i0] = FAUSTFLOAT(fTemp39);
			output1[i0] = FAUSTFLOAT(fTemp39);
			iRec15[1] = iRec15[0];
			iVec0[1] = iVec0[0];
			fRec21[1] = fRec21[0];
			fRec22[1] = fRec22[0];
			IOTA0 = IOTA0 + 1;
			fRec24[1] = fRec24[0];
			fRec25[1] = fRec25[0];
			fVec1[1] = fVec1[0];
			iRec27[1] = iRec27[0];
			fRec26[2] = fRec26[1];
			fRec26[1] = fRec26[0];
			fRec28[1] = fRec28[0];
			fRec30[1] = fRec30[0];
			fVec4[1] = fVec4[0];
			fRec20[1] = fRec20[0];
			fRec7[1] = fRec7[0];
			fRec1[1] = fRec1[0];
			fRec2[1] = fRec2[0];
			fRec0[1] = fRec0[0];
		}
	}

};

#undef private
#undef virtual
#undef mydsp

/*
 * ChucK glue code
 */
static t_CKUINT FluteFaust_offset_data = 0;
static int g_sr = 44100;
static int g_nChans = 1;

CK_DLL_CTOR(FluteFaust_ctor)
{
    // return data to be used later
    FluteFaust *d = new FluteFaust;
    OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data) = (t_CKUINT)d;
    d->init(g_sr);
    d->ck_frame_in = new SAMPLE*[g_nChans];
    d->ck_frame_out = new SAMPLE*[g_nChans];
}

CK_DLL_DTOR(FluteFaust_dtor)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);

    delete[] d->ck_frame_in;
    delete[] d->ck_frame_out;
    
    delete d;
    
    OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data) = 0;
}

// mono tick
CK_DLL_TICK(FluteFaust_tick)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    
    d->ck_frame_in[0] = &in;
    d->ck_frame_out[0] = out;

    d->compute(1, d->ck_frame_in, d->ck_frame_out);
    
    return TRUE;
}

// multichannel tick
CK_DLL_TICKF(FluteFaust_tickf)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    
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

CK_DLL_MFUN(FluteFaust_ctrl_fHslider4)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider4 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider4);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider4)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider4);
}


CK_DLL_MFUN(FluteFaust_ctrl_fHslider3)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider3 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider3);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider3)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider3);
}


CK_DLL_MFUN(FluteFaust_ctrl_fHslider2)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider2 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider2);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider2)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider2);
}


CK_DLL_MFUN(FluteFaust_ctrl_fHslider5)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider5 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider5);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider5)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider5);
}


CK_DLL_MFUN(FluteFaust_ctrl_fHslider6)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider6 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider6);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider6)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider6);
}


CK_DLL_MFUN(FluteFaust_ctrl_fHslider1)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider1 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider1);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider1)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider1);
}


CK_DLL_MFUN(FluteFaust_ctrl_fHslider0)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider0 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider0);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider0)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider0);
}


CK_DLL_MFUN(FluteFaust_ctrl_fHslider7)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider7 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider7);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider7)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider7);
}




CK_DLL_QUERY(FluteFaust_query)
{
    g_sr = QUERY->srate;

	FluteFaust temp; // needed to get IO channel count

    QUERY->setname(QUERY, "FluteFaust");
    
    QUERY->begin_class(QUERY, "FluteFaust", "UGen");
    QUERY->doc_class(QUERY, "FluteFaust");
    QUERY->add_ex(QUERY, "FluteFaust-test.ck");
    
    QUERY->add_ctor(QUERY, FluteFaust_ctor);
    QUERY->add_dtor(QUERY, FluteFaust_dtor);
    
    g_nChans = std::max(temp.getNumInputs(), temp.getNumOutputs());
    
    if(g_nChans == 1)
        QUERY->add_ugen_func(QUERY, FluteFaust_tick, NULL, g_nChans, g_nChans);
    else
        QUERY->add_ugen_funcf(QUERY, FluteFaust_tickf, NULL, g_nChans, g_nChans);

    // add member variable
    FluteFaust_offset_data = QUERY->add_mvar( QUERY, "int", "@FluteFaust_data", FALSE );
    if( FluteFaust_offset_data == CK_INVALID_OFFSET ) goto error;

    
    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider4 , "float", "pressure" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider4 , "float", "pressure" );
    QUERY->add_arg( QUERY, "float", "pressure" );
    QUERY->doc_func(QUERY, "float value controls pressure" );
    

    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider3 , "float", "breathGain" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider3 , "float", "breathGain" );
    QUERY->add_arg( QUERY, "float", "breathGain" );
    QUERY->doc_func(QUERY, "float value controls breathGain" );
    

    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider2 , "float", "breathCutoff" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider2 , "float", "breathCutoff" );
    QUERY->add_arg( QUERY, "float", "breathCutoff" );
    QUERY->doc_func(QUERY, "float value controls breathCutoff" );
    

    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider5 , "float", "vibratoFreq" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider5 , "float", "vibratoFreq" );
    QUERY->add_arg( QUERY, "float", "vibratoFreq" );
    QUERY->doc_func(QUERY, "float value controls vibratoFreq" );
    

    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider6 , "float", "vibratoGain" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider6 , "float", "vibratoGain" );
    QUERY->add_arg( QUERY, "float", "vibratoGain" );
    QUERY->doc_func(QUERY, "float value controls vibratoGain" );
    

    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider1 , "float", "tubeLength" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider1 , "float", "tubeLength" );
    QUERY->add_arg( QUERY, "float", "tubeLength" );
    QUERY->doc_func(QUERY, "float value controls tubeLength" );
    

    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider0 , "float", "mouthPosition" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider0 , "float", "mouthPosition" );
    QUERY->add_arg( QUERY, "float", "mouthPosition" );
    QUERY->doc_func(QUERY, "float value controls mouthPosition" );
    

    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider7 , "float", "outGain" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider7 , "float", "outGain" );
    QUERY->add_arg( QUERY, "float", "outGain" );
    QUERY->doc_func(QUERY, "float value controls outGain" );
    


    // end import
	QUERY->end_class(QUERY);
	
    return TRUE;

error:
    // end import
	QUERY->end_class(QUERY);

    return FALSE;
}

#endif
