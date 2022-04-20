#ifndef exp2_h
#define exp2_h
/*
 * Copyright 2012 Google Inc.
 * Copyright 2022 Dana Batali
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cstdint>

#define EXP2_LG_N_SAMPLES 10
#define EXP2_N_SAMPLES (1 << EXP2_LG_N_SAMPLES)

class Exp2 
{
public:
    Exp2();
    static void init();

    // Q24 in, Q24 out
    static int32_t lookup(int32_t x)
    {
        const int SHIFT = 24 - EXP2_LG_N_SAMPLES;
        int lowbits = x & ((1 << SHIFT) - 1);
        int x_int = (x >> (SHIFT - 1)) & ((EXP2_N_SAMPLES - 1) << 1);
        int dy = exp2tab[x_int];
        int y0 = exp2tab[x_int + 1];
        int y = y0 + (((int64_t)dy * (int64_t)lowbits) >> SHIFT);
        return y >> (6 - (x >> 24));
    }
private:
    static int32_t exp2tab[EXP2_N_SAMPLES << 1];
};

#define TANH_LG_N_SAMPLES 10
#define TANH_N_SAMPLES (1 << TANH_LG_N_SAMPLES)
class Tanh 
{
public:
    static void init();

    // Q24 in, Q24 out
    static int32_t lookup(int32_t x)
    {
        int32_t signum = x >> 31;
        x ^= signum;
        if(x >= (4 << 24)) 
        {
            if (x >= (17 << 23)) 
            {
                return signum ^ (1 << 24);
            }
            int32_t sx = ((int64_t)-48408812 * (int64_t)x) >> 24;
            return signum ^ ((1 << 24) - 2 * Exp2::lookup(sx));
        } 
        else 
        {
            const int SHIFT = 26 - TANH_LG_N_SAMPLES;
            int lowbits = x & ((1 << SHIFT) - 1);
            int x_int = (x >> (SHIFT - 1)) & ((TANH_N_SAMPLES - 1) << 1);
            int dy = tanhtab[x_int];
            int y0 = tanhtab[x_int + 1];
            int y = y0 + (((int64_t)dy * (int64_t)lowbits) >> SHIFT);
            return y ^ signum;
        }
    }
private:
    static int32_t tanhtab[TANH_N_SAMPLES << 1];
};

#endif