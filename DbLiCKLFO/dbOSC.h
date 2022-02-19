#define _USE_MATH_DEFINES
#include <math.h> // want constants

const float kTwoPi = M_PI * 2.;

class DbOSC
{
public:
    DbOSC(float srate) :
        sampleRate(srate),
        phase(0.f)
    {
        this->SetFreq(120);
    }

    void SetFreq(float freq)
    {
        this->freq = freq;
        this->num = freq / this->sampleRate;
    }

    float GetPhase()
    {
        return this->phase;
    }

    void IncPhase()
    {
        this->phase += this->num;
        if(this->phase > 1.f)
            this->phase -= 1.f;
        else
        if(this->phase < 0.f)
            this->phase += 1.f;
    }

    float Saw(float t)
    {
        float phase = t + .25; 
        if(phase > 1.0) 
            phase -= 1.0;
        return 1.0f - 2.0f * phase;
    }

    float Sine(float t)
    {
        return ::sin(t * 6.24);
    }

    // pulse requires extra width argument.

    float Sqr(float t)
    {
        return t < .5f ? 1.f : -1.f;
    }

    float Tri(float t) // no width control
    {
        float sig;
        float x = t + .25f; // at x == .25, we're at a zero crossing
        if(x > 1.f) 
            x -= 1.f;
        if(x < .5f) // positive slope
            sig = -1.f + 4.f * x; 
        else
            sig = 1.f - 4.f * (x-.5f);
        return sig;
    }

    float Hyper(float t)
    {
        // based on LiCK's hyper LFO inspired by
        // http://hammer.ampage.org/files/hypertriangleclock.gif
        // hack is to tree inverted sign as exponential sweep.
        // perhaps we should just pay for the pow?
        // compare https://www.desmos.com/calculator
        // x^2 to 1 - abs(cos(x*1.6))
        #if 0
        float x = Sine(t*.5f); // because 1/2 wave is a "cycle"
        // fullwave rectifier
        if(x < 0.f) x = -x; 
        // invert
        x = 1.f - x;  /* signal is still 0->1 */
        // remap [-1, 0] => [-1, 1]
        x = -1.f + 2.f * x;
        return x;
        #else
        // sine-based solution looks like a u
        // we'll fallow suit
        if(t < .5f)
            t *= 2.f;
        else
            t = 2. * (1.f - t);
        return 2.f * t * t - 1.f;
        #endif
    }

private:
    float freq;
    float num;
    float sampleRate;
    float phase;
};