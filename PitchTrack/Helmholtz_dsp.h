#ifndef Helmholtz_H
#define Helmholtz_H


/***********************************************************************

Class Helmholtz implements a period-length detector using Philip McLeod's
Specially Normalized AutoCorrelation function (SNAC).

Function iosamples() takes a pointer to a buffer with n signal input samples 
and a pointer to a buffer where n output samples are stored, 
representing the SNAC function.

Via function setframesize(), analysis frame size can be set to 
128, 256, 512, 1024 or 2048 samples. Default is 1024.

With setoverlap(), analysis frames can be set to overlap each other 
with factor 1, 2, 4 or 8. Default is 1.

Function setbias() sets a bias which favours small lags over large lags in 
the period detection, thereby avoiding low-octave jumps. Default is 0.2

Function setminRMS() is used as a sensitivity setting. Default is RMS 0.003.

With function getperiod(), the last detected period length is returned 
as number of samples with a possible fraction (floating point format). 

Function getfidelity() returns a value between 0. and 1. indicating
to which extent the input signal is periodic. A fidelity of ~0.95 can
be considered to indicate a periodic signal.

Class Helmholtz needs mayer_realfft() and mayer_realifft() or similar
fft functions. Note that Ron Mayer's functions for real fft have a
peculiar organisation of imaginary coefficients (reversed order, sign flipped).

Class Helmholtz uses t_float for float or double. Depending on the context
where the class is used, you may need to define t_float. If used with
PD or MaxMsp, it is already defined.

***********************************************************************

Licensed under three-clause BSD license.
 
Katja Vetter, Feb 2012.

***********************************************************************/

/* This section includes the Pure Data API header. If you build Helmholtz
against another DSP framework, you need to define t_float, and you need to
include Ron Mayer's fft or similar functionality. */

//#include "fft_mayer.c"
#define t_float float
#define REALFFT mayer_realfft
#define REALIFFT mayer_realifft

extern "C"
{
  void REALFFT (int, float*);
  void REALIFFT (int, float*);
}  

#ifdef PD
#include "m_pd.h"
#define t_float float
#define REALFFT mayer_realfft
#define REALIFFT mayer_realifft
#endif

/***********************************************************************/

#define DEFFRAMESIZE 1024       // default analysis framesize
#define DEFOVERLAP 1            // default overlap
#define DEFBIAS 0.2             // default bias
#define DEFMINRMS 0.003         // default minimum RMS 
#define SEEK 0.85               // seek-length as ratio of framesize


class Helmholtz
{
public:
    Helmholtz(int periodarg = DEFFRAMESIZE, int overlaparg = DEFOVERLAP, t_float biasarg = DEFBIAS); 
    
    ~Helmholtz();                                            
    void iosamples(t_float *in, t_float *out, int size);
    void setframesize(int frame);
    void setoverlap(int lap);
    void setbias(t_float bias);
    void setminRMS(t_float rms);
    t_float getperiod() const;
    t_float getfidelity() const;
    
private:
    // procedures
    void analyzeframe();
    void autocorrelation();
    void normalize();
    void pickpeak();
    void periodandfidelity();
    
    // functions
    t_float interpolate3max(t_float *buf, int peakindex);
    t_float interpolate3phase(t_float *buf, int peakindex);
        
    // buffers
    t_float *inputbuf;
    t_float *inputbuf2;
    t_float *processbuf;
    
    // state variables
    int timeindex;
    int framesize;
    int overlap;
    int periodindex;
    
    t_float periodlength;
    t_float fidelity;
    t_float biasfactor;
    t_float minrms;
};

#endif // #ifndef Helmholtz_H
