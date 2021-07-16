#ifndef dbGrainBuf_h
#define dbGrainBuf_h

/**
 * Our job is to create and combine a number of in-flight grains into 
 * a single output sample. All grains index into the one sndbufState.
 * Primary entrypoint/class is dbGrainBuf (at bottom).
 */

#include "dbGrainMgr.h"
#include "dbSndBuf.h"
#include "dbPhasor.h"
#include "dbTrigger.h"
#include "dbRand.h"

#include <string>

/* ----------------------------------------------------------------------- */
/**
 * This is the main context for grain buf...
 */
class dbGrainBuf
{
public:
    dbGrainBuf(float sampleRate) :
        sampleRate(sampleRate),
        sndbuf(sampleRate),
        grainMgr(512),
        phasor(sampleRate),
        bypassGrains(false),
        windowFilter(dbWindowing::kBlackman),
        grainPeriod(.2f * sampleRate), // 200 ms
        currentGrainPeriod(.2f * sampleRate), // 200 ms
        grainPeriodVariance(0.f),
        grainPeriodVarianceFreq(1.f),
        grainRate(1.0f),
        totalTicks(0),
        nextGrainPeriodRand(0),
        debug(0)
    {
    }
    ~dbGrainBuf() {}

    int Debug(int debug)
    {
        this->debug = debug;
        this->phasor.Debug(debug);
        return debug;
    }

    int Read(std::string &filename)
    {
        int err = this->sndbuf.ReadHeader(filename);
        if(err == 0)
            this->phasor.SetFileDur(this->sndbuf.GetLengthInSeconds());
        return err;
    }

    int GrainWindow(std::string &filtername)
    {
        int err = 0;
        if(filtername == "blackman")
            this->windowFilter = dbWindowing::kBlackman;
        else
        if(filtername == "hanning")
            this->windowFilter = dbWindowing::kHanning;
        else
        if(filtername == "hamming")
            this->windowFilter = dbWindowing::kHamming;
        else
        if(filtername == "bartlett")
            this->windowFilter = dbWindowing::kBartlett;
        else
        if(filtername == "plancktaper85" ||
           filtername == "plancktaper")
            this->windowFilter = dbWindowing::kPlanckTaper85;
        else
        if(filtername == "plancktaper95")
            this->windowFilter = dbWindowing::kPlanckTaper95;
        else
        {
            std::cout << "Unknown windowing filter:" << filtername << std::endl;
            err = 1;
        }
        return err;
    }

    double GetFileDur() // double gets us sample accuracy
    {
        return this->sndbuf.GetLengthInSeconds();
    }

    int GetNChan()
    {
        return this->sndbuf.GetNChan();
    }

    SAMPLE Tick(SAMPLE in)
    {
        if(!this->bypassGrains)
        {
            // Trigger and Duration conspire to characterize the number
            // of active Grains at a given time.  Faster triggers (say 100hz)
            // with longer durations (say 10sec) would require more live grains
            // than we can afford (~1000). (Supercollider default max is 512).
            this->totalTicks++;
            this->phasor.Tick(this->totalTicks);
            if(this->trigger.SampleAndTick(in))
            {
                Grain *g = this->grainMgr.Allocate();
                if(g)
                {
                    // upon Grain 'creation', we sample the parameter 
                    // generators and then initialize the grain: 
                    // NB: we'd like to support CC for each of these so users
                    // can wire-up arbitrary behavior.
                    //    dur: within a range 
                    //    pos: 
                    //          constant, 
                    //          sliding range with randomness (looping implicitly)
                    //          random locations
                    //    rate:
                    // 
                    long startPos = (long) this->phasor.Sample();
                     // end conditions handled by grain
                    long stopPos = this->getGrainStop(startPos);
                    g->Init(startPos, stopPos, this->grainRate, 
                            this->windowFilter);
                    if(debug)
                    {
                        std::cout << "New grain " <<
                            startPos << "->" << stopPos <<
                            ", rate:" << this->grainRate << " (" <<
                            this->grainMgr.GetActiveGrainCount() << "/" <<
                            this->grainMgr.GrainPoolSize() << ")" <<
                            std::endl;
                    }
                }
                else
                {
                    std::cout << "DbGrainBuf: too many active grains." << std::endl;
                }
            }

            SAMPLE sum = 0;
            for(const auto& g: this->grainMgr.ActiveGrains) 
            {
                sum += g->SampleAndTick(this->sndbuf);
            }
            this->grainMgr.Prune();
            return sum;
        }
        else
            return this->sndbuf.Sample();
    }

