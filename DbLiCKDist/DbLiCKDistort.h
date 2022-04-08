#ifndef DbLiCKDistort_h
#define DbLiCKDistort_h

// These distortion functions were ported by Dana Batali from 
// https://github.com/hauermh/lick and subject to its GPL3 license.
//
// See http://electro-music.com/forum/topic-19287.html&postorder=asc

#include <cmath>
#include <cstring> // memset
#include <string>
#include <iostream>

class DbLiCKDistort
{
public:
    DbLiCKDistort(char const *nmIgnored) {}
    virtual ~DbLiCKDistort() {};
    virtual void SetParam(int i, float x) {}
    virtual float Doit(float x) = 0;

    void SetSampleRate(float srate)
    {
        this->sampleRate = srate;
    }

    float FreqToStep(float freq)
    {
        return freq/this->sampleRate;
    }

protected:
    float sampleRate; 
};

class WaveShaper : public DbLiCKDistort // default WaveShaper
{
public:
    WaveShaper() : DbLiCKDistort("WaveShaper") {}
    float Doit(float x) override
    {
        return x / (1.f + fabsf(x));
    }
};

class Atan : public DbLiCKDistort
{
public:
    Atan() : DbLiCKDistort("AtanDist") {}
    float Doit(float x) override
    {
        return atanf(x) / M_PI_2;
    }
};

// db: not sure what the intent is here. ck implementation has
// even and odd running in different shred each 2 and 4 samples.
// Seems to suggest that dropping samples is intended?
class BucketBrigade : public DbLiCKDistort
{
public:
    int stages[512];
    unsigned state;
    float last;

    BucketBrigade() :
        DbLiCKDistort("BucketBrigade")

    {
        memset(this->stages, 0, sizeof(this->stages));
        this->state = 0;
    }

    float Doit(float x) override
    {
        this->last = x;
        if(this->state == 0)
        {
            this->even();
            this->state++;
        }
        else
        if(this->state == 1)
        {
            this->odd();
            this->state = 0;
        }
        else
            this->state++;
        
        return this->stages[511];
    }

    void odd()
    {
        this->last = this->stages[0];
        for(int i=1;i<511;i+=2)
            this->stages[i+1] = this->stages[i];
    }

    void even() 
    {
        this->last = this->stages[0];
        for(int i=0;i<511;i+=2)
            this->stages[i+1] = this->stages[i];
    }
};

class Clip : public DbLiCKDistort
{
public:
    float min, max;

    Clip() : DbLiCKDistort("Clip") 
    {
        this->setRange(.8);
    }
    
    virtual void SetParam(int i, float x) override
    {
        if(i == 0)
            this->setRange(x);
    }

    void setRange(float x)
    {
        float half = fabsf(x) / 2.f;
        this->min = -half;
        this->max = half;
    }

    float Doit(float x) override
    {
        if(x > this->max)
            return this->max;
        if(x < this->min)
            return this->min;
        return x;
    }
};

/* LiCK's Chew effect has two triangular LFOs running 
 * by default at 4400 hz and depth of .1. These are
 * used to define a top and bottom envelope against
 * which to clip the signal in the clip region we
 * replace the signal with the triwave.
 */
class Chew : public DbLiCKDistort
{
public:
    float lfo0Step; 
    float lfo0Depth;
    float lfo1Step;
    float lfo1Depth;
    float phase0;
    float phase1;

    float cutoff0; 
    float cutoff1;
    float dc0; 
    float dc1;

    Chew() : DbLiCKDistort("Chew") 
    {
        this->lfo0Depth = .1f;
        this->lfo0Step = this->FreqToStep(4400.f);
        this->lfo1Depth = .1f;
        this->lfo1Step = this->FreqToStep(4400.f);
        this->phase0 = 0.f;
        this->phase1 = 0.f;
        this->updateCutoffs();
    }
    
    virtual void SetParam(int i, float x) override
    {
        switch(i)
        {
        case 0: 
            this->lfo0Step = this->FreqToStep(x);
            break;
        case 1:
            this->lfo0Depth = x;
            this->updateCutoffs();
            break;
        case 2:
            this->lfo1Step = this->FreqToStep(x);
            break;
        case 3:
            this->lfo1Depth = x;
            this->updateCutoffs();
            break;
        }
    }

    void updateCutoffs()
    {
        this->cutoff0 = 1.0f - this->lfo0Depth;
        this->dc0 = 1.0f - this->lfo0Depth/2.f;
        this->cutoff1 = -1.0f + this->lfo1Depth;
        this->dc1 = -1.0f + this->lfo1Depth/2.f;
    }

