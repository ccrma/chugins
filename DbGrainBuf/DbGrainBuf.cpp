#include "dbGrainBuf.h"
#include <math.h>

dbGrainBuf::dbGrainBuf(float sampleRate) :
    m_sndbuf(sampleRate)
{
    // WIP
}

dbGrainBuf::~dbGrainBuf()
{
}

int 
dbGrainBuf::Read(std::string &filename)
{
    return this->m_sndbuf.ReadHeader(filename);
}

int
dbGrainBuf::SetLoop(int loop)
{
    this->m_sndbuf.SetLoop(loop);
    return loop;
}

int 
dbGrainBuf::GetLoop()
{
    return this->m_sndbuf.GetLoop();
}

int
dbGrainBuf::SetPos(int pos)
{
    this->m_sndbuf.SetPosition(pos);
    return pos;
}

int 
dbGrainBuf::GetPos()
{
    return this->m_sndbuf.GetPosition(); // int-version is sample index
}

float
dbGrainBuf::SetRate(float rate)
{
    this->m_sndbuf.SetRate(rate);
    return rate;
}

float 
dbGrainBuf::GetRate()
{
    return this->m_sndbuf.GetRate();
}

int
dbGrainBuf::SetMaxFilt(int w)
{
    return this->m_sndbuf.SetMaxFilt(w);
}

int
dbGrainBuf::GetMaxFilt()
{
    return this->m_sndbuf.GetMaxFilt();
}

SAMPLE 
dbGrainBuf::Tick(SAMPLE in)
{
    return this->m_sndbuf.Sample();
}

void 
dbGrainBuf::hanning(t_float * window, unsigned long length)
{
    unsigned long i;
    t_float phase = 0, delta = TWO_PI / (t_float) length;
    for(i = 0; i < length; i++)
    {
        window[i] = (t_float)(0.5 * (1.0 - cos(phase)));
        phase += delta;
    }
}

void 
dbGrainBuf::hamming(t_float * window, unsigned long length )
{
    unsigned long i;
    double phase = 0, delta = TWO_PI / (double) length;
    for( i = 0; i < length; i++ )
    {
        window[i] = (t_float)(0.54 - .46*cos(phase));
        phase += delta;
    }
}

void 
dbGrainBuf::blackman( t_float * window, unsigned long length )
{
    unsigned long i;
    double phase = 0, delta = TWO_PI / (double) length;
    for( i = 0; i < length; i++ )
    {
        window[i] = (t_float)(0.42 - .5*cos(phase) + .08*cos(2*phase));
        phase += delta;
    }
}

void 
dbGrainBuf::bartlett( t_float* window, unsigned long length )
{
    unsigned long i;
    t_float half = (t_float)length / 2;
    for( i = 0; i < length; i++ )
    {
        if( i < half ) window[i] = i / half;
        else window[i] = (length - i) / half;
    }
}

//-----------------------------------------------------------------------------
// name: apply_window()
// desc: apply a window to data
//-----------------------------------------------------------------------------
void 
dbGrainBuf::applyWindow(t_float * data, t_float * window, 
    unsigned long length)
{
    unsigned long i;
    for( i = 0; i < length; i++ )
        data[i] *= window[i];
}