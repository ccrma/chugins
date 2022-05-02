#ifndef dbWindowing_h
#define dbWindowing_h

#include <cmath>
#include <memory>

const float kPI = 3.14159265358979323846;
const float kTwoPI = 2.f * kPI;
const int kDefaultTableLength = 8192;

/**
 * a fixed-size windowing function with linearly interpolated weights.
 */
static std::unique_ptr<class dbWindowing> sSingletons[6];

class dbWindowing
{
public:
    enum FilterType
    {
        k_Sine, // since we window twice (in and out), this turns into Hann
        k_Hann,  
        k_Hamming,
        k_Blackman, 
        k_Bartlett,
        k_PlanckTaper85, // for long grains (less fading at edges)
        k_PlanckTaper95 // for long grains (less fading at edges)
    };
    static dbWindowing *Get(FilterType t=k_Sine)
    {
        unsigned i = (unsigned) t;
        if(!sSingletons[i])
        {
            // std::cout << "new window " << i << std::endl;
            sSingletons[i] = std::unique_ptr<dbWindowing>(new dbWindowing(t));
        }
        return sSingletons[i].get();
    }

    inline float Sample(float pct) const
    {
        if(pct > .9999f)
            return m_window[8191];
        else
        if(pct < .0002)
            return m_window[0];

        float fpos = pct * 8191;
        unsigned pos = fpos;
        float alpha = fpos - pos;
        float s1 = m_window[pos];
        float s2 = m_window[pos+1];
        return s1 + alpha * (s2-s1);
    }

    inline void Apply(float *data, int len) const
    {
        float dphase = 1.0f / (len-1);
        float phase = 0.f;
        for(int i=0;i<len;i++)
        {
            data[i] *= this->Sample(phase);
            phase += dphase;
        }
    }

    ~dbWindowing() // for unique_ptr destruction
    {
        delete [] m_window;
    }

private:
    dbWindowing(FilterType t=k_Blackman, int len=8192)
    {
        m_window = new float[len];
        switch(t)
        {
        case k_Sine:
            sine(m_window, len);
            break;
        case k_Hann:
            hann(m_window, len);
            break;
        case k_Blackman:
            blackman(m_window, len);
            break;
        case k_Hamming:
            hamming(m_window, len);
            break;
        case k_Bartlett:
            bartlett(m_window, len);
            break;
        case k_PlanckTaper85:
            planckTaper(m_window, len, .15f);
            break;
        case k_PlanckTaper95:
            planckTaper(m_window, len, .05f);
            break;
        }
    }

    float *m_window;

    static void sine(float * window, int length) 
    {
        float phase = 0, delta = kPI / (float) length;
        for(int i = 0; i < length; i++)
        {
            window[i] = sinf(phase);
            phase += delta;
        }
    }

    static void hann(float * window, int length) // aka hann
    {
        int i;
        float phase = 0, delta = kPI / (float) length;
        for(i = 0; i < length; i++)
        {
            double s = sin(phase);
            window[i] = s * s;
            phase += delta;
        }
    }

    static void blackman( float * window, int length )
    {
        int i;
        double phase = 0, delta = kTwoPI / (double) length;
        for( i = 0; i < length; i++ )
        {
            window[i] = (float)(0.42 - .5*cos(phase) + .08*cos(2*phase));
            phase += delta;
        }
    }
    static void hamming(float * window, int length )
    {
        int i;
        double phase = 0, delta = kTwoPI / (double) length;
        for( i = 0; i < length; i++ )
        {
            window[i] = (float)(0.54 - .46*cos(phase));
            phase += delta;
        }
    }
    static void bartlett( float* window, int length )
    {
        int i;
        float half = (float)length / 2;
        for( i = 0; i < length; i++ )
        {
            if( i < half ) window[i] = i / half;
            else window[i] = (length - i) / half;
        }
    }
    static float za(int k, float lowKnee)
    {
        return lowKnee*(1.f/k + 1.f/(k-lowKnee));
    }
    static float zb(int k, float lowKnee,
        int max, float eps)
    {
        return lowKnee *
            (1.f/(max-k) + 1.f/((1.f-eps)*max - k));
    }
    static void planckTaper(float* window, int length,
        float eps)
    {
        const int max = length - 1;
        const float lowKnee = eps * max;
        const float highKnee = (1.f-eps) * max;
        int k=0;
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