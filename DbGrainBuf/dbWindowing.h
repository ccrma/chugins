#ifndef dbWindowing_h
#define dbWindowing_h

#include <cmath>

/**
 * a fixed-size windowing function with linearly interpolated weights.
 */
static class dbWindowing *sSingletons[6] = {
    nullptr, nullptr, nullptr, 
    nullptr, nullptr, nullptr
};

class dbWindowing
{
public:
    enum FilterType
    {
        // general purpose
        kBlackman = 0, 
        kHanning = 1,  
        kHamming = 2,
        kBartlett = 3,
        kPlanckTaper85 = 4, // for long grains (less fading at edges)
        kPlanckTaper95 = 5 // for long grains (less fading at edges)
    };
    static dbWindowing *Get(FilterType t=kBlackman)
    {
        unsigned i = (unsigned) t;
        if(!sSingletons[i])
        {
            // std::cout << "new window " << i << std::endl;
            sSingletons[i] = new dbWindowing(t);
        }
        return sSingletons[i];
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
    dbWindowing(FilterType t=kBlackman, unsigned len=8192)
    {
        this->window = new float[len];
        switch(t)
        {
        case kBlackman:
            blackman(this->window, len);
            break;
        case kHanning:
            hanning(this->window, len);
            break;
        case kHamming:
            hamming(this->window, len);
            break;
        case kBartlett:
            bartlett(this->window, len);
            break;
        case kPlanckTaper85:
            planckTaper(this->window, len, .15f);
            break;
        case kPlanckTaper95:
            planckTaper(this->window, len, .05f);
            break;
        }
    }
    float *window;

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
    static float za(unsigned long k, float lowKnee)
    {
        return lowKnee*(1.f/k + 1.f/(k-lowKnee));
    }
    static float zb(unsigned long k, float lowKnee,
        unsigned long max, float eps)
    {
        return lowKnee *
            (1.f/(max-k) + 1.f/((1.f-eps)*max - k));
    }
    static void planckTaper(float* window, unsigned long length,
        float eps)
    {
        const unsigned long max = length - 1;
        const float lowKnee = eps * max;
        const float highKnee = (1.f-eps) * max;
        unsigned long k=0;
        window[k++] = 0.f;
        for(;k<lowKnee;k++)
        {
            window[k] = 1.0 / (exp(za(k, lowKnee)) + 1);
        }
        for(;k<=highKnee;k++)
        {
            window[k] = 1.0f;
        }
        for(;k<max;k++)
        {
            window[k] = 1.0 / (exp(zb(k, lowKnee, max, eps)) + 1);
        }
        window[k] = 0.f;
    }
};

#endif