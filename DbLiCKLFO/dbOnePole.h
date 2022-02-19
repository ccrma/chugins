#ifdef __arm__
inline void ddn(float &x) {}
#else
inline void ddn(float &x) 
{
    if(x >= 0.f)
    {
        if(x < 1e-15f || x > 1e15f)
            x = 0.f;
    }
    else
    if(x > -1e-15f || x < -1e15f)
        x = 0.f;
}
#endif

class DbOnePole // for "natural" modulation, derived from Stk
{
public:
    DbOnePole(float pole=.999f)
    {
        if(pole > 0.f)
            this->b0 = 1.f - pole;
        else
            this->b0 = 1.f + pole;
        this->a0 = 1.f;
        this->a1 = -pole;
        this->gain = 1.f;
        this->out0 = 0.f;
        this->out1 = 0.f;
    }
    void setGain(float g)
    {
        this->gain = g;
    }
    float tick(float x)
    {
        this->in0 = this->gain * x;
        this->out0 = this->b0 * this->in0 - this->a1 * this->out1;
        this->out1 = this->out0;
        ddn(this->out1);
        return this->out0;
    }

private:
    float a0, a1;
    float b0;
    float gain;
    float in0, in1;
    float out0, out1;
};