#ifndef dbTrigger_h
#define dbTrigger_h

#include <chuck_def.h> // defines SAMPLE, usually float
#include "dbRand.h"

class dbTrigger
{
public:
    dbTrigger()
    {
        this->counter = 0;
        this->Set(4410, 0); // 10 triggers per second at 44KHz
        this->lastSamp = 0.f;
    }

    void SetPeriod(long period) // measured in ticks
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

        // counter initially 0 and we want to fire then
        if(this->counter++ == 0)
        {
            this->updateEndPt(); // in case of randomness we re-roll
            return true;
        }
        else
        {
            if(this->counter >= this->cycleEnd)
                this->counter = 0;
            return false;
        }
    }

private:
    long counter; // through current cycle
    long cycleEnd; // for current "cycle", changes randomly

    long period;
    float range; // non-zero for "dust" (periodpct)
    SAMPLE lastSamp;
};

#endif