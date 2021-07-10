/**
 * Our job is to create and combine a number of in-flight grains into 
 * a single output sample. All grains index into the one sndbufState.
 * sndbufState lazily loads chunks to minimize startup costs.
 */


#include "dbSndBuf.h"
#include <string>

typedef float t_float;

class dbGrainBufImpl
{
public:
    dbGrainBufImpl(float sampleRate);
    ~dbGrainBufImpl();

    SAMPLE Tick(SAMPLE in);
    int Read(std::string &filename);
    int SetLoop(int loop);
    int GetLoop();

private:
    struct grainState
    {
        t_float m_phase, m_rate;
        t_float m_b1, m_y1, m_y2, m_curamp, m_winPos, m_winInc;
        t_float m_pan1, m_pan2, m_winType;
        int m_counter, m_chan, m_bufnum, m_interp;
    };

    dbSndBuf m_sndbuf;

    t_float m_curtrig;
    int m_numActive, m_channels, m_MaxGrains;
    grainState* m_Grains;

    void applyWindow(t_float * data, t_float * window, 
        unsigned long length);
    void hanning(t_float *w, unsigned long len);
    void hamming(t_float *w, unsigned long len);
    void blackman(t_float *w, unsigned long len);
    void bartlett(t_float *w, unsigned long len);

};