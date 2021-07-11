#ifndef dbGraphBuf_h
#define dbGraphBuf_h
/**
 * Our job is to create and combine a number of in-flight grains into 
 * a single output sample. All grains index into the one sndbufState.
 * sndbufState lazily loads chunks to minimize startup costs.
 */

#include "dbSndBuf.h"
#include <string>
#include <vector>
#include <forward_list>
#include <cstdlib> // rand

struct Grain
{
    Grain()
    {
        this->active = 0;
    }

    void InitDur()
    {
    }

    int chan;
    float rate; // aka sample increment
    int pos; 
    int start;
    int stop;
    SAMPLE samp;
    float weight; //windowing[pos/start->stop] 
    bool active;
};

class dbGrainMgr
{
public:
    dbGrainMgr(unsigned maxGrains)
    {
        this->m_grainPool.resize(maxGrains);
    };

    ~dbGrainMgr() {};

    Grain *Activate()
    {
        for(auto& value: this->m_grainPool) 
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
    std::vector<Grain> m_grainPool;
};

class dbTrigger
{
public:
    dbTrigger()
    {
        this->counter = 0;
        this->SetTriggerRange(4410, 0); // 10 triggers per second at 44KHz
    }

    void SetTriggerMin(long ticks)
    {
        this->minTicks = ticks;
        this->ticksPerTrigger = ticks; // in case range is 0
    }

    void SetTriggerRange(long range)
    {
        this->randomTickRange = range;
        this->update();
    }

    void SetTriggerRange(long minticks, long range)
    {
        this->minTicks = minticks;
        this->ticksPerTrigger = minticks; // in case range is 0
        this->randomTickRange = range;
        this->update();
    }

    void update()
    {
        if(this->randomTickRange)
            this->ticksPerTrigger = this->minTicks + (rand() % this->randomTickRange);
    }

    bool IsTriggered()
    {
        this->counter++;
        if(this->counter < this->ticksPerTrigger)
            return false;
        else
        {
            this->counter = 0;
            this->update();
            return true;
        }
    }

private:
    long counter;
    long ticksPerTrigger; // for current "cycle"
    long minTicks;
    long randomTickRange; // non-zero for "dust"
};

class dbGrainBuf
{
public:
    dbGrainBuf(float sampleRate);
    ~dbGrainBuf();

    SAMPLE Tick(SAMPLE in);
    int Read(std::string &filename);

    /* SndBuf interface-ish ---- */
    int SetBypass(int b)
    {
        this->m_bypassGrains = b;
        return b;
    }
    int GetBypass() { return this->m_bypassGrains; }

    int SetLoop(int loop)
    {
        this->m_sndbuf.SetLoop(loop);
        return loop;
    }
    int GetLoop() { return this->m_sndbuf.GetLoop(); }

    int
    SetPos(int pos)
    {
        this->m_pos = pos;
        this->m_phase = -666.;
        this->m_sndbuf.SetPosition(pos);
        return pos;
    }
    int GetPos() { return this->m_sndbuf.GetPosition(); }

    float
    SetPhase(float phase)
    {
        this->m_phase = phase;
        this->m_pos = -666;
        this->m_sndbuf.SetPhase(phase);
        return phase;
    }
    float GetPhase() { return this->m_sndbuf.GetPhase(); }

    float SetRate(float rate)
    {
        this->m_sndbuf.SetRate(rate);
        return rate;
    }
    float GetRate() { return this->m_sndbuf.GetRate(); }

    int SetMaxFilt(int w) { return this->m_sndbuf.SetMaxFilt(w); }
    int GetMaxFilt() { return this->m_sndbuf.GetMaxFilt(); }

    float SetTriggerRate(float seconds)
    {
        long ticks = this->m_sampleRate * seconds;
        this->m_trigger.SetTriggerMin(ticks);
        return seconds;
    }

private:
    float m_sampleRate;
    dbSndBuf m_sndbuf;
    dbGrainMgr m_grainMgr;
    dbTrigger m_trigger;
    bool m_bypassGrains;

    float m_phase;  // -666 if unset
    int m_pos;      // -666 if unset

    int m_minDur, m_maxDur;
    int m_minPos, m_maxPos;

    void applyWindow(float * data, float * window, 
        unsigned long length);
    void hanning(float *w, unsigned long len);
    void hamming(float *w, unsigned long len);
    void blackman(float *w, unsigned long len);
    void bartlett(float *w, unsigned long len);

};

#endif