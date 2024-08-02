/* ------------------------------------------------------------
copyright: "(c)Romain Michon, CCRMA (Stanford University), GRAME"
license: "MIT"
name: "DjembeMIDI"
Code generated with Faust 2.74.5. (https://faust.grame.fr)
Compilation options: -a .faust2ck_tmp/DjembeMIDIF.dsp-wrapper.cpp -lang cpp -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
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
#define mydsp DjembeMIDI

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

static float mydsp_faustpower2_f(float value) {
	return value * value;
}

class mydsp : public dsp {
	
 private:
	
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	FAUSTFLOAT fHslider0;
	float fConst3;
	float fConst4;
	FAUSTFLOAT fButton0;
	float fVec0[2];
	int iRec1[2];
	FAUSTFLOAT fHslider1;
	float fConst5;
	FAUSTFLOAT fHslider2;
	float fConst6;
	int iRec4[2];
	float fRec3[3];
	float fRec2[3];
	FAUSTFLOAT fHslider3;
	float fRec0[3];
	float fConst7;
	float fConst8;
	float fConst9;
	float fRec5[3];
	float fConst10;
	float fConst11;
	float fConst12;
	float fRec6[3];
	float fConst13;
	float fConst14;
	float fConst15;
	float fRec7[3];
	float fConst16;
	float fConst17;
	float fConst18;
	float fRec8[3];
	float fConst19;
	float fConst20;
	float fConst21;
	float fRec9[3];
	float fConst22;
	float fConst23;
	float fConst24;
	float fRec10[3];
	float fConst25;
	float fConst26;
	float fConst27;
	float fRec11[3];
	float fConst28;
	float fConst29;
	float fConst30;
	float fRec12[3];
	float fConst31;
	float fConst32;
	float fConst33;
	float fRec13[3];
	float fConst34;
	float fConst35;
	float fConst36;
	float fRec14[3];
	float fConst37;
	float fConst38;
	float fConst39;
	float fRec15[3];
	float fConst40;
	float fConst41;
	float fConst42;
	float fRec16[3];
	float fConst43;
	float fConst44;
	float fConst45;
	float fRec17[3];
	float fConst46;
	float fConst47;
	float fConst48;
	float fRec18[3];
	float fConst49;
	float fConst50;
	float fConst51;
	float fRec19[3];
	float fConst52;
	float fConst53;
	float fConst54;
	float fRec20[3];
	float fConst55;
	float fConst56;
	float fConst57;
	float fRec21[3];
	float fConst58;
	float fConst59;
	float fConst60;
	float fRec22[3];
	float fConst61;
	float fConst62;
	float fConst63;
	float fRec23[3];
	FAUSTFLOAT fHslider4;
	
 public:
	mydsp() {
	}
	
	void metadata(Meta* m) { 
		m->declare("compile_options", "-a .faust2ck_tmp/DjembeMIDIF.dsp-wrapper.cpp -lang cpp -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("copyright", "(c)Romain Michon, CCRMA (Stanford University), GRAME");
		m->declare("description", "Simple MIDI-controllable djembe physical model.");
		m->declare("envelopes.lib/ar:author", "Yann Orlarey, Stéphane Letz");
		m->declare("envelopes.lib/author", "GRAME");
		m->declare("envelopes.lib/copyright", "GRAME");
		m->declare("envelopes.lib/license", "LGPL with exception");
		m->declare("envelopes.lib/name", "Faust Envelope Library");
		m->declare("envelopes.lib/version", "1.3.0");
		m->declare("filename", "DjembeMIDIF.dsp");
		m->declare("filters.lib/fir:author", "Julius O. Smith III");
		m->declare("filters.lib/fir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/fir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/highpass:author", "Julius O. Smith III");
		m->declare("filters.lib/highpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/iir:author", "Julius O. Smith III");
		m->declare("filters.lib/iir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/iir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/tf2:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf2s:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2s:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2s:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/version", "1.3.0");
		m->declare("license", "MIT");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.8.0");
		m->declare("name", "DjembeMIDI");
		m->declare("noises.lib/name", "Faust Noise Generator Library");
		m->declare("noises.lib/version", "1.4.1");
		m->declare("physmodels.lib/name", "Faust Physical Models Library");
		m->declare("physmodels.lib/version", "1.1.0");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
	}

	virtual int getNumInputs() {
		return 0;
	}
	virtual int getNumOutputs() {
		return 2;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, float(fSampleRate)));
		fConst1 = std::pow(0.001f, 1.6666666f / fConst0);
		fConst2 = mydsp_faustpower2_f(fConst1);
		fConst3 = 6.2831855f / fConst0;
		fConst4 = 2.0f * fConst1;
		fConst5 = 0.002f * fConst0;
		fConst6 = 3.1415927f / fConst0;
		fConst7 = std::pow(0.001f, 33.333332f / fConst0);
		fConst8 = mydsp_faustpower2_f(fConst7);
		fConst9 = 2.0f * fConst7;
		fConst10 = std::pow(0.001f, 16.666666f / fConst0);
		fConst11 = mydsp_faustpower2_f(fConst10);
		fConst12 = 2.0f * fConst10;
		fConst13 = std::pow(0.001f, 11.111111f / fConst0);
		fConst14 = mydsp_faustpower2_f(fConst13);
		fConst15 = 2.0f * fConst13;
		fConst16 = std::pow(0.001f, 8.333333f / fConst0);
		fConst17 = mydsp_faustpower2_f(fConst16);
		fConst18 = 2.0f * fConst16;
		fConst19 = std::pow(0.001f, 6.6666665f / fConst0);
		fConst20 = mydsp_faustpower2_f(fConst19);
		fConst21 = 2.0f * fConst19;
		fConst22 = std::pow(0.001f, 5.5555553f / fConst0);
		fConst23 = mydsp_faustpower2_f(fConst22);
		fConst24 = 2.0f * fConst22;
		fConst25 = std::pow(0.001f, 4.7619047f / fConst0);
		fConst26 = mydsp_faustpower2_f(fConst25);
		fConst27 = 2.0f * fConst25;
		fConst28 = std::pow(0.001f, 4.1666665f / fConst0);
		fConst29 = mydsp_faustpower2_f(fConst28);
		fConst30 = 2.0f * fConst28;
		fConst31 = std::pow(0.001f, 3.7037036f / fConst0);
		fConst32 = mydsp_faustpower2_f(fConst31);
		fConst33 = 2.0f * fConst31;
		fConst34 = std::pow(0.001f, 3.3333333f / fConst0);
		fConst35 = mydsp_faustpower2_f(fConst34);
		fConst36 = 2.0f * fConst34;
		fConst37 = std::pow(0.001f, 3.030303f / fConst0);
		fConst38 = mydsp_faustpower2_f(fConst37);
		fConst39 = 2.0f * fConst37;
		fConst40 = std::pow(0.001f, 2.7777777f / fConst0);
		fConst41 = mydsp_faustpower2_f(fConst40);
		fConst42 = 2.0f * fConst40;
		fConst43 = std::pow(0.001f, 2.5641026f / fConst0);
		fConst44 = mydsp_faustpower2_f(fConst43);
		fConst45 = 2.0f * fConst43;
		fConst46 = std::pow(0.001f, 2.3809524f / fConst0);
		fConst47 = mydsp_faustpower2_f(fConst46);
		fConst48 = 2.0f * fConst46;
		fConst49 = std::pow(0.001f, 2.2222223f / fConst0);
		fConst50 = mydsp_faustpower2_f(fConst49);
		fConst51 = 2.0f * fConst49;
		fConst52 = std::pow(0.001f, 2.0833333f / fConst0);
		fConst53 = mydsp_faustpower2_f(fConst52);
		fConst54 = 2.0f * fConst52;
		fConst55 = std::pow(0.001f, 1.9607843f / fConst0);
		fConst56 = mydsp_faustpower2_f(fConst55);
		fConst57 = 2.0f * fConst55;
		fConst58 = std::pow(0.001f, 1.8518518f / fConst0);
		fConst59 = mydsp_faustpower2_f(fConst58);
		fConst60 = 2.0f * fConst58;
		fConst61 = std::pow(0.001f, 1.754386f / fConst0);
		fConst62 = mydsp_faustpower2_f(fConst61);
		fConst63 = 2.0f * fConst61;
	}
	
	virtual void instanceResetUserInterface() {
		fHslider0 = FAUSTFLOAT(6e+01f);
		fButton0 = FAUSTFLOAT(0.0f);
		fHslider1 = FAUSTFLOAT(0.5f);
		fHslider2 = FAUSTFLOAT(0.5f);
		fHslider3 = FAUSTFLOAT(1.0f);
		fHslider4 = FAUSTFLOAT(1.0f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fVec0[l0] = 0.0f;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			iRec1[l1] = 0;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			iRec4[l2] = 0;
		}
		for (int l3 = 0; l3 < 3; l3 = l3 + 1) {
			fRec3[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 3; l4 = l4 + 1) {
			fRec2[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 3; l5 = l5 + 1) {
			fRec0[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 3; l6 = l6 + 1) {
			fRec5[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 3; l7 = l7 + 1) {
			fRec6[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 3; l8 = l8 + 1) {
			fRec7[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 3; l9 = l9 + 1) {
			fRec8[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 3; l10 = l10 + 1) {
			fRec9[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 3; l11 = l11 + 1) {
			fRec10[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 3; l12 = l12 + 1) {
			fRec11[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 3; l13 = l13 + 1) {
			fRec12[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 3; l14 = l14 + 1) {
			fRec13[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 3; l15 = l15 + 1) {
			fRec14[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 3; l16 = l16 + 1) {
			fRec15[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 3; l17 = l17 + 1) {
			fRec16[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 3; l18 = l18 + 1) {
			fRec17[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 3; l19 = l19 + 1) {
			fRec18[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 3; l20 = l20 + 1) {
			fRec19[l20] = 0.0f;
		}
		for (int l21 = 0; l21 < 3; l21 = l21 + 1) {
			fRec20[l21] = 0.0f;
		}
		for (int l22 = 0; l22 < 3; l22 = l22 + 1) {
			fRec21[l22] = 0.0f;
		}
		for (int l23 = 0; l23 < 3; l23 = l23 + 1) {
			fRec22[l23] = 0.0f;
		}
		for (int l24 = 0; l24 < 3; l24 = l24 + 1) {
			fRec23[l24] = 0.0f;
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
		ui_interface->openVerticalBox("djembe");
		ui_interface->declare(0, "0", "");
		ui_interface->openHorizontalBox("midi");
		ui_interface->declare(&fHslider0, "0", "");
		ui_interface->declare(&fHslider0, "style", "knob");
		ui_interface->addHorizontalSlider("freq", &fHslider0, FAUSTFLOAT(6e+01f), FAUSTFLOAT(5e+01f), FAUSTFLOAT(1e+02f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider3, "2", "");
		ui_interface->declare(&fHslider3, "style", "knob");
		ui_interface->addHorizontalSlider("gain", &fHslider3, FAUSTFLOAT(1.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->declare(0, "1", "");
		ui_interface->openHorizontalBox("otherParams");
		ui_interface->declare(&fHslider2, "0", "");
		ui_interface->declare(&fHslider2, "midi", "ctrl 1");
		ui_interface->declare(&fHslider2, "style", "knob");
		ui_interface->addHorizontalSlider("strikePosition", &fHslider2, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider1, "1", "");
		ui_interface->declare(&fHslider1, "style", "knob");
		ui_interface->addHorizontalSlider("strikeSharpness", &fHslider1, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.01f), FAUSTFLOAT(5.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider4, "2", "");
		ui_interface->declare(&fHslider4, "style", "knob");
		ui_interface->addHorizontalSlider("outGain", &fHslider4, FAUSTFLOAT(1.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->declare(&fButton0, "3", "");
		ui_interface->addButton("gate", &fButton0);
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = float(fHslider0);
		float fSlow1 = fConst4 * std::cos(fConst3 * fSlow0);
		float fSlow2 = float(fButton0);
		float fSlow3 = 1.0f / std::max<float>(1.0f, fConst5 * float(fHslider1));
		float fSlow4 = float(fHslider2);
		float fSlow5 = std::tan(fConst6 * (1.5e+04f * fSlow4 + 5e+02f));
		float fSlow6 = 2.0f * (1.0f - 1.0f / mydsp_faustpower2_f(fSlow5));
		float fSlow7 = 1.0f / fSlow5;
		float fSlow8 = (fSlow7 + -1.4142135f) / fSlow5 + 1.0f;
		float fSlow9 = (fSlow7 + 1.4142135f) / fSlow5 + 1.0f;
		float fSlow10 = 1.0f / fSlow9;
		float fSlow11 = std::tan(fConst6 * (5e+02f * fSlow4 + 4e+01f));
		float fSlow12 = mydsp_faustpower2_f(fSlow11);
		float fSlow13 = 2.0f * (1.0f - 1.0f / fSlow12);
		float fSlow14 = 1.0f / fSlow11;
		float fSlow15 = (fSlow14 + -1.4142135f) / fSlow11 + 1.0f;
		float fSlow16 = (fSlow14 + 1.4142135f) / fSlow11 + 1.0f;
		float fSlow17 = 1.0f / fSlow16;
		float fSlow18 = 1.0f / (fSlow12 * fSlow16);
		float fSlow19 = float(fHslider3) / fSlow9;
		float fSlow20 = fConst9 * std::cos(fConst3 * (fSlow0 + 3.8e+03f));
		float fSlow21 = fConst12 * std::cos(fConst3 * (fSlow0 + 3.6e+03f));
		float fSlow22 = fConst15 * std::cos(fConst3 * (fSlow0 + 3.4e+03f));
		float fSlow23 = fConst18 * std::cos(fConst3 * (fSlow0 + 3.2e+03f));
		float fSlow24 = fConst21 * std::cos(fConst3 * (fSlow0 + 3e+03f));
		float fSlow25 = fConst24 * std::cos(fConst3 * (fSlow0 + 2.8e+03f));
		float fSlow26 = fConst27 * std::cos(fConst3 * (fSlow0 + 2.6e+03f));
		float fSlow27 = fConst30 * std::cos(fConst3 * (fSlow0 + 2.4e+03f));
		float fSlow28 = fConst33 * std::cos(fConst3 * (fSlow0 + 2.2e+03f));
		float fSlow29 = fConst36 * std::cos(fConst3 * (fSlow0 + 2e+03f));
		float fSlow30 = fConst39 * std::cos(fConst3 * (fSlow0 + 1.8e+03f));
		float fSlow31 = fConst42 * std::cos(fConst3 * (fSlow0 + 1.6e+03f));
		float fSlow32 = fConst45 * std::cos(fConst3 * (fSlow0 + 1.4e+03f));
		float fSlow33 = fConst48 * std::cos(fConst3 * (fSlow0 + 1.2e+03f));
		float fSlow34 = fConst51 * std::cos(fConst3 * (fSlow0 + 1e+03f));
		float fSlow35 = fConst54 * std::cos(fConst3 * (fSlow0 + 8e+02f));
		float fSlow36 = fConst57 * std::cos(fConst3 * (fSlow0 + 6e+02f));
		float fSlow37 = fConst60 * std::cos(fConst3 * (fSlow0 + 4e+02f));
		float fSlow38 = fConst63 * std::cos(fConst3 * (fSlow0 + 2e+02f));
		float fSlow39 = 0.05f * float(fHslider4);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			fVec0[0] = fSlow2;
			iRec1[0] = (iRec1[1] + (iRec1[1] > 0)) * (fSlow2 <= fVec0[1]) + (fSlow2 > fVec0[1]);
			float fTemp0 = fSlow3 * float(iRec1[0]);
			iRec4[0] = 1103515245 * iRec4[1] + 12345;
			fRec3[0] = 4.656613e-10f * float(iRec4[0]) - fSlow17 * (fSlow15 * fRec3[2] + fSlow13 * fRec3[1]);
			fRec2[0] = fSlow18 * (fRec3[2] + (fRec3[0] - 2.0f * fRec3[1])) - fSlow10 * (fSlow8 * fRec2[2] + fSlow6 * fRec2[1]);
			float fTemp1 = fSlow19 * (fRec2[2] + fRec2[0] + 2.0f * fRec2[1]) * std::max<float>(0.0f, std::min<float>(fTemp0, 2.0f - fTemp0));
			fRec0[0] = fTemp1 + fSlow1 * fRec0[1] - fConst2 * fRec0[2];
			fRec5[0] = fTemp1 + fSlow20 * fRec5[1] - fConst8 * fRec5[2];
			fRec6[0] = fTemp1 + fSlow21 * fRec6[1] - fConst11 * fRec6[2];
			fRec7[0] = fTemp1 + fSlow22 * fRec7[1] - fConst14 * fRec7[2];
			fRec8[0] = fTemp1 + fSlow23 * fRec8[1] - fConst17 * fRec8[2];
			fRec9[0] = fTemp1 + fSlow24 * fRec9[1] - fConst20 * fRec9[2];
			fRec10[0] = fTemp1 + fSlow25 * fRec10[1] - fConst23 * fRec10[2];
			fRec11[0] = fTemp1 + fSlow26 * fRec11[1] - fConst26 * fRec11[2];
			fRec12[0] = fTemp1 + fSlow27 * fRec12[1] - fConst29 * fRec12[2];
			fRec13[0] = fTemp1 + fSlow28 * fRec13[1] - fConst32 * fRec13[2];
			fRec14[0] = fTemp1 + fSlow29 * fRec14[1] - fConst35 * fRec14[2];
			fRec15[0] = fTemp1 + fSlow30 * fRec15[1] - fConst38 * fRec15[2];
			fRec16[0] = fTemp1 + fSlow31 * fRec16[1] - fConst41 * fRec16[2];
			fRec17[0] = fTemp1 + fSlow32 * fRec17[1] - fConst44 * fRec17[2];
			fRec18[0] = fTemp1 + fSlow33 * fRec18[1] - fConst47 * fRec18[2];
			fRec19[0] = fTemp1 + fSlow34 * fRec19[1] - fConst50 * fRec19[2];
			fRec20[0] = fTemp1 + fSlow35 * fRec20[1] - fConst53 * fRec20[2];
			fRec21[0] = fTemp1 + fSlow36 * fRec21[1] - fConst56 * fRec21[2];
			fRec22[0] = fTemp1 + fSlow37 * fRec22[1] - fConst59 * fRec22[2];
			fRec23[0] = fTemp1 + fSlow38 * fRec23[1] - fConst62 * fRec23[2];
			float fTemp2 = fSlow39 * (fRec0[0] + 0.25f * (fRec23[0] - fRec23[2]) + 0.11111111f * (fRec22[0] - fRec22[2]) + 0.0625f * (fRec21[0] - fRec21[2]) + 0.04f * (fRec20[0] - fRec20[2]) + 0.027777778f * (fRec19[0] - fRec19[2]) + 0.020408163f * (fRec18[0] - fRec18[2]) + 0.015625f * (fRec17[0] - fRec17[2]) + 0.012345679f * (fRec16[0] - fRec16[2]) + 0.01f * (fRec15[0] - fRec15[2]) + 0.008264462f * (fRec14[0] - fRec14[2]) + 0.0069444445f * (fRec13[0] - fRec13[2]) + 0.00591716f * (fRec12[0] - fRec12[2]) + 0.0051020407f * (fRec11[0] - fRec11[2]) + 0.0044444446f * (fRec10[0] - fRec10[2]) + 0.00390625f * (fRec9[0] - fRec9[2]) + 0.0034602077f * (fRec8[0] - fRec8[2]) + 0.0030864198f * (fRec7[0] - fRec7[2]) + 0.002770083f * (fRec6[0] - fRec6[2]) + 0.0025f * (fRec5[0] - fRec5[2]) - fRec0[2]);
			output0[i0] = FAUSTFLOAT(fTemp2);
			output1[i0] = FAUSTFLOAT(fTemp2);
			fVec0[1] = fVec0[0];
			iRec1[1] = iRec1[0];
			iRec4[1] = iRec4[0];
			fRec3[2] = fRec3[1];
			fRec3[1] = fRec3[0];
			fRec2[2] = fRec2[1];
			fRec2[1] = fRec2[0];
			fRec0[2] = fRec0[1];
			fRec0[1] = fRec0[0];
			fRec5[2] = fRec5[1];
			fRec5[1] = fRec5[0];
			fRec6[2] = fRec6[1];
			fRec6[1] = fRec6[0];
			fRec7[2] = fRec7[1];
			fRec7[1] = fRec7[0];
			fRec8[2] = fRec8[1];
			fRec8[1] = fRec8[0];
			fRec9[2] = fRec9[1];
			fRec9[1] = fRec9[0];
			fRec10[2] = fRec10[1];
			fRec10[1] = fRec10[0];
			fRec11[2] = fRec11[1];
			fRec11[1] = fRec11[0];
			fRec12[2] = fRec12[1];
			fRec12[1] = fRec12[0];
			fRec13[2] = fRec13[1];
			fRec13[1] = fRec13[0];
			fRec14[2] = fRec14[1];
			fRec14[1] = fRec14[0];
			fRec15[2] = fRec15[1];
			fRec15[1] = fRec15[0];
			fRec16[2] = fRec16[1];
			fRec16[1] = fRec16[0];
			fRec17[2] = fRec17[1];
			fRec17[1] = fRec17[0];
			fRec18[2] = fRec18[1];
			fRec18[1] = fRec18[0];
			fRec19[2] = fRec19[1];
			fRec19[1] = fRec19[0];
			fRec20[2] = fRec20[1];
			fRec20[1] = fRec20[0];
			fRec21[2] = fRec21[1];
			fRec21[1] = fRec21[0];
			fRec22[2] = fRec22[1];
			fRec22[1] = fRec22[0];
			fRec23[2] = fRec23[1];
			fRec23[1] = fRec23[0];
		}
	}

};

#undef private
#undef virtual
#undef mydsp

/*
 * ChucK glue code
 */
