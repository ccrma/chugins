#ifndef dbRand_h
#define dbRand_h

#include <stdint.h>

// from: https://github.com/lemire/fastrange (apache2 licensed)
inline uint32_t rand32()
{
    /* The state word must be initialized to non-zero */
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
    static uint32_t state = 55;
	uint32_t x = state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return state = x;
}

inline uint32_t rand32Range(int32_t range) 
{
    uint64_t v = rand32();
    uint64_t prod = v * range;
    uint32_t low = (uint32_t) prod;
    if(low < range) 
    {
        uint32_t threshold = uint32_t(-range) % range ;
        while (low < threshold) 
        {
            v = rand32();
            prod = v * range;
            low = (uint32_t) prod;
        }
    }
    return prod >> 32;
}

inline int32_t rand32HalfRange(int32_t range) 
{
    uint64_t v = rand32();
    uint64_t prod = v * range;
    uint32_t lowbits = (uint32_t) prod;
    if(lowbits < range) 
    {
        uint32_t threshold = uint32_t(-range) % range ;
        while (lowbits < threshold) 
        {
            v = rand32();
            prod = v * range;
            lowbits = (uint32_t) prod;
        }
    }
    int32_t ret = ((prod >> 32)) - (range/2);
    return ret;
}

inline float rand32Pct() 
{
    return rand32Range(1000) / 1000.0f;
}

#endif
