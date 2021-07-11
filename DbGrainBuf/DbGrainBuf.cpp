#include "dbGrainBuf.h"
#include <math.h>

dbGrainBuf::dbGrainBuf(float sampleRate) :
    m_sampleRate(sampleRate),
    m_sndbuf(sampleRate),
    m_grainMgr(512),
    m_bypassGrains(false),
    m_pos(-666), // signals that user hasn't asserted anything
    m_phase(-666) // signals that user hasn't asserted anything
{
}

dbGrainBuf::~dbGrainBuf()
{
}

int 
dbGrainBuf::Read(std::string &filename)
{
    return this->m_sndbuf.ReadHeader(filename);
}

SAMPLE 
dbGrainBuf::Tick(SAMPLE in)
{
    if(this->m_bypassGrains)
        return this->m_sndbuf.Sample();
    else
    {
        // Trigger and Duration conspire to characterize the number
        // of active Grains at a given time.  Faster triggers (say 100hz)
        // with longer durations (say 10sec) would require more live grains
        // than we can afford (~1000). (Supercollider default max is 512).
        if(this->m_trigger.IsTriggered())
        {
            Grain *g = this->m_grainMgr.Activate();
            if(g)
            {
                // at the moment we create a Grain, we 'sample' the 
                // parameter generators and then initialize the grain: 
                // NB: we'd like to support CC for each of these so users
                // can wire-up arbitrary behavior.
                // 
                //    dur: within a range 
                //    pos: 
                //          constant, 
                //          sliding range with randomness (looping implicitly)
                //          random locations
                //    rate:
                // 
                g->InitDur();
            }
            else
            {
                std::cout << "DbGrainBuf: too many active grains." << std::endl;
            }
        }

        SAMPLE sum = 0;
        for(const auto& value: this->m_grainMgr.ActiveGrains) 
        {
            // Value ia Grain *
            Grain *g = value;
        }
        return sum;
    }
}

void 
dbGrainBuf::applyWindow(float * data, float * window, 
    unsigned long length)
{
    unsigned long i;
    for( i = 0; i < length; i++ )
        data[i] *= window[i];
}

void 
dbGrainBuf::hanning(float * window, unsigned long length)
{
    unsigned long i;
    float phase = 0, delta = TWO_PI / (float) length;
    for(i = 0; i < length; i++)
    {
        window[i] = (float)(0.5 * (1.0 - cos(phase)));
        phase += delta;
    }
}

void 
dbGrainBuf::hamming(float * window, unsigned long length )
{
    unsigned long i;
    double phase = 0, delta = TWO_PI / (double) length;
    for( i = 0; i < length; i++ )
    {
        window[i] = (float)(0.54 - .46*cos(phase));
        phase += delta;
    }
}

void 
dbGrainBuf::blackman( float * window, unsigned long length )
{
    unsigned long i;
    double phase = 0, delta = TWO_PI / (double) length;
    for( i = 0; i < length; i++ )
    {
        window[i] = (float)(0.42 - .5*cos(phase) + .08*cos(2*phase));
        phase += delta;
    }
}

void 
dbGrainBuf::bartlett( float* window, unsigned long length )
{
    unsigned long i;
    float half = (float)length / 2;
    for( i = 0; i < length; i++ )
    {
        if( i < half ) window[i] = i / half;
        else window[i] = (length - i) / half;
    }
}
