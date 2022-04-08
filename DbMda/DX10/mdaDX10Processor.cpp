/*
 *  mdaDX10Processor.cpp
 *  mda-vst3
 *
 *  Created by Arne Scheffler on 6/14/08.
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

#include "mdaDX10Processor.h"

#include <cmath>
#include <iostream>
#include <cassert>
#include <cstring>

#define NOUTS    2       //number of outputs
#define SILENCE 0.0003f  //voice choking

enum paramIds
{
    kAttack = 0,
    kDecay = 1,
    kRelease = 2,
    kCoarse = 3,
    kFine = 4,
    kModInit = 5,
    kModDec = 6,
    kModSus = 7,
    kModRel = 8,
    kModVel = 9,
    kVibrato = 10,
    kOctave = 11,
    kFineTune = 12,
    kWaveForm = 13,
    kModThru = 14,
    kLFORate = 15
};

float DX10Processor::s_programParams[][kNumParams] = 
{ 
	{0.000f, 0.650f, 0.441f, 0.842f, 0.329f, 0.230f, 0.800f, 0.050f, 0.800f, 0.900f, 0.000f, 0.500f, 0.500f, 0.447f, 0.000f, 0.414f },
	{0.000f, 0.500f, 0.100f, 0.671f, 0.000f, 0.441f, 0.336f, 0.243f, 0.800f, 0.500f, 0.000f, 0.500f, 0.500f, 0.178f, 0.000f, 0.500f },
	{0.000f, 0.700f, 0.400f, 0.230f, 0.184f, 0.270f, 0.474f, 0.224f, 0.800f, 0.974f, 0.250f, 0.500f, 0.500f, 0.428f, 0.836f, 0.500f },
	{0.000f, 0.700f, 0.400f, 0.320f, 0.217f, 0.599f, 0.670f, 0.309f, 0.800f, 0.500f, 0.263f, 0.507f, 0.500f, 0.276f, 0.638f, 0.526f },
	{0.400f, 0.600f, 0.650f, 0.760f, 0.000f, 0.390f, 0.250f, 0.160f, 0.900f, 0.500f, 0.362f, 0.500f, 0.500f, 0.401f, 0.296f, 0.493f },
	{0.000f, 0.342f, 0.000f, 0.280f, 0.000f, 0.880f, 0.100f, 0.408f, 0.740f, 0.000f, 0.000f, 0.600f, 0.500f, 0.842f, 0.651f, 0.500f },
	{0.000f, 0.400f, 0.100f, 0.360f, 0.000f, 0.875f, 0.160f, 0.592f, 0.800f, 0.500f, 0.000f, 0.500f, 0.500f, 0.303f, 0.868f, 0.500f },
	{0.000f, 0.500f, 0.704f, 0.230f, 0.000f, 0.151f, 0.750f, 0.493f, 0.770f, 0.500f, 0.000f, 0.400f, 0.500f, 0.421f, 0.632f, 0.500f },
	{0.600f, 0.990f, 0.400f, 0.320f, 0.283f, 0.570f, 0.300f, 0.050f, 0.240f, 0.500f, 0.138f, 0.500f, 0.500f, 0.283f, 0.822f, 0.500f },
	{0.000f, 0.500f, 0.650f, 0.368f, 0.651f, 0.395f, 0.550f, 0.257f, 0.900f, 0.500f, 0.300f, 0.800f, 0.500f, 0.000f, 0.414f, 0.500f },
	{0.000f, 0.700f, 0.520f, 0.230f, 0.197f, 0.520f, 0.720f, 0.280f, 0.730f, 0.500f, 0.250f, 0.500f, 0.500f, 0.336f, 0.428f, 0.500f },
	{0.000f, 0.240f, 0.000f, 0.390f, 0.000f, 0.880f, 0.100f, 0.600f, 0.740f, 0.500f, 0.000f, 0.500f, 0.500f, 0.526f, 0.480f, 0.500f },
	{0.000f, 0.500f, 0.700f, 0.160f, 0.000f, 0.158f, 0.349f, 0.000f, 0.280f, 0.900f, 0.000f, 0.618f, 0.500f, 0.401f, 0.000f, 0.500f },
	{0.000f, 0.500f, 0.100f, 0.390f, 0.000f, 0.490f, 0.250f, 0.250f, 0.800f, 0.500f, 0.000f, 0.500f, 0.500f, 0.263f, 0.145f, 0.500f },
	{0.000f, 0.300f, 0.507f, 0.480f, 0.730f, 0.000f, 0.100f, 0.303f, 0.730f, 1.000f, 0.000f, 0.600f, 0.500f, 0.579f, 0.000f, 0.500f },
	{0.000f, 0.300f, 0.500f, 0.320f, 0.000f, 0.467f, 0.079f, 0.158f, 0.500f, 0.500f, 0.000f, 0.400f, 0.500f, 0.151f, 0.020f, 0.500f },
	{0.000f, 0.990f, 0.100f, 0.230f, 0.000f, 0.000f, 0.200f, 0.450f, 0.800f, 0.000f, 0.112f, 0.600f, 0.500f, 0.711f, 0.000f, 0.401f },
	{0.280f, 0.990f, 0.280f, 0.230f, 0.000f, 0.180f, 0.400f, 0.300f, 0.800f, 0.500f, 0.000f, 0.400f, 0.500f, 0.217f, 0.480f, 0.500f },
	{0.220f, 0.990f, 0.250f, 0.170f, 0.000f, 0.240f, 0.310f, 0.257f, 0.900f, 0.757f, 0.000f, 0.500f, 0.500f, 0.697f, 0.803f, 0.500f },
	{0.220f, 0.990f, 0.250f, 0.450f, 0.070f, 0.240f, 0.310f, 0.360f, 0.900f, 0.500f, 0.211f, 0.500f, 0.500f, 0.184f, 0.000f, 0.414f },
	{0.697f, 0.990f, 0.421f, 0.230f, 0.138f, 0.750f, 0.390f, 0.513f, 0.800f, 0.316f, 0.467f, 0.678f, 0.500f, 0.743f, 0.757f, 0.487f },
	{0.000f, 0.400f, 0.000f, 0.280f, 0.125f, 0.474f, 0.250f, 0.100f, 0.500f, 0.500f, 0.000f, 0.400f, 0.500f, 0.579f, 0.592f, 0.500f },
	{0.230f, 0.500f, 0.100f, 0.395f, 0.000f, 0.388f, 0.092f, 0.250f, 0.150f, 0.500f, 0.200f, 0.200f, 0.500f, 0.178f, 0.822f, 0.500f },
	{0.000f, 0.600f, 0.400f, 0.230f, 0.000f, 0.450f, 0.320f, 0.050f, 0.900f, 0.500f, 0.000f, 0.200f, 0.500f, 0.520f, 0.105f, 0.500f },
	{0.000f, 0.600f, 0.400f, 0.170f, 0.145f, 0.290f, 0.350f, 0.100f, 0.900f, 0.500f, 0.000f, 0.400f, 0.500f, 0.441f, 0.309f, 0.500f },
	{0.000f, 0.600f, 0.490f, 0.170f, 0.151f, 0.099f, 0.400f, 0.000f, 0.900f, 0.500f, 0.000f, 0.400f, 0.500f, 0.118f, 0.013f, 0.500f },
	{0.000f, 0.600f, 0.100f, 0.320f, 0.000f, 0.350f, 0.670f, 0.100f, 0.150f, 0.500f, 0.000f, 0.200f, 0.500f, 0.303f, 0.730f, 0.500f },
	{0.300f, 0.500f, 0.400f, 0.280f, 0.000f, 0.180f, 0.540f, 0.000f, 0.700f, 0.500f, 0.000f, 0.400f, 0.500f, 0.296f, 0.033f, 0.500f },
	{0.300f, 0.500f, 0.400f, 0.360f, 0.000f, 0.461f, 0.070f, 0.070f, 0.700f, 0.500f, 0.000f, 0.400f, 0.500f, 0.546f, 0.467f, 0.500f },
	{0.000f, 0.500f, 0.500f, 0.280f, 0.000f, 0.330f, 0.200f, 0.000f, 0.700f, 0.500f, 0.000f, 0.500f, 0.500f, 0.151f, 0.079f, 0.500f },
	{0.000f, 0.500f, 0.000f, 0.000f, 0.240f, 0.580f, 0.630f, 0.000f, 0.000f, 0.500f, 0.000f, 0.600f, 0.500f, 0.816f, 0.243f, 0.500f },
	{0.000f, 0.355f, 0.350f, 0.000f, 0.105f, 0.000f, 0.000f, 0.200f, 0.500f, 0.500f, 0.000f, 0.645f, 0.500f, 1.000f, 0.296f, 0.500f }
};

const char* 
DX10Processor::s_presetNames[kNumPrograms] = 
{
	"Bright E.Piano",
	"Jazz E.Piano",  
	"E.Piano Pad",   
	"Fuzzy E.Piano", 
	"Soft Chimes",   
	"Harpsichord",   
	"Funk Clav",     
	"Sitar",         
	"Chiff Organ",   
	"Tinkle",        
	"Space Pad",     
	"Koto",          
	"Harp",          
	"Jazz Guitar",   
	"Steel Drum",    
	"Log Drum",      
	"Trumpet",       
	"Horn",          
	"Reed 1",        
	"Reed 2",        
	"Violin",        
	"Chunky Bass",   
	"E.Bass",        
	"Clunk Bass",    
	"Thick Bass",    
	"Sine Bass",     
	"Square Bass",   
	"Upright Bass 1",
	"Upright Bass 2",
	"Harmonics",     
	"Scratch",       
	"Syn Tom",
};

ParamDef DX10Processor::s_paramDefs[kNumParams] = 
{
    ParamDef("Attack", nullptr, .15, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kAttack),
    ParamDef("Decay", nullptr, .6, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kDecay),
    ParamDef("Release", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kRelease),
    ParamDef("Coarse", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kRatio, ParamDef::kAutomatable, kCoarse),
    ParamDef("Fine", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kRatio, ParamDef::kAutomatable, kFine),
    ParamDef("Mod Init", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kModInit),
    ParamDef("Mod Dec", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kModDec),
    ParamDef("Mod Sus", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kModSus),
    ParamDef("Mod Rel", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kModRel),
    ParamDef("Mod Vel", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kModVel),
    ParamDef("Vibrato", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kVibrato),
    ParamDef("Octave", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kUInt, ParamDef::kAutomatable, kOctave),
    ParamDef("FineTune", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kCents, ParamDef::kAutomatable, kFineTune),
    ParamDef("WaveForm", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kWaveForm),
    ParamDef("Mod Thru", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kModThru),
    ParamDef("LFO Rate", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kHz, ParamDef::kAutomatable, kLFORate)
};

//-----------------------------------------------------------------------------
DX10Processor::DX10Processor(double sampleRate) :
    BaseProcessor(sampleRate, kNumParams, s_paramDefs)
{
    this->selectPreset(0);
}

//-----------------------------------------------------------------------------
DX10Processor::~DX10Processor()
{
}

void 
DX10Processor::printPresets() 
{ 
    for(int i=0;i<kNumPrograms;i++)
        std::cerr << i << ": " << s_presetNames[i] << "\n";
}

//-----------------------------------------------------------------------------
void
DX10Processor::selectPreset(int k)
{
    for(int i = 0; i < kNumParams; i++)
    {
        m_params[i].value = s_programParams[k][i];

		//initialise...
		for(int i = 0; i<m_synthData.numVoices; i++) 
		{
            VOICE &voice = m_synthData.voice[i];
			memset(&voice, 0, sizeof(VOICE));
			voice.env = 0.0f;
			voice.car = voice.dcar = 0.0f;
			voice.mod0 = voice.mod1 = voice.dmod = 0.0f;
			voice.cdec = 0.99f; //all notes off
		}
		m_synthData.sustain = m_synthData.activevoices = 0;
		lfo0 = dlfo = modwhl = 0.0f;
		lfo1 = pbend = 1.0f;
		volume = 0.0035f;
		K = 0;
        m_paramsDirty = true;
		processParamChanges();
	}
    this->m_currentProgram = k;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void DX10Processor::monoProcessing(
    float *in, float *out, int sampleFrames)
{
	int frame=0, framesToNextEvent, v;
	float o, x, e, mw = MW, w = rich, m = modmix;
	int k = K;

    //detect & bypass completely empty blocks
    int eventPos = 0;
	if(m_synthData.activevoices>0 || this->hasEvents()) 
	{
		while(frame<sampleFrames)  
		{
            // frame 'til next event (when empty frames will be >> sampleFrames)
            if(eventPos < m_eventPos)
            {
                framesToNextEvent = m_events[eventPos].sampleOffset; 
                if(framesToNextEvent>sampleFrames) 
                    framesToNextEvent = sampleFrames;
                if(framesToNextEvent == 0) //next note on/off
                    this->processEvents(eventPos, frame);
            }
            else
                framesToNextEvent = sampleFrames;

            // update active voices before potentially activating more
            // would be faster with voice loop outside frame loop!
            // but then each voice would need it's own LFO...
			while (framesToNextEvent>=0) 
            {
				VOICE *V = m_synthData.voice.data();
				o = 0.0f;
				if(--k<0)
				{
					lfo0 += dlfo * lfo1; //sine LFO
					lfo1 -= dlfo * lfo0;
					mw = lfo1 * (modwhl + vibrato);
					k = 100;
				}
				for(v=0; v<m_synthData.numVoices; v++) //for each voice
				{
					e = V->env;
					if (e > SILENCE) //**** this is the synth ****
					{
						V->env = e * V->cdec; //decay & release
						V->cenv += V->catt * (e - V->cenv); //attack

						x = V->dmod * V->mod0 - V->mod1; //could add more modulator blocks like
						V->mod1 = V->mod0;               //this for a wider range of FM sounds
						V->mod0 = x;    
						V->menv += V->mdec * (V->mlev - V->menv);

						x = V->car + V->dcar + x * V->menv + mw; //carrier phase
						while (x >  1.0f) x -= 2.0f;  //wrap phase
						while (x < -1.0f) x += 2.0f;
						V->car = x;
						o += V->cenv * (m * V->mod1 + (x + x * x * x * (w * x * x - 1.0f - w))); 
					}      //amp env //mod thru-mix //5th-order sine approximation

					///  xx = x * x;
					///  x + x + x * xx * (xx - 3.0f);
					V++;
				}
				*out++ = o;
                framesToNextEvent--;
                frame++;
			} // while framesToNextEvent
		} // while frame < sampleFrames

		m_synthData.activevoices = m_synthData.numVoices;
		for(v=0; v<m_synthData.numVoices; v++)
		{
			if(m_synthData.voice[v].env < SILENCE)  //choke voices that have finished
			{
				m_synthData.voice[v].env = m_synthData.voice[v].cenv = 0.0f;
				m_synthData.activevoices--;
			}
			if(m_synthData.voice[v].menv < SILENCE) 
                m_synthData.voice[v].menv = m_synthData.voice[v].mlev = 0.0f;
		}
	}
	else //completely empty block
	{
		while (--sampleFrames >= 0)
			*out++ = 0.0f;
	}
	K=k; 
    MW=mw; //remember these so vibrato speed not buffer size dependant!
}

//-----------------------------------------------------------------------------
void 
DX10Processor::processEvents(int &eventPos, int frame)
{
    while(eventPos < m_eventPos)
    {
        MidiEvent &e = m_events[eventPos];
        if(e.sampleOffset > frame)
            break;
        else
        {
            this->processEvent(e);
            eventPos++;
        }
    }
}

void 
DX10Processor::processEvent(MidiEvent& event)
{
	float l = 1.0f;
	int v, vl=0;

	if(event.IsNoteOn())
	{
		unsigned pitch = event.GetNote(); // 0 -> 127
		float velocity = event.GetVelocity();
		for(v=0; v<m_synthData.numVoices; v++)  //find quietest voice
		{
			if(m_synthData.voice[v].env<l) 
            { 
                l = m_synthData.voice[v].env;  
                vl = v; 
            }
		}

		l = expf(0.05776226505f * 
                (pitch + m_params[kFineTune].value + m_params[kFineTune].value - 1.0f));
		m_synthData.voice[vl].note = pitch;
		m_synthData.voice[vl].car  = 0.0f;
		m_synthData.voice[vl].dcar = tune * pbend * l; //pitch bend not updated during note as a bit tricky...

		if (l>50.0f) l = 50.0f; //key tracking
		l *= (64.0f + velsens * (velocity - 64)); //vel sens
		m_synthData.voice[vl].menv = depth * l;
		m_synthData.voice[vl].mlev = dept2 * l;
		m_synthData.voice[vl].mdec = mdec;

		m_synthData.voice[vl].dmod = ratio * m_synthData.voice[vl].dcar; //sine oscillator
		m_synthData.voice[vl].mod0 = 0.0f;
		m_synthData.voice[vl].mod1 = sinf(m_synthData.voice[vl].dmod); 
		m_synthData.voice[vl].dmod = 2.0f * cosf(m_synthData.voice[vl].dmod);
		//scale volume with richness
		m_synthData.voice[vl].env  = (1.5f - m_params[kWaveForm].value) * volume * (velocity + 10);
		m_synthData.voice[vl].catt = catt;
		m_synthData.voice[vl].cenv = 0.0f;
		m_synthData.voice[vl].cdec = cdec;
	}
	else //note off
    if(event.IsNoteOff())
	{
		float pitch = event.GetNote();
		for(v=0; v<m_synthData.numVoices; v++) 
        {
            if(m_synthData.voice[v].note==pitch) //any voices playing that note?
            {
                if(m_synthData.sustain==0)
                {
                    m_synthData.voice[v].cdec = crel; //release phase
                    m_synthData.voice[v].env  = m_synthData.voice[v].cenv;
                    m_synthData.voice[v].catt = 1.0f;
                    m_synthData.voice[v].mlev = 0.0f;
                    m_synthData.voice[v].mdec = mrel;
                }
                else 
                    m_synthData.voice[v].note = kInvalidNote;
            }
        }
	}
    else
    if(event.IsPitchWheel())
    {
        // pbend is currently only accessed during note-down (XXX)
        // and doesn't trigger a paramchange
        this->pbend = event.PitchWheelVal();
        if(pbend>0.0f) 
            this->pbend = 1.0f + 0.000014951f * this->pbend; 
        else 
            this->pbend = 1.0f + 0.000013318f * this->pbend; 
    }
    else
    if(event.IsModWheel())
    {
        // modwheel works for all live notes, and doesn't trigger 
        // a paramchange.
        int newValue = event.ModWheelVal(); // [0-127]
        this->modwhl = 0.00000005f * (newValue * newValue);
    }
}

//-----------------------------------------------------------------------------
// input params:
//      kOctave, kCoarse, kFine, kModInit
// output params:
//    tune, rati, ratf, ratio, depth, depht2, velsens, vibrato
void 
DX10Processor::processParamChanges()
{
    if(!this->m_paramsDirty) return;

	float ifs = this->m_samplePeriod;

	tune = (float)(8.175798915644 * ifs * pow(2.0, floor(m_params[kOctave].value * 6.9) - 2.0));

	rati = m_params[kCoarse].value;
	rati = (float)floor(40.1f * rati * rati);
	if(m_params[kFine].value<0.5f) 
		ratf = 0.2f * m_params[kFine].value * m_params[kFine].value;
	else 
    switch ((int)(8.9f * m_params[kFine].value))
    {
        case  4: ratf = 0.25f;       break;
        case  5: ratf = 0.33333333f; break;
        case  6: ratf = 0.50f;       break;
        case  7: ratf = 0.66666667f; break;
        default: ratf = 0.75f;
    }
	ratio = 1.570796326795f * (rati + ratf);

	depth = 0.0002f * m_params[kModInit].value * m_params[kModInit].value;
	dept2 = 0.0002f * m_params[kModSus].value * m_params[kModSus].value;

	velsens = m_params[kModVel].value;
	vibrato = 0.001f * m_params[kVibrato].value * m_params[kVibrato].value;

	catt = 1.0f - (float)exp (-ifs * exp (8.0 - 8.0 * m_params[kAttack].value));
	if(m_params[kDecay].value>0.98f) 
        cdec = 1.0f; 
    else 
        cdec = expf(-ifs * expf(5.f - 8.f * m_params[kDecay].value));
	crel =        expf(-ifs * expf(5.f - 5.f * m_params[kRelease].value));
	mdec = 1.0f - expf(-ifs * expf(6.f - 7.f * m_params[kModDec].value));
	mrel = 1.0f - expf(-ifs * expf(5.f - 8.f * m_params[kModRel].value));

	rich = 0.50f - 3.0f * m_params[kWaveForm].value * m_params[kWaveForm].value;
	//rich = -1.0f + 2 * params[kWaveForm];
	modmix = 0.25f * m_params[kModThru].value * m_params[kModThru].value;

    //these params not in original DX10
	dlfo = 628.3f * ifs * 25.0f 
            * m_params[kLFORate].value * m_params[kLFORate].value; 

    m_paramsDirty = false;
}
