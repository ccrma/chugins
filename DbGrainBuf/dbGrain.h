#ifndef dbGrain_h
#define dbGrain_h

#include <chuck_def.h> // defines SAMPLE, usually float

#include "dbSndBuf.h"
#include "dbWindowing.h"

struct Grain
{
    Grain()
    {
        this->active = 0;
        this->chan = 0;
    }

    void Init(long startPos, long stopPos, float rate, 
            dbWindowing::FilterType t, float pan, int chan=0)
    {
        this->active = 1;
        this->chan = chan;
        this->pos = startPos;
        this->start = startPos;
        this->stop = stopPos;
        this->rate = rate;
        this->filterRadius = 1 + std::abs(static_cast<int>(.5*rate));
        if(this->filterRadius > 5)
            this->filterRadius = 5;
        this->winPct = 0.f;
        this->winStep = std::abs(rate) / (stopPos - startPos + 1);
        this->window = dbWindowing::Get(t); // we don't own this

        if(pan == 0.f)
        {
            this->panLeft = .7071f;
            this->panRight = .7071f;
        }
        else
        {
            // remap pan from [-1,1] to [0,pi/2]
            float panme = (pan+1.0)/2.f * ONE_PI/2.f;
            // pan it (NEW: constant-power panning; fixed 1.4.1.0)
            this->panLeft = cos(panme);
            this->panRight = sin(panme);
        }

        // std::cout << "New Grain at " << this->pos << std::endl;
    }

    SAMPLE SampleAndTick(dbSndBuf &sndbuf)
    {
        // update 
        //  weight, pos, active
        float w = this->window->Sample(this->winPct);
        SAMPLE samp = w*sndbuf.Sample(this->chan, this->pos, this->filterRadius);
        this->pos += this->rate;
        if(this->pos <= this->stop)
            this->winPct += this->winStep;
        else
            this->active = 0;
        return samp;
    }

    bool active;
    float rate; // aka sample increment
    double pos; 
    long start;
    long stop;
    float winPct;
    float winStep;
    int filterRadius;
    int chan;
    float panLeft;
    float panRight;
    dbWindowing *window; // we don't own this
};

#endif