    float triwave(float phase, float depth)
    {
        float sig;
        float x = phase + .25f; // at x == .25, we're at a zero crossing
        if(x > 1.f) 
            x -= 1.f;
        if(x < .5f) // positive slope
            sig = -1.f + 4.f * x; 
        else
            sig = 1.f - 4.f * (x-.5f);
        return sig * depth;
    }

    void incphase(float &phase, float step)
    {
        phase += step;
        if(phase > 1.f)
            phase -= 1.f;
        else
        if(phase < 0.f)
            phase += 1.f;
    }

    float Doit(float x) override
    {
        float sig;
        if(x > this->cutoff0)
        {
            sig = this->dc0 + this->triwave(this->phase0, this->lfo0Depth);
        }
        else
        if(x < this->cutoff1)
        {
            sig = this->dc1 - this->triwave(this->phase1, this->lfo1Depth);
        }
        else
            sig = x;
        this->incphase(this->phase0, this->lfo0Step);
        this->incphase(this->phase1, this->lfo1Step);
        return sig;
    }
};

class Duff : public DbLiCKDistort
{
public:
    float shape; // 0 to 1

    Duff() : DbLiCKDistort("Duff") 
    {
        this->shape = .5;
    }
    
    virtual void SetParam(int i, float x) override
    {
        if(i == 0)
            this->shape = x;
    }

    float Doit(float x) override
    {
        return x / ((1.1f - this->shape) + fabsf(x));
    }
};

class Frostburn : public DbLiCKDistort
{
public:
    Frostburn() : DbLiCKDistort("Frostburn") {}
    float Doit(float x) override
    {
        // f(x) = (x * abs(x) + x) / (x^2 + abs(x) + 1.0)
        return (x * fabsf(x) + x) / (x*x + fabsf(x) + 1.f);
    }
};

class FullRectifier : public DbLiCKDistort
{
public:
    float threshold;
    float bias;
    FullRectifier() : DbLiCKDistort("FullRectifier")
    { 
        this->threshold = 0.f;
        this->bias = 0.f;
    }
    virtual void SetParam(int i, float x) override
    {
        if(i == 0)
            this->threshold = x;
        else
        if(i == 1)
            this->bias = x;
    }
    float Doit(float x) override
    {
        if(x > this->threshold)
            return x + this->bias;
        else
        {
            // when a value is above the threshold we rectify:
            //  threshold:  0, x: -.5 => .5
            //  threshold: .5, x: 0 => 1.
            //  threshold: .5, x: -1 => 1.5 => .5
            //  threshold: .5, x:  0 => 1.0 => 1.
            //  threshold: .5, x: .4 => 1.5 => .6
            float xx = this->threshold + (this->threshold - x) + this->bias;
            if(xx > 1.f)
                xx = 1.f - xx; // another flip
            return xx;
        }
    }
};

class Offset : public DbLiCKDistort
{
public:
    float offset;
    Offset(float o=0.f) : DbLiCKDistort("Offset")
    {
        this->offset = 0.f;
    }
    virtual void SetParam(int i, float x) override
    {
        if(i == 0)
            this->offset = x;
    }
    float Doit(float x) override
    {
        return x + this->offset;
    }
};

class Phase : public DbLiCKDistort
{
public:
    bool inOrOut;
    Phase() : DbLiCKDistort("Phase")
    {
        this->inOrOut = true; // in phase
    }
    virtual void SetParam(int i, float x) override
    {
        if(i == 0)
        {
            this->inOrOut = x > .5;
        }
    }
    float Doit(float x) override
    {
        return this->inOrOut ? x : -x;
    }
};

class Invert : public DbLiCKDistort
{
public:
    Invert() : DbLiCKDistort("Invert") {}
    float Doit(float x) override
    {
        return -x;
    }
};

class KijjazDist : public DbLiCKDistort
{
public:
    KijjazDist() : DbLiCKDistort("KijjazDist") {}
    float Doit(float x) override
    {
        // f(x) = x / (1.0 + x^2)
        return x / (1.f + x*x);
    }
};

class KijjazDist2 : public DbLiCKDistort
{
public:
    KijjazDist2() : DbLiCKDistort("KijjazDist2") {}
    float Doit(float x) override
    {
        // f(x) = x^3 / (1.0 + abs(x^3))
        float v = (x * x * x);
        return v / (1.f + fabsf(v));
    }
};