static t_CKUINT DjembeMIDI_offset_data = 0;
static int g_sr = 44100;
static int g_nChans = 1;

CK_DLL_CTOR(DjembeMIDI_ctor)
{
    // return data to be used later
    DjembeMIDI *d = new DjembeMIDI;
    OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data) = (t_CKUINT)d;
    d->init(g_sr);
    d->ck_frame_in = new SAMPLE*[g_nChans];
    d->ck_frame_out = new SAMPLE*[g_nChans];
}

CK_DLL_DTOR(DjembeMIDI_dtor)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);

    delete[] d->ck_frame_in;
    delete[] d->ck_frame_out;
    
    delete d;
    
    OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data) = 0;
}

// mono tick
CK_DLL_TICK(DjembeMIDI_tick)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    
    d->ck_frame_in[0] = &in;
    d->ck_frame_out[0] = out;

    d->compute(1, d->ck_frame_in, d->ck_frame_out);
    
    return TRUE;
}

// multichannel tick
CK_DLL_TICKF(DjembeMIDI_tickf)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    
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

CK_DLL_MFUN(DjembeMIDI_ctrl_fHslider0)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    d->fHslider0 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider0);
}

CK_DLL_MFUN(DjembeMIDI_cget_fHslider0)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider0);
}


CK_DLL_MFUN(DjembeMIDI_ctrl_fHslider3)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    d->fHslider3 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider3);
}

