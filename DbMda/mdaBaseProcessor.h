/*
 *  mdaBaseProcessor.h
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

#ifndef mdaBaseProcessor_h
#define mdaBaseProcessor_h

#include "mdaParam.h"
#include <vector>
#include <array>
#include <iostream>

// notes:  
//  - VST3 bypass applies ramp, we'll punt and rely on chuck passthrough
//  - all subclasses should implement processParamChanges
//  - all subclasses should implement doProcessing
//  - processors that have presets should override selectPreset.
//  - instrument subclass should:
//      * instance member of SynthData
//      * make sure to process events during doProcessing

#define kSilenceThreshold 0.000132184039
#define kInvalidNote 999 

class BaseProcessor
{
public:
    virtual void printPresets() { std::cerr << "No presets present\n"; }
    virtual void selectPreset(int i) {}
    virtual int getNumPresets() { return 0; }
    
    int getNumParams() { return m_numParams; }
    float getParamValue(int i) { return m_params[i].value; }
    void setParamValue(int i, float v) {m_params[i].value = v; m_paramsDirty = true;}
    void printParams();
    void addNoteOn(int mnote, float vel)
    {
        this->newMidiEvent(144, mnote, (int) vel*127);
    }
    void addNoteOff(int mnote, float vel)
    {
        this->newMidiEvent(128, mnote, (int) vel*127);
    }
    void addMidiEvent(int status, int data1, int data2,
                        int sampleOffset=0)
    {
        this->newMidiEvent(status, data1, data2, sampleOffset);
    }

    void processMono(float *in, float *out, int nframes);
    void processMulti(float *in, int inch, float *out, int och, int nframes);

protected:
	BaseProcessor(double sampleRate, int nparams, ParamDef const *);
	~BaseProcessor();

    virtual void processParamChanges() = 0;
    virtual void monoProcessing(float *in, float *out, int nframes) = 0;
    virtual void multiProcessing(float *in, int nIn, float *out, int nOut, int nframes) = 0;

	ParamValue* m_params;
    bool m_paramsDirty;
	int m_numParams;
    double m_sampleRate; // Hz (1/sec)
    double m_samplePeriod; // sec

    struct MidiEvent
    {
        int status;
        int data1;
        int data2;
        int sampleOffset;
        bool IsNoteOn() { return (status & 0xF0) == 144; }
        bool IsNoteOff() { return (status & 0xF0) == 128; }
        bool IsPitchWheel() { return (status &0xF0) == 224; }
        bool IsPitchBend() { return IsCC() && data1 == 128; } // non-standard
        bool IsModWheel() { return IsCC() && data1 == 1; }
        float ModWheelFVal() { return data2 / 127.f; }
        int ModWheelVal() { return data2; }
        int PitchWheelVal() { return ((data2 << 16) & data1) - 8192; } // -8192 - 8191
        int PitchBendVal() { return data2; }
        bool IsCC() { return (status &0xF0) == 176; }
        bool IsValid() { return status != 0; }
        int Channel() { return (status & 0x0F); }

        // iff IsNoteOn/Off
        unsigned GetNote() { return this->data1; }
        unsigned GetVelocity() { return this->data2; }
    };

	static constexpr int eventBufferSize = 64;
	static constexpr int eventsDoneID = 99999999;
	using EventArray = std::array<MidiEvent, eventBufferSize>;
	using EventPos = typename EventArray::size_type;
	EventArray m_events;
	EventPos m_eventPos {0};

	bool hasEvents() const noexcept
	{
		return m_eventPos > 0;
	}

    void newMidiEvent(int status, int data1, int data2,
                        int sampleOffset=0)
    {
        if(m_eventPos < eventBufferSize)
        {
            MidiEvent &e = m_events[m_eventPos++];
            e.status = status;
            e.data1 = data1;
            e.data2 = data2;
            e.sampleOffset = sampleOffset;
            if(m_eventPos < eventBufferSize)
                m_events[m_eventPos].status = 0; // empty
        }
        else
            std::cerr << "mdaBase WARNING MIDI event overflow\n";
    }

	void clearEvents() noexcept
	{
		m_eventPos = 0;
		m_events[0].status = 0;
		m_events[0].sampleOffset = eventsDoneID;
	}

};

//------------------------------------------------------------------------
template <typename VoiceT, int kEventBufferSize, int kNumVoices>
struct SynthData
{
	using VOICE = VoiceT;
	static constexpr int numVoices = kNumVoices;

	using VoiceArray = std::array<VOICE, kNumVoices>;
	VoiceArray voice;
	int activevoices {0};
	int sustain {0};

	void init() noexcept { activevoices = 0; }
};

#endif