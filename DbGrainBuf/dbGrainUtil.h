#ifndef dbGrainUtil_h
#define dbGrainUtil_h

#include <chuck_def.h> // defines SAMPLE, usually float
#include <forward_list>
#include <vector>

#include "dbRand.h"
#include "dbSndBuf.h"
#include "dbWindowing.h"

/* support classes/structs for dbGrainBuf.h --- */

/* ----------------------------------------------------------------------- */
struct Grain
{
    Grain()
    {
        this->active = 0;
        this->chan = 0;
    }

    void Init(long startPos, long stopPos, float rate)
    {
        this->active = 1;
        this->pos = startPos;
        this->start = startPos;
        this->stop = stopPos;
        this->rate = rate;
        this->filterRadius = 1 + std::abs(static_cast<int>(.5*rate));
        if(this->filterRadius > 5)
            this->filterRadius = 5;
        this->winPct = 0.f;
        this->winStep = std::abs(rate) / (stopPos - startPos + 1);
        this->window = dbWindowing::Get(); // we don't own this

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
    dbWindowing *window; // we don't own this
};

/* ----------------------------------------------------------------------- */
class dbGrainMgr
{
public:
    dbGrainMgr(unsigned maxGrains)
    {
        this->grainPool.resize(maxGrains);
    };

    ~dbGrainMgr() {};

    void Prune()
    {
        this->ActiveGrains.remove_if(this->pruner);
    }

    Grain *Allocate()
    {
        for(auto& value: this->grainPool) 
        {
            if(!value.active)
            {
                value.active = true;
                this->ActiveGrains.push_front(&value);
                return &value;
            }
        }
        return nullptr;
    }

    void Release(Grain *g)
    {
        if(g) 
        {
            this->ActiveGrains.remove(g);
            g->active = false;
        }
    }

    std::forward_list<Grain*> ActiveGrains;

private:
    std::vector<Grain> grainPool;
    class isInactive
    {
    public:
        bool operator() (Grain * g) {return !g->active;}
    } pruner;
};

/* ----------------------------------------------------------------------- */
class dbTrigger
{
public:
    dbTrigger()
    {
        this->counter = 0;
        this->Set(4410, 0); // 10 triggers per second at 44KHz
        this->lastSamp = 0.f;
    }

    void SetPeriod(long period) // measured in ticksa
    {
        this->period = period;
        this->updateEndPt();
    }

    void SetRange(float range) // measured as a pct of period
    {
        this->range = range;
        this->updateEndPt();
    }

    void Set(long period, float range)
    {
        this->period = period;
        this->range = range;
        this->updateEndPt();
    }

    void updateEndPt()
    {
        if(this->range == 0.f)
            this->cycleEnd = this->period;
        else
        {
            uint32_t prange = rand32Range(this->period * this->range);
            this->cycleEnd = this->period + prange;
        }
    }

    /* Tick returns true when the trigger activates
     */
    bool SampleAndTick(SAMPLE s)
    {
        if(s > 0 && this->lastSamp == 0) // external trigger trumps
        {
            this->lastSamp = s;
            return true;
        }

        this->counter++;
        if(this->counter < this->cycleEnd)
            return false;
        else
        {
            this->counter = 0;
            this->updateEndPt();
            return true;
        }
    }

private:
    long counter; // through current cycle
    long cycleEnd; // for current "cycle", changes randomly

    long period;
    float range; // non-zero for "dust" (periodpct)
    SAMPLE lastSamp;
};

/* ----------------------------------------------------------------------- */
/* Phasor produces values between [0, 1] representing the 
 * sndbuf location pointer. When rate is 1, the expectation 
 * is that, when sampled, Phasor returns a phase that matches
 * the natural phase for the sndbuf file (which is a function 
 * of its duration).  For this reason Phasor can't operate without
 * knowledge of the current file duration.
 */
class dbPhasor
{
public:
    dbPhasor(float sr) :
        sampleRate(sr),

        fileTicks(0), // only known after file is provided
        pos(0.),
        startPos(0.),
        stopPos(0.),
        deltaPos(0.),

        start(0.f), // pct of file
        stop(1.f),  // pct of file
        rate(1.f),
        wobble(0.f),
        wobbleFreq(0.f)
    {}
    ~dbPhasor() {};

    // we want phasor to go from min to max in seconds (*rate)
    // samples/sec * sec = samples (in song/phasor), so phaseStep is range / samples
    void SetFileDur(double secs)
    {
        this->fileTicks = secs * this->sampleRate;
        this->updatePos();
    }

    void Tick() 
    {
        this->pos += this->deltaPos;
        if(this->pos > this->stopPos)
        {
            std::cout << "Phasor reset " << 
                this->pos << " " << this->stopPos << std::endl;
            this->pos = this->start + (this->pos - this->stop);
        }
        else
        if(this->pos < this->startPos)
        {
            this->pos = this->stop - (this->start - this->pos);
        }
        // XXX: apply wobble
    }

    /**
     @returns the current pos value as a sample index. Double cuz
       we can return a inter-sample location. Also: we need more than
       23 bits of file-offset that float provides.
     */
    double Sample()
    {
        return this->pos;
    }

    void SetStart(float start)
    {
        this->start = start;
        this->updatePos();
    }

    void SetStop(float stop)
    {
        this->stop = stop;
        this->updatePos();
    }

    void SetRate(float rate)
    {
        this->rate = rate;
        this->updatePos();
    }

    void SetWobble(float wobble)
    {
        this->wobble = wobble;
    }

private:

    void updatePos()
    {
        this->startPos = this->start * this->fileTicks;
        this->stopPos = this->stop * this->fileTicks;
        this->pos = this->startPos;
        this->deltaPos = this->rate;
    }

    float sampleRate; // samples/sec (chuck sample rate)

    // we maintain state in chuck samplecoords relative to file
    long fileTicks; 
    double pos;
    double startPos;
    double stopPos;
    double deltaPos; // need to handle negative delta

    // user expresses phase as pct of file. 
    // we convert to ticks when we know the file size.
    float start; 
    float stop;
    float rate;
    float wobble;
    float wobbleFreq; 
};

#endif