CK_DLL_MFUN(DjembeMIDI_cget_fHslider3)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider3);
}


CK_DLL_MFUN(DjembeMIDI_ctrl_fHslider2)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    d->fHslider2 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider2);
}

CK_DLL_MFUN(DjembeMIDI_cget_fHslider2)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider2);
}


CK_DLL_MFUN(DjembeMIDI_ctrl_fHslider1)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    d->fHslider1 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider1);
}

CK_DLL_MFUN(DjembeMIDI_cget_fHslider1)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider1);
}


CK_DLL_MFUN(DjembeMIDI_ctrl_fHslider4)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    d->fHslider4 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider4);
}

CK_DLL_MFUN(DjembeMIDI_cget_fHslider4)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider4);
}


CK_DLL_MFUN(DjembeMIDI_ctrl_fButton0)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    d->fButton0 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fButton0);
}

CK_DLL_MFUN(DjembeMIDI_cget_fButton0)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fButton0);
}




CK_DLL_QUERY(DjembeMIDI_query)
{
    g_sr = QUERY->srate;

	DjembeMIDI temp; // needed to get IO channel count

    QUERY->setname(QUERY, "DjembeMIDI");
    
    QUERY->begin_class(QUERY, "DjembeMIDI", "UGen");
    QUERY->doc_class(QUERY, "DjembeMIDI");
    QUERY->add_ex(QUERY, "DjembeMIDI-test.ck");
    
    QUERY->add_ctor(QUERY, DjembeMIDI_ctor);
    QUERY->add_dtor(QUERY, DjembeMIDI_dtor);
    
    g_nChans = std::max(temp.getNumInputs(), temp.getNumOutputs());
    
    if(g_nChans == 1)
        QUERY->add_ugen_func(QUERY, DjembeMIDI_tick, NULL, g_nChans, g_nChans);
    else
        QUERY->add_ugen_funcf(QUERY, DjembeMIDI_tickf, NULL, g_nChans, g_nChans);

    // add member variable
    DjembeMIDI_offset_data = QUERY->add_mvar( QUERY, "int", "@DjembeMIDI_data", FALSE );
    if( DjembeMIDI_offset_data == CK_INVALID_OFFSET ) goto error;

    
    QUERY->add_mfun( QUERY, DjembeMIDI_cget_fHslider0 , "float", "freq" );
    
    QUERY->add_mfun( QUERY, DjembeMIDI_ctrl_fHslider0 , "float", "freq" );
    QUERY->add_arg( QUERY, "float", "freq" );
    QUERY->doc_func(QUERY, "float value controls freq" );
    

    QUERY->add_mfun( QUERY, DjembeMIDI_cget_fHslider3 , "float", "gain" );
    
    QUERY->add_mfun( QUERY, DjembeMIDI_ctrl_fHslider3 , "float", "gain" );
    QUERY->add_arg( QUERY, "float", "gain" );
    QUERY->doc_func(QUERY, "float value controls gain" );
    

    QUERY->add_mfun( QUERY, DjembeMIDI_cget_fHslider2 , "float", "strikePosition" );
    
    QUERY->add_mfun( QUERY, DjembeMIDI_ctrl_fHslider2 , "float", "strikePosition" );
    QUERY->add_arg( QUERY, "float", "strikePosition" );
    QUERY->doc_func(QUERY, "float value controls strikePosition" );
    

    QUERY->add_mfun( QUERY, DjembeMIDI_cget_fHslider1 , "float", "strikeSharpness" );
    
    QUERY->add_mfun( QUERY, DjembeMIDI_ctrl_fHslider1 , "float", "strikeSharpness" );
    QUERY->add_arg( QUERY, "float", "strikeSharpness" );
    QUERY->doc_func(QUERY, "float value controls strikeSharpness" );
    

    QUERY->add_mfun( QUERY, DjembeMIDI_cget_fHslider4 , "float", "outGain" );
    
    QUERY->add_mfun( QUERY, DjembeMIDI_ctrl_fHslider4 , "float", "outGain" );
    QUERY->add_arg( QUERY, "float", "outGain" );
    QUERY->doc_func(QUERY, "float value controls outGain" );
    

    QUERY->add_mfun( QUERY, DjembeMIDI_cget_fButton0 , "float", "gate" );
    
    QUERY->add_mfun( QUERY, DjembeMIDI_ctrl_fButton0 , "float", "gate" );
    QUERY->add_arg( QUERY, "float", "gate" );
    QUERY->doc_func(QUERY, "float value controls gate" );
    


    // end import
	QUERY->end_class(QUERY);
	
    return TRUE;

error:
    // end import
	QUERY->end_class(QUERY);

    return FALSE;
}

#endif
