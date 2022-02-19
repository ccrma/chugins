// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)

#include <cstdint>
#include <cmath>

typedef struct { uint64_t state;  uint64_t inc; } pcg32_random_t;

uint32_t 
pcg32_random_r(pcg32_random_t* rng)
{
    uint64_t oldstate = rng->state;
    // Advance internal state
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
    // Calculate output function (XSH RR), uses old state for max ILP
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    #pragma warning( disable : 4146 )  /* -rot warning */
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

/* return a random float on [0, 1) */
float
pcg32_random_f(pcg32_random_t &rng)
{
    return ldexpf((float) pcg32_random_r(&rng), -32);
}

class DbNoise
{
public:
    DbNoise(unsigned repeats=300)
    {
        this->ctx = DbNoise::getNextContext();
        this->counter = repeats;
        this->repeats = repeats;
        this->lastSig = 0.f;
    }

    void SetRepeats(unsigned repeats)
    {
        this->repeats = repeats;
        this->counter = repeats;
    }

    float tick() // returns [-1, 1)
    {
        if(++this->counter > this->repeats)
        {
            this->lastSig = (2.f * pcg32_random_f(this->ctx)) - 1.f;
            this->counter = 1;
        }
        return lastSig;
    }

private:
    pcg32_random_t ctx;
    float lastSig;
    unsigned counter;
    unsigned repeats;

private:
    static pcg32_random_t getNextContext()
    {
        static pcg32_random_t x = {0, 0};
        x.state++;
        return x;
    }
};