class KijjazDist3 : public DbLiCKDistort
{
public:
    KijjazDist3() : DbLiCKDistort("KijjazDist3") {}
    float Doit(float x) override
    {
        // f(x) = x(1.0 + x^2) / (1.0 + abs(x(1.0 + x^2)))`
        float v = x * (1.f + (x * x));
        return v / (1.f + fabsf(v));
    }
};

class KijjazDist4 : public DbLiCKDistort
{
public:
    KijjazDist4() : DbLiCKDistort("KijjazDist4") {}
    float Doit(float x) override
    {
        // f(x) = x(1.0 + x^4) / (1.0 + abs(x(1.0 + x^4)))
        float v = x * (1.f + x*x*x*x) ;
        return v / (1.f + fabsf(v));
    }
};

class Ribbon : public DbLiCKDistort
{
public:
    Ribbon() : DbLiCKDistort("Ribbon") {}
    float Doit(float x) override
    {
        // f(x) = x / (0.25 * x^2 + 1.0)
        return x / (.25 * x*x + 1.f);
    }
};

class Tanh : public DbLiCKDistort
{
public:
    Tanh() : DbLiCKDistort("Tanh") {}
    float Doit(float x) override
    {
        return tanh(x);
    }
};

/* -------------------------------------------------------------- */
class DbLiCKDistortMgr
{
private:
    float srate;

public:
    DbLiCKDistortMgr(float sampleRate) : currentDistortion(nullptr) 
    {
        this->srate = sampleRate;
        this->Set("WaveShaper");
    }
    ~DbLiCKDistortMgr() { delete this->currentDistortion; }

    int Set(char const *nm) // return 0 on success
    {
        int err = 0;
        DbLiCKDistort *newDistort = nullptr;
        if(!strcmp(nm, "WaveShaper"))
            newDistort = new WaveShaper();
        else
        if(!strcmp(nm, "Atan"))
            newDistort = new Atan();
        else
        if(!strcmp(nm, "BucketBrigade"))
            newDistort = new BucketBrigade();
        else
        if(!strcmp(nm, "Chew"))
            newDistort = new Chew();
        else
        if(!strcmp(nm, "Clip"))
            newDistort = new Clip();
        else
        if(!strcmp(nm, "Duff"))
            newDistort = new Duff();
        else
        if(!strcmp(nm, "Frostburn"))
            newDistort = new Frostburn();
        else
        if(!strcmp(nm, "FullRectifier"))
            newDistort = new FullRectifier();
        else
        if(!strcmp(nm, "Offset"))
            newDistort = new Offset();
        else
        if(!strcmp(nm, "Phase"))
            newDistort = new Phase();
        else
        if(!strcmp(nm, "Invert"))
            newDistort = new Invert();
        else
        if(!strcmp(nm, "KijjazDist"))
            newDistort = new KijjazDist();
        else
        if(!strcmp(nm, "KijjazDist2"))
            newDistort = new KijjazDist2();
        else
        if(!strcmp(nm, "KijjazDist3"))
            newDistort = new KijjazDist3();
        else
        if(!strcmp(nm, "KijjazDist4"))
            newDistort = new KijjazDist4();
        else
        if(!strcmp(nm, "Ribbon"))
            newDistort = new Ribbon();
        else
        if(!strcmp(nm, "Tanh"))
            newDistort = new Tanh();
        else
        if(strcmp(nm, "None") != 0)
        {
            std::cerr << "DbLiCKDistort unknown effect " << nm << "\n";
            std::cerr << "  expect one of:\n" <<
                "  WaveShaper, Atan, BucketBrigade,\n"  <<
                "  Clip(range), Frostburn, FullRectifier,\n" <<
                "  Offset(x), Phase(x), Invert, " <<
                "  KijjazDist, KijjazDist2, KijjazDist3, KijjazDist4,\n" <<
                "  Ribbon, Tanh, None\n";
        }
        if(!newDistort)
            err = 1;
        else
            newDistort->SetSampleRate(this->srate);

        this->name = nm;
        if(this->currentDistortion)
            delete this->currentDistortion;
        this->currentDistortion = newDistort;
        return err;
    }

    int SetParam(int i, float val)
    {
        int err = 0;
        if(this->currentDistortion)
            this->currentDistortion->SetParam(i, val);
        else
            err = 1;
        return err;
    }

    float Tick(float in)
    {
        if(this->currentDistortion)
            return this->currentDistortion->Doit(in);
        else
            return in;
    }

    DbLiCKDistort *currentDistortion;
    std::string name;
};


#endif
