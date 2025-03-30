/* ------------------------------------------------------------
copyright: "(c)Romain Michon, CCRMA (Stanford University), GRAME"
license: "MIT"
name: "BrassFaust"
Code generated with Faust 2.74.5. (https://faust.grame.fr)
Compilation options: -a .faust2ck_tmp/BrassFaustF.dsp-wrapper.cpp -lang cpp -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
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
#define mydsp BrassFaust

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
	
	int iVec1[2];
	int iRec23[2];
	
  public:
	
	int getNumInputsmydspSIG0() {
		return 0;
	}
	int getNumOutputsmydspSIG0() {
		return 1;
	}
	
	void instanceInitmydspSIG0(int sample_rate) {
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			iVec1[l10] = 0;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			iRec23[l11] = 0;
		}
	}
	
	void fillmydspSIG0(int count, float* table) {
		for (int i1 = 0; i1 < count; i1 = i1 + 1) {
			iVec1[0] = 1;
			iRec23[0] = (iVec1[1] + iRec23[1]) % 65536;
			table[i1] = std::sin(9.58738e-05f * float(iRec23[0]));
			iVec1[1] = iVec1[0];
			iRec23[1] = iRec23[0];
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
	
	FAUSTFLOAT fHslider0;
	int iRec6[2];
	int iVec0[2];
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	FAUSTFLOAT fHslider1;
	float fRec13[2];
	float fRec12[2];
	float fRec14[2];
	FAUSTFLOAT fHslider2;
	float fRec18[2];
	FAUSTFLOAT fHslider3;
	float fRec19[2];
	float fConst3;
	FAUSTFLOAT fHslider4;
	float fConst4;
	int iRec21[2];
	float fRec20[3];
	FAUSTFLOAT fHslider5;
	FAUSTFLOAT fHslider6;
	float fRec22[2];
	FAUSTFLOAT fHslider7;
	float fConst5;
	float fRec24[2];
	FAUSTFLOAT fHslider8;
	float fRec17[3];
	float fVec2[2];
	float fRec16[2];
	int IOTA0;
	float fRec15[2048];
	float fConst6;
	float fConst7;
	float fVec3[2];
	float fRec11[2];
	float fRec2[2048];
	float fRec0[2];
	
 public:
	mydsp() {
	}
	
	void metadata(Meta* m) { 
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/tabulateNd", "Copyright (C) 2023 Bart Brouns <bart@magnetophon.nl>");
		m->declare("basics.lib/version", "1.17.1");
		m->declare("compile_options", "-a .faust2ck_tmp/BrassFaustF.dsp-wrapper.cpp -lang cpp -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("copyright", "(c)Romain Michon, CCRMA (Stanford University), GRAME");
		m->declare("delays.lib/fdelay4:author", "Julius O. Smith III");
		m->declare("delays.lib/fdelayltv:author", "Julius O. Smith III");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.1.0");
		m->declare("description", "Simple brass instrument physical model with physical parameters.");
		m->declare("filename", "BrassFaustF.dsp");
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
		m->declare("name", "BrassFaust");
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
		fConst1 = 44.1f / fConst0;
		fConst2 = 1.0f - fConst1;
		fConst3 = 2136.283f / fConst0;
		fConst4 = 3.1415927f / fConst0;
		fConst5 = 1.0f / fConst0;
		fConst6 = 0.00882353f * fConst0;
		fConst7 = 0.0014705883f * fConst0;
	}
	
	virtual void instanceResetUserInterface() {
		fHslider0 = FAUSTFLOAT(0.0f);
		fHslider1 = FAUSTFLOAT(0.5f);
		fHslider2 = FAUSTFLOAT(0.5f);
		fHslider3 = FAUSTFLOAT(0.5f);
		fHslider4 = FAUSTFLOAT(2e+03f);
		fHslider5 = FAUSTFLOAT(0.1f);
		fHslider6 = FAUSTFLOAT(0.0f);
		fHslider7 = FAUSTFLOAT(5.0f);
		fHslider8 = FAUSTFLOAT(0.25f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iRec6[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			iVec0[l1] = 0;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec13[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec12[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fRec14[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fRec18[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fRec19[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			iRec21[l7] = 0;
		}
		for (int l8 = 0; l8 < 3; l8 = l8 + 1) {
			fRec20[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
			fRec22[l9] = 0.0f;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			fRec24[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 3; l13 = l13 + 1) {
			fRec17[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
			fVec2[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fRec16[l15] = 0.0f;
		}
		IOTA0 = 0;
		for (int l16 = 0; l16 < 2048; l16 = l16 + 1) {
			fRec15[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2; l17 = l17 + 1) {
			fVec3[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec11[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 2048; l19 = l19 + 1) {
			fRec2[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 2; l20 = l20 + 1) {
			fRec0[l20] = 0.0f;
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
		ui_interface->openVerticalBox("BrassFaust");
		ui_interface->openHorizontalBox("brass");
		ui_interface->openVerticalBox("blower");
		ui_interface->declare(&fHslider6, "0", "");
		ui_interface->addHorizontalSlider("pressure", &fHslider6, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider5, "1", "");
		ui_interface->addHorizontalSlider("breathGain", &fHslider5, FAUSTFLOAT(0.1f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider4, "2", "");
		ui_interface->addHorizontalSlider("breathCutoff", &fHslider4, FAUSTFLOAT(2e+03f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(2e+04f), FAUSTFLOAT(0.1f));
		ui_interface->declare(&fHslider7, "3", "");
		ui_interface->addHorizontalSlider("vibratoFreq", &fHslider7, FAUSTFLOAT(5.0f), FAUSTFLOAT(0.1f), FAUSTFLOAT(1e+01f), FAUSTFLOAT(0.1f));
		ui_interface->declare(&fHslider8, "4", "");
		ui_interface->addHorizontalSlider("vibratoGain", &fHslider8, FAUSTFLOAT(0.25f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->openVerticalBox("brassModel");
		ui_interface->declare(&fHslider2, "1", "");
		ui_interface->addHorizontalSlider("tubeLength", &fHslider2, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.01f), FAUSTFLOAT(2.5f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider3, "2", "");
		ui_interface->addHorizontalSlider("lipsTension", &fHslider3, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider1, "3", "");
		ui_interface->addHorizontalSlider("mute", &fHslider1, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->closeBox();
		ui_interface->addHorizontalSlider("fb", &fHslider0, FAUSTFLOAT(0.0f), FAUSTFLOAT(-0.999f), FAUSTFLOAT(0.999f), FAUSTFLOAT(0.001f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = float(fHslider0);
		float fSlow1 = fConst1 * float(fHslider1);
		float fSlow2 = fConst1 * float(fHslider2);
		float fSlow3 = fConst1 * float(fHslider3);
		float fSlow4 = std::tan(fConst4 * float(fHslider4));
		float fSlow5 = 2.0f * (1.0f - 1.0f / mydsp_faustpower2_f(fSlow4));
		float fSlow6 = 1.0f / fSlow4;
		float fSlow7 = (fSlow6 + -1.4142135f) / fSlow4 + 1.0f;
		float fSlow8 = (fSlow6 + 1.4142135f) / fSlow4 + 1.0f;
		float fSlow9 = 1.0f / fSlow8;
		float fSlow10 = 0.05f * (float(fHslider5) / fSlow8);
		float fSlow11 = fConst1 * float(fHslider6);
		float fSlow12 = fConst5 * float(fHslider7);
		float fSlow13 = 0.03f * float(fHslider8);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			output0[i0] = FAUSTFLOAT(fSlow0);
			iRec6[0] = 0;
			iVec0[0] = 1;
			fRec13[0] = fSlow1 + fConst2 * fRec13[1];
			fRec12[0] = (1.0f - fRec13[0]) * fRec11[1] + fRec13[0] * fRec12[1];
			float fRec10 = fRec12[0] + float(iRec6[1]);
			fRec14[0] = fRec0[1];
			fRec18[0] = fSlow2 + fConst2 * fRec18[1];
			fRec19[0] = fSlow3 + fConst2 * fRec19[1];
			iRec21[0] = 1103515245 * iRec21[1] + 12345;
			fRec20[0] = 4.656613e-10f * float(iRec21[0]) - fSlow9 * (fSlow7 * fRec20[2] + fSlow5 * fRec20[1]);
			fRec22[0] = fSlow11 + fConst2 * fRec22[1];
			float fTemp0 = ((1 - iVec0[1]) ? 0.0f : fSlow12 + fRec24[1]);
			fRec24[0] = fTemp0 - std::floor(fTemp0);
			float fTemp1 = fSlow13 * ftbl0mydspSIG0[std::max<int>(0, std::min<int>(int(65536.0f * fRec24[0]), 65535))] + fRec22[0] * (fSlow10 * (fRec20[2] + fRec20[0] + 2.0f * fRec20[1]) + 1.0f);
			fRec17[0] = 0.03f * (0.3f * fTemp1 - 0.85f * fRec14[1]) + 1.994f * fRec17[1] * std::cos(fConst3 * (std::pow(4.0f, 2.0f * fRec19[0] + -1.0f) / fRec18[0])) - 0.994009f * fRec17[2];
			float fTemp2 = std::max<float>(-1.0f, std::min<float>(1.0f, mydsp_faustpower2_f(fRec17[0])));
			float fTemp3 = 0.85f * fRec14[1] * (1.0f - fTemp2);
			float fTemp4 = 0.3f * fTemp1 * fTemp2;
			fVec2[0] = fTemp4 + fTemp3;
			fRec16[0] = fTemp3 + 0.995f * fRec16[1] + fTemp4 - fVec2[1];
			fRec15[IOTA0 & 2047] = fRec16[0];
			float fTemp5 = fConst7 * fRec18[0];
			float fTemp6 = fTemp5 + -1.499995f;
			int iTemp7 = int(fTemp6);
			int iTemp8 = int(std::min<float>(fConst6, float(std::max<int>(0, iTemp7 + 4)))) + 1;
			float fTemp9 = std::floor(fTemp6);
			float fTemp10 = fTemp5 + (-3.0f - fTemp9);
			float fTemp11 = fTemp5 + (-2.0f - fTemp9);
			float fTemp12 = fTemp5 + (-1.0f - fTemp9);
			float fTemp13 = fTemp5 - fTemp9;
			float fTemp14 = fTemp13 * fTemp12;
			float fTemp15 = fTemp14 * fTemp11;
			float fTemp16 = fTemp15 * fTemp10;
			int iTemp17 = int(std::min<float>(fConst6, float(std::max<int>(0, iTemp7 + 3)))) + 1;
			int iTemp18 = int(std::min<float>(fConst6, float(std::max<int>(0, iTemp7 + 2)))) + 1;
			int iTemp19 = int(std::min<float>(fConst6, float(std::max<int>(0, iTemp7 + 1)))) + 1;
			int iTemp20 = int(std::min<float>(fConst6, float(std::max<int>(0, iTemp7)))) + 1;
			float fTemp21 = fTemp5 + (-4.0f - fTemp9);
			fVec3[0] = fTemp21 * (fTemp10 * (fTemp11 * (0.041666668f * fRec15[(IOTA0 - iTemp20) & 2047] * fTemp12 - 0.16666667f * fTemp13 * fRec15[(IOTA0 - iTemp19) & 2047]) + 0.25f * fTemp14 * fRec15[(IOTA0 - iTemp18) & 2047]) - 0.16666667f * fTemp15 * fRec15[(IOTA0 - iTemp17) & 2047]) + 0.041666668f * fTemp16 * fRec15[(IOTA0 - iTemp8) & 2047];
			fRec11[0] = fVec3[1];
			float fRec7 = fRec10;
			float fRec8 = fRec11[0];
			float fRec9 = fRec11[0];
			fRec2[IOTA0 & 2047] = fRec7;
			float fRec3 = fTemp21 * (fTemp10 * (fTemp11 * (0.041666668f * fTemp12 * fRec2[(IOTA0 - iTemp20) & 2047] - 0.16666667f * fTemp13 * fRec2[(IOTA0 - iTemp19) & 2047]) + 0.25f * fTemp14 * fRec2[(IOTA0 - iTemp18) & 2047]) - 0.16666667f * fTemp15 * fRec2[(IOTA0 - iTemp17) & 2047]) + 0.041666668f * fTemp16 * fRec2[(IOTA0 - iTemp8) & 2047];
			float fRec4 = fRec8;
			float fRec5 = fRec9;
			fRec0[0] = fRec3;
			float fRec1 = fRec5;
			output1[i0] = FAUSTFLOAT(fRec1);
			iRec6[1] = iRec6[0];
			iVec0[1] = iVec0[0];
			fRec13[1] = fRec13[0];
			fRec12[1] = fRec12[0];
			fRec14[1] = fRec14[0];
			fRec18[1] = fRec18[0];
			fRec19[1] = fRec19[0];
			iRec21[1] = iRec21[0];
			fRec20[2] = fRec20[1];
			fRec20[1] = fRec20[0];
			fRec22[1] = fRec22[0];
			fRec24[1] = fRec24[0];
			fRec17[2] = fRec17[1];
			fRec17[1] = fRec17[0];
			fVec2[1] = fVec2[0];
			fRec16[1] = fRec16[0];
			IOTA0 = IOTA0 + 1;
			fVec3[1] = fVec3[0];
			fRec11[1] = fRec11[0];
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
static t_CKUINT BrassFaust_offset_data = 0;
static int g_sr = 44100;
static int g_nChans = 1;

CK_DLL_CTOR(BrassFaust_ctor)
{
    // return data to be used later
    BrassFaust *d = new BrassFaust;
    OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data) = (t_CKUINT)d;
    d->init(g_sr);
    d->ck_frame_in = new SAMPLE*[g_nChans];
    d->ck_frame_out = new SAMPLE*[g_nChans];
}

CK_DLL_DTOR(BrassFaust_dtor)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);

    delete[] d->ck_frame_in;
    delete[] d->ck_frame_out;
    
    delete d;
    
    OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data) = 0;
}

// mono tick
CK_DLL_TICK(BrassFaust_tick)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    
    d->ck_frame_in[0] = &in;
    d->ck_frame_out[0] = out;

    d->compute(1, d->ck_frame_in, d->ck_frame_out);
    
    return TRUE;
}

// multichannel tick
CK_DLL_TICKF(BrassFaust_tickf)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    
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

CK_DLL_MFUN(BrassFaust_ctrl_fHslider6)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    d->fHslider6 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider6);
}

CK_DLL_MFUN(BrassFaust_cget_fHslider6)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider6);
}


CK_DLL_MFUN(BrassFaust_ctrl_fHslider5)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    d->fHslider5 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider5);
}

CK_DLL_MFUN(BrassFaust_cget_fHslider5)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider5);
}


CK_DLL_MFUN(BrassFaust_ctrl_fHslider4)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    d->fHslider4 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider4);
}

CK_DLL_MFUN(BrassFaust_cget_fHslider4)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider4);
}


CK_DLL_MFUN(BrassFaust_ctrl_fHslider7)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    d->fHslider7 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider7);
}

CK_DLL_MFUN(BrassFaust_cget_fHslider7)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider7);
}


CK_DLL_MFUN(BrassFaust_ctrl_fHslider8)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    d->fHslider8 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider8);
}

CK_DLL_MFUN(BrassFaust_cget_fHslider8)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider8);
}


CK_DLL_MFUN(BrassFaust_ctrl_fHslider2)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    d->fHslider2 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider2);
}

CK_DLL_MFUN(BrassFaust_cget_fHslider2)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider2);
}


CK_DLL_MFUN(BrassFaust_ctrl_fHslider3)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    d->fHslider3 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider3);
}

CK_DLL_MFUN(BrassFaust_cget_fHslider3)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider3);
}


CK_DLL_MFUN(BrassFaust_ctrl_fHslider1)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    d->fHslider1 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider1);
}

CK_DLL_MFUN(BrassFaust_cget_fHslider1)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider1);
}


CK_DLL_MFUN(BrassFaust_ctrl_fHslider0)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    d->fHslider0 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider0);
}

CK_DLL_MFUN(BrassFaust_cget_fHslider0)
{
    BrassFaust *d = (BrassFaust*)OBJ_MEMBER_UINT(SELF, BrassFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider0);
}




CK_DLL_QUERY(BrassFaust_query)
{
    g_sr = QUERY->srate;

	BrassFaust temp; // needed to get IO channel count

    QUERY->setname(QUERY, "BrassFaust");
    
    QUERY->begin_class(QUERY, "BrassFaust", "UGen");
    QUERY->doc_class(QUERY, "BrassFaust");
    QUERY->add_ex(QUERY, "BrassFaust-test.ck");
    
    QUERY->add_ctor(QUERY, BrassFaust_ctor);
    QUERY->add_dtor(QUERY, BrassFaust_dtor);
    
    g_nChans = std::max(temp.getNumInputs(), temp.getNumOutputs());
    
    if(g_nChans == 1)
        QUERY->add_ugen_func(QUERY, BrassFaust_tick, NULL, g_nChans, g_nChans);
    else
        QUERY->add_ugen_funcf(QUERY, BrassFaust_tickf, NULL, g_nChans, g_nChans);

    // add member variable
    BrassFaust_offset_data = QUERY->add_mvar( QUERY, "int", "@BrassFaust_data", FALSE );
    if( BrassFaust_offset_data == CK_INVALID_OFFSET ) goto error;

    
    QUERY->add_mfun( QUERY, BrassFaust_cget_fHslider6 , "float", "pressure" );
    
    QUERY->add_mfun( QUERY, BrassFaust_ctrl_fHslider6 , "float", "pressure" );
    QUERY->add_arg( QUERY, "float", "pressure" );
    QUERY->doc_func(QUERY, "float value controls pressure" );
    

    QUERY->add_mfun( QUERY, BrassFaust_cget_fHslider5 , "float", "breathGain" );
    
    QUERY->add_mfun( QUERY, BrassFaust_ctrl_fHslider5 , "float", "breathGain" );
    QUERY->add_arg( QUERY, "float", "breathGain" );
    QUERY->doc_func(QUERY, "float value controls breathGain" );
    

    QUERY->add_mfun( QUERY, BrassFaust_cget_fHslider4 , "float", "breathCutoff" );
    
    QUERY->add_mfun( QUERY, BrassFaust_ctrl_fHslider4 , "float", "breathCutoff" );
    QUERY->add_arg( QUERY, "float", "breathCutoff" );
    QUERY->doc_func(QUERY, "float value controls breathCutoff" );
    

    QUERY->add_mfun( QUERY, BrassFaust_cget_fHslider7 , "float", "vibratoFreq" );
    
    QUERY->add_mfun( QUERY, BrassFaust_ctrl_fHslider7 , "float", "vibratoFreq" );
    QUERY->add_arg( QUERY, "float", "vibratoFreq" );
    QUERY->doc_func(QUERY, "float value controls vibratoFreq" );
    

    QUERY->add_mfun( QUERY, BrassFaust_cget_fHslider8 , "float", "vibratoGain" );
    
    QUERY->add_mfun( QUERY, BrassFaust_ctrl_fHslider8 , "float", "vibratoGain" );
    QUERY->add_arg( QUERY, "float", "vibratoGain" );
    QUERY->doc_func(QUERY, "float value controls vibratoGain" );
    

    QUERY->add_mfun( QUERY, BrassFaust_cget_fHslider2 , "float", "tubeLength" );
    
    QUERY->add_mfun( QUERY, BrassFaust_ctrl_fHslider2 , "float", "tubeLength" );
    QUERY->add_arg( QUERY, "float", "tubeLength" );
    QUERY->doc_func(QUERY, "float value controls tubeLength" );
    

    QUERY->add_mfun( QUERY, BrassFaust_cget_fHslider3 , "float", "lipsTension" );
    
    QUERY->add_mfun( QUERY, BrassFaust_ctrl_fHslider3 , "float", "lipsTension" );
    QUERY->add_arg( QUERY, "float", "lipsTension" );
    QUERY->doc_func(QUERY, "float value controls lipsTension" );
    

    QUERY->add_mfun( QUERY, BrassFaust_cget_fHslider1 , "float", "mute" );
    
    QUERY->add_mfun( QUERY, BrassFaust_ctrl_fHslider1 , "float", "mute" );
    QUERY->add_arg( QUERY, "float", "mute" );
    QUERY->doc_func(QUERY, "float value controls mute" );
    

    QUERY->add_mfun( QUERY, BrassFaust_cget_fHslider0 , "float", "fb" );
    
    QUERY->add_mfun( QUERY, BrassFaust_ctrl_fHslider0 , "float", "fb" );
    QUERY->add_arg( QUERY, "float", "fb" );
    QUERY->doc_func(QUERY, "float value controls fb" );
    


    // end import
	QUERY->end_class(QUERY);
	
    return TRUE;

error:
    // end import
	QUERY->end_class(QUERY);

    return FALSE;
}

#endif
