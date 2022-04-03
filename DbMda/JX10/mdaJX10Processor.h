/*
 *  mdaJX10Processor.h
 *
 *  Created by Arne Scheffler on 6/14/08.
 *  VST->Chugin by Dana Batali 4/22
 *
 *  mda VST Plug-ins
 *
 *  Copyright (c) 2008 Paul Kellett
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#ifndef mdaJX10Processor_h
#define mdaJX10Processor_h

#include "../mdaBaseProcessor.h"
#include "../mdaParam.h"

class JX10Processor : public BaseProcessor
{
public:
    enum ParamIds
    {
        kOSCMix = 0,
        kOSCTune = 1,
        kOSCFine = 2,
        kGlide = 3,
        kGldRate = 4,
        kGldBend = 5,
        kVCFFreq = 6,
        kVCFReso = 7,
        kVCFEnv = 8,
        kVCFLFO = 9,
        kVCFVel = 10,
        kVCFAtt = 11,
        kVCFDec = 12,
        kVCFSus = 13,
        kVCFRel = 14,
        kENVAtt = 15,
        kENVDec = 16,
        kENVSus = 17,
        kENVRel = 18,
        kLFORate = 19,
        kVibrato = 20,
        kNoise = 21,
        kOctave = 22,
        kTuning = 23,
        kNumParams = 24
    };

private:
	static float s_programParams[][kNumParams];
	static const unsigned kNumPrograms = 52;
    static char const *s_presetNames[kNumPrograms];
	static constexpr unsigned kNumVoices = 8;
	static constexpr unsigned kEventBufferSize = 64;
    static ParamDef s_paramDefs[kNumParams];

public:
	JX10Processor(double sampleRate);
	~JX10Processor();

    void printPresets() override;
    void selectPreset(int i) override;
    int getNumPresets() override { return kNumPrograms; }

protected:
    void processParamChanges() override;
    void monoProcessing(float *in, float *out, int nframes) override;
    void multiProcessing(float *in, int nIn, float *out, int nout, int nframes) override {} // no-op
	
private:
    void update();
    void processEvents(int &eventPos, int frame);
    void processEvent(MidiEvent &);

	struct VOICE  //voice state
	{
		float  period;
		float  p;    //sinc position
		float  pmax; //loop length
		float  dp;   //delta
		float  sin0; //sine osc
		float  sin1;
		float  sinx;
		float  dc;   //dc offset

		float  detune;
		float  p2;    //sinc position
		float  pmax2; //loop length
		float  dp2;   //delta
		float  sin02; //sine osc
		float  sin12;
		float  sinx2;
		float  dc2;   //dc offset

		float  fc;  //filter cutoff root
		float  ff;  //filter cutoff
		float  f0;  //filter buffers
		float  f1;
		float  f2;

		float  saw;
		//float  vca;  //current level  ///eliminate osc1 level when separate amp & filter envs?
		//float  env;  //envelope
		//float  att;  //attack
		//float  dec;  //decay
		float  env;
		float  envd;
		float  envl;
		float  fenv;
		float  fenvd;
		float  fenvl;

		float  lev;  //osc levels
		float  lev2;
		float  target; //period target
		int    note; //remember what note triggered this
		int    noteID;	// SNA addition
		float  snaPitchbend;// SNA addition
		float  snaVolume;// SNA addition
	};

	using SynthDataT = SynthData<VOICE, kEventBufferSize, kNumVoices>;
	SynthDataT m_synthData;

	///global internal variables
	static const int KMAX = 32;

	///global internal variables
	float semi, cent;
	float tune, detune;
	float filtf, fzip, filtq, filtlfo, filtenv, filtvel, filtwhl;
	float oscmix, noisemix;
	float att, dec, sus, rel, fatt, fdec, fsus, frel;
	float lfo, dlfo, modwhl, press, pbend, ipbend, rezwhl;
	float velsens, volume, voltrim;
	float vibrato, pwmdep, lfoHz, glide, glidedisp;
	int  K, lastnote, veloff, mode;
	unsigned noise;

	unsigned int m_currentProgram;

    void clearVoice(VOICE& v);
};

#endif
