/*
 *  mdaDX10Processor.h
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
#ifndef mdaDX10Processor_h
#define mdaDX10Processor_h

#include "../mdaBaseProcessor.h"
#include "../mdaParam.h"

class DX10Processor : public BaseProcessor
{
private:
    static const unsigned kNumParams = 16;
	static float s_programParams[][kNumParams];
	static const unsigned kNumPrograms = 32;
    static char const *s_presetNames[kNumPrograms];
	static constexpr unsigned kNumVoices = 8;
	static constexpr unsigned kEventBufferSize = 64;
    static ParamDef s_paramDefs[kNumParams];

public:

	DX10Processor(double sampleRate);
	~DX10Processor();

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
		float env;  //carrier envelope
		float dmod; //modulator oscillator 
		float mod0;
		float mod1;
		float menv; //modulator envelope
		float mlev; //modulator target level
		float mdec; //modulator envelope decay
		float car;  //carrier oscillator
		float dcar;
		float cenv; //smoothed env
		float catt; //smoothing
		float cdec; //carrier envelope decay
		unsigned note; //remember what note triggered this
	};

	float Fs;

	using SynthDataT = SynthData<VOICE, kEventBufferSize, kNumVoices>;
	SynthDataT m_synthData;

	///global internal variables
	int K;

	float tune, rati, ratf, ratio; //modulator ratio
	float catt, cdec, crel;        //carrier envelope
	float depth, dept2, mdec, mrel; //modulator envelope
	float lfo0, lfo1, dlfo, modwhl, MW, pbend, velsens, volume, vibrato; //LFO and CC
	float rich, modmix;

	unsigned int m_currentProgram;
};

#endif
