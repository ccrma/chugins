#ifndef DbLiCKLFO_h
#define DbLiCKLFO_h

// This LFO chugin was inspired by LiCK's lfo Chugens,
// here: // https://github.com/hauermh/lick/lfo .
// Ported (with enhancements) by Dana Batali.
// Subject to GPL3 license.

#include "dbOSC.h"
#include "dbOnePole.h"
#include "dbNoise.h"
#include <cassert>

#define sDebug 0

class DbLiCKLFO
{
public:
    DbLiCKLFO(float srate) :
        sampleRate(srate),
        osc(srate),
        modulateInput(false)
    {
        this->Sine();
        this->SetOutputRange(-1.f, 1.f);
    }
    ~DbLiCKLFO() {}

    void SetFreq(float freq)
    {
        this->osc.SetFreq(freq);
    }

    void SetOutputRange(float min, float max)
    {
        this->outMin = min;
        this->outMax = max;
        this->deltaOut = max - min;
    }

    void Mix(float wSaw, float wSine, float wSqr, float wTri,
            float wHyper, float wSh, float wSsh, float wFNoise)
    {
        float total = wSaw + wSine + wSqr + wTri 
                      + wHyper + wSh + wSsh + wFNoise;
        this->sawMix = wSaw / total;
        this->sineMix = wSine / total;
        this->sqrMix = wSqr / total;
        this->triMix = wTri / total;
        this->hyperMix = wHyper / total;
        this->sampleHoldMix = wSh / total;
        this->smoothSampleHoldMix = wSsh / total;
        this->filteredNoiseMix = wFNoise / total;

        if(sDebug)
        {
            printf("mix %g %g %g %g %g %g %g %g\n",
                this->sawMix, this->sineMix, this->sqrMix,
                this->triMix, this->hyperMix, this->sampleHoldMix,
                this->smoothSampleHoldMix, this->filteredNoiseMix);
        }
            
    }

    void Saw()
    {
        this->Mix(1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
    }

    void Phasor()
    {
        this->Mix(-1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
    }

    void Sine()
    {
        this->Mix(0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
    }

    void Sqr()
    {
        this->Mix(0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f);
    }

    void Tri()
    {
        this->Mix(0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f);
    }

    void Hyper()
    {
        this->Mix(0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f);
    }

    void SampleHold()
    {
        this->Mix(0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f);
    }

    void SmoothSampleHold()
    {
        this->Mix(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
    }

    void FilteredNoise()
    {
        this->Mix(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f);
    } 

    void Modulate(int mod)
    {
        this->modulateInput = mod ? true : false;
    }

    float Phase(float phase)
    {
        return this->osc.SetPhase(phase);
    }

    void PhaseWobble(float phaseWobble)
    {
        this->osc.SetPhaseWobble(phaseWobble);
    }

    void PhaseWobbleFreq(float phaseWobbleFreq)
    {
        this->osc.SetPhaseWobbleFreq(phaseWobbleFreq);
    }

    // we support negative gain to eg convert saw to phasor
    float Eval(float phase, float in=0.f)
    {
        float out = 0.f;
        if(this->sawMix != 0.f)
            out += this->sawMix * this->osc.Saw(phase);
        if(this->sineMix != 0.f)
            out += this->sineMix * this->osc.Sine(phase);
        if(this->sqrMix != 0.f)
            out += this->sqrMix * this->osc.Sqr(phase);
        if(this->triMix != 0.f)
            out += this->triMix * this->osc.Tri(phase);
        if(this->hyperMix != 0.f)
            out += this->hyperMix * this->osc.Hyper(phase);
        if(this->sampleHoldMix != 0.f)
            out += this->sampleHoldMix * this->sampleHold(in);
        if(this->smoothSampleHoldMix != 0.f)
            out += this->smoothSampleHoldMix * this->smoothSampleHold(in);
        if(this->filteredNoiseMix != 0.f)
        {
            out += this->filteredNoiseMix * 
                    this->onePole.tick(this->noise.tick());
        }
        // remap
        float lfo = this->outMin + this->deltaOut * (.5f * (out+1.f));
        return this->modulateInput ? lfo*in : lfo;
    }

    float Tick(float in)
    {
        float phase = this->osc.GetPhase();
        float out = this->Eval(phase, in);
        this->osc.IncPhase();
        return out;
    }

    // durations converted to samples by chuck...
    void SetHold(float hold)
    {
        this->shCtx.holdAmount = (unsigned) hold;
    }

    void SetSmoothHold(float hold, float interphold)
    {
        this->sshCtx.holdAmount = (unsigned) hold;
        this->sshCtx.smoothHoldAmount = (unsigned) interphold;
    }

    void SetNoiseHold(float hold)
    {
        this->noise.SetRepeats(hold);
    }

    float sampleHold(float in)
    {
        if(this->shCtx.holdCounter == 0)
        {
            this->shCtx.lastHeld = in;
            this->shCtx.holdCounter = this->shCtx.holdAmount;
            return in;
        }
        else
        {
            this->shCtx.holdCounter--;
            return this->shCtx.lastHeld;
        }
    }

    float smoothSampleHold(float in)
    {
        if(this->sshCtx.holdCounter == 0)
        {
            this->sshCtx.lastHeld = this->sshCtx.nextHeld;
            this->sshCtx.nextHeld = in;
            this->sshCtx.holdCounter = this->sshCtx.holdAmount;
            this->sshCtx.smoothHoldCounter = this->sshCtx.smoothHoldAmount;
            this->sshCtx.delta = (this->sshCtx.nextHeld - this->sshCtx.lastHeld) 
                                    / (this->sshCtx.smoothHoldAmount-1);
            return this->sshCtx.lastHeld;
        }
        else
        {
            this->sshCtx.holdCounter--;
            if(this->sshCtx.smoothHoldCounter > 0)
            {
                this->sshCtx.smoothHoldCounter--;
                this->sshCtx.lastHeld += this->sshCtx.delta;
                if(this->sshCtx.smoothHoldCounter == 0)
                {
                    /*
                    printf("done smoothing %g %g\n", 
                        this->sshCtx.lastHeld, this->sshCtx.nextHeld);
                    */
                }
            }
            return this->sshCtx.lastHeld;
        }
    }

private:
    float sampleRate;
    float outMin, outMax, deltaOut;

    float sawMix;
    float sineMix;
    float sqrMix;
    float triMix;
    float hyperMix;
    float sampleHoldMix;
    float smoothSampleHoldMix;
    float filteredNoiseMix;

    struct
    {
        unsigned holdCounter=0; 
        unsigned holdAmount=100000; 
        float lastHeld = 0.f;
    } shCtx;

    struct
    {
        unsigned holdCounter=0; 
        unsigned holdAmount=100000; 
        unsigned smoothHoldCounter=0; 
        unsigned smoothHoldAmount = 10000;
        float lastHeld = 0.f, nextHeld = 0.f, delta;
    } sshCtx;

    DbOnePole onePole;
    DbNoise noise;
    DbOSC osc;
    bool modulateInput;
};

#endif