    /* SndBuf interface-ish ---- */
    int SetBypass(int b)
    {
        this->bypassGrains = b;
        return b;
    }
    int GetBypass() { return this->bypassGrains; }

    /* Grainbuf parameters ------------------------------------------------- */
    float SetTriggerFreq(float freq)
    {
        long ticks = this->sampleRate / freq;
        this->trigger.SetPeriod(ticks);
        return freq;
    }

    float GetTriggerFreq()
    {
        return this->sampleRate / this->trigger.GetPeriod();
    }

    float SetTriggerRange(float pct) // value depends on  value of trigger rate
    {
        this->trigger.SetRange(pct);
        return pct;
    }

    float SetGrainRate(float factor)
    {
        this->grainRate = factor;
        return factor;
    }

    long SetGrainPeriod(long period) // measured in samples
    {
        this->grainPeriod = period;
        this->currentGrainPeriod = period;
        return period;
    }

    long GetGrainPeriod()
    {
        return this->grainPeriod;
    }

    float SetGrainPeriodVariance(float pct)
    {
        this->grainPeriodVariance = pct;
        return pct;
    }

    float SetGrainPeriodVarianceFreq(float hz)
    {
        this->grainPeriodVarianceFreq = hz;
        return hz;
    }

    float SetGrainPhaseStart(float startPhase)
    {
        this->phasor.SetStart(startPhase);
        return startPhase;
    }

    float SetGrainPhaseStop(float stopPhase)
    {
        this->phasor.SetStop(stopPhase);
        return stopPhase;
    }

    float SetGrainPhaseStartSec(float startPhaseSec)
    {
        float startPhase = startPhaseSec/this->GetFileDur();
        this->phasor.SetStart(startPhase);
        return startPhase;
    }

    float SetGrainPhaseStopSec(float stopPhaseSec)
    {
        float stopPhase = stopPhaseSec/this->GetFileDur();
        this->phasor.SetStop(stopPhase);
        return stopPhase;
    }

    float SetGrainPhaseRate(float phaseRate)
    {
        this->phasor.SetRate(phaseRate);
        return phaseRate;
    }

    float SetGrainPhaseWobble(float phaseWobble)
    {
        this->phasor.SetWobble(phaseWobble);
        return phaseWobble;
    }

    float SetGrainPhaseWobbleFreq(float wfreq)
    {
        this->phasor.SetWobbleFreq(wfreq);
        return wfreq;
    }

    /* Bypass parameters --------------------------------------------- */
    int SetLoop(int loop)
    {
        this->sndbuf.SetLoop(loop);
        return loop;
    }
    int GetLoop() { return this->sndbuf.GetLoop(); }

    int
    SetPos(int pos)
    {
        this->sndbuf.SetPosition(pos);
        return pos;
    }
    int GetPos() { return this->sndbuf.GetPosition(); }

    float
    SetPhase(float phase)
    {
        this->sndbuf.SetPhase(phase);
        return phase;
    }
    float GetPhase() { return this->sndbuf.GetPhase(); }

    float SetRate(float rate)
    {
        this->sndbuf.SetRate(rate);
        return rate;
    }
    float GetRate() { return this->sndbuf.GetRate(); }

    int SetMaxFilt(int w) { return this->sndbuf.SetMaxFilt(w); }
    int GetMaxFilt() { return this->sndbuf.GetMaxFilt(); }

private:
    long getGrainStop(long start)
    {
        long stop = start + this->currentGrainPeriod;
        if(this->grainPeriodVariance != 0)
        {
            if(this->totalTicks > this->nextGrainPeriodRand)
            {
                this->nextGrainPeriodRand = this->totalTicks  +
                    this->grainPeriodVarianceFreq * this->sampleRate;
                this->currentGrainPeriod = this->grainPeriod +
                    rand32HalfRange(this->grainPeriod*this->grainPeriodVariance);
                if(this->debug)
                    std::cout << "grain period:" << this->currentGrainPeriod << std::endl;
            }
        }
        return stop;
    }

private:
    float sampleRate;
    dbSndBuf sndbuf;
    dbGrainMgr grainMgr;
    dbTrigger trigger;
    dbPhasor phasor;
    bool bypassGrains; // and use sndbuf directly
    dbWindowing::FilterType windowFilter;

    float grainRate; // fractional samplesteps/sample
    long grainPeriod; // measured in samples
    long currentGrainPeriod; // == grainPeriod unless grainPeriodVariance > 0
    float grainPeriodVariance; // pct of period
    float grainPeriodVarianceFreq; // Hz

    bool debug;
    long totalTicks;
    long nextGrainPeriodRand;
};

#endif