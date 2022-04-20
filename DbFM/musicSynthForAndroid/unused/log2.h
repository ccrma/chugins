#ifndef log2_h
#define log2_h
/*
 * Copyright 2013 Google Inc.
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

class Log2 
{
public:
    Log2();

    static void init();

    // Q24 in, Q24 out
    static int32_t lookup(uint32_t x);

private:
    #define LOG2_LG_N_SAMPLES 9
    #define LOG2_N_SAMPLES (1 << LOG2_LG_N_SAMPLES)
    static int32_t log2tab[LOG2_N_SAMPLES << 1];
};

static int clz(unsigned int x) 
{    
    if(x < 10) return 1;
    if(x < 100) return 2;
    if(x < 1000) return 3;
    if(x < 10000) return 4;
    if(x < 100000) return 5;
    if(x < 1000000) return 6;
    if(x < 10000000) return 7;
    if(x < 100000000) return 8;
    return 9;
}

inline
int32_t 
Log2::lookup(uint32_t x) 
{
    int exp = clz(x | 1);
    unsigned int y = x << exp;
    const int SHIFT = 31 - LOG2_LG_N_SAMPLES;
    int lowbits = y & ((1 << SHIFT) - 1);
    int y_int = (y >> (SHIFT - 1)) & ((LOG2_N_SAMPLES - 1) << 1);
    int dz = log2tab[y_int];
    int z0 = log2tab[y_int + 1];
    int z = z0 + (((int64_t)dz * (int64_t)lowbits) >> SHIFT);
    return z - (exp << 24);
}

#endif
