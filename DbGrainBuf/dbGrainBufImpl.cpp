#include "dbGrainBufImpl.h"
#include <math.h>

dbGrainBufImpl::dbGrainBufImpl(float sampleRate) :
    m_sndbuf(sampleRate)
{
    // WIP
}

void 
dbGrainBufImpl::hanning(t_float * window, unsigned long length)
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
dbGrainBufImpl::hamming(t_float * window, unsigned long length )
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
dbGrainBufImpl::blackman( t_float * window, unsigned long length )
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
dbGrainBufImpl::bartlett( t_float* window, unsigned long length )
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
dbGrainBufImpl::applyWindow(t_float * data, t_float * window, 
    unsigned long length)
{
    unsigned long i;
    for( i = 0; i < length; i++ )
        data[i] *= window[i];
}