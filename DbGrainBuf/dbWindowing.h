#ifndef dbWindowing_h
#define dbWindowing_h

#include <cmath>

/**
 * a fixed-size windowing function with linearly interpolated weights.
 */
static class dbWindowing *sSingleton = nullptr;
class dbWindowing
{
public:
    static dbWindowing *Get()
    {
        if(!sSingleton)
        {
            sSingleton = new dbWindowing();
        }
        return sSingleton;
    }

    inline float Sample(float pct)
    {
        if(pct > .9999f)
            return this->window[8191];
        else
        if(pct < .0002)
            return this->window[0];

        float fpos = pct * 8191;
        unsigned pos = fpos;
        float alpha = fpos - pos;
        float s1 = this->window[pos];
        float s2 = this->window[pos+1];
        return s1 + pct * (s2-s1);
    }

private:
    dbWindowing()
    {
        blackman(this->window, sizeof(window)/sizeof(float));
    }
    float window[8192];

    static void hanning(float * window, unsigned long length)
    {
        unsigned long i;
        float phase = 0, delta = TWO_PI / (float) length;
        for(i = 0; i < length; i++)
        {
            window[i] = (float)(0.5 * (1.0 - cos(phase)));
            phase += delta;
        }
    }
    static void hamming(float * window, unsigned long length )
    {
        unsigned long i;
        double phase = 0, delta = TWO_PI / (double) length;
        for( i = 0; i < length; i++ )
        {
            window[i] = (float)(0.54 - .46*cos(phase));
            phase += delta;
        }
    }
    static void blackman( float * window, unsigned long length )
    {
        unsigned long i;
        double phase = 0, delta = TWO_PI / (double) length;
        for( i = 0; i < length; i++ )
        {
            window[i] = (float)(0.42 - .5*cos(phase) + .08*cos(2*phase));
            phase += delta;
        }
    }
    static void bartlett( float* window, unsigned long length )
    {
        unsigned long i;
        float half = (float)length / 2;
        for( i = 0; i < length; i++ )
        {
            if( i < half ) window[i] = i / half;
            else window[i] = (length - i) / half;
        }
    }
};

#endif