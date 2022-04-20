#ifndef sin_h
#define sin_h
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

#define SIN_DELTA
#define SIN_LG_N_SAMPLES 10
#define SIN_N_SAMPLES (1 << SIN_LG_N_SAMPLES)

class Sin 
{
public:
    Sin();
    static void init();
    static int32_t lookup(int32_t phase)
    {
        const int SHIFT = 24 - SIN_LG_N_SAMPLES;
        int lowbits = phase & ((1 << SHIFT) - 1);
    #ifdef SIN_DELTA
        int phase_int = (phase >> (SHIFT - 1)) & ((SIN_N_SAMPLES - 1) << 1);
        int dy = s_sintab[phase_int];
        int y0 = s_sintab[phase_int + 1];
        return y0 + (((int64_t)dy * (int64_t)lowbits) >> SHIFT);
    #else
        int phase_int = (phase >> SHIFT) & (SIN_N_SAMPLES - 1);
        int y0 = s_sintab[phase_int];
        int y1 = s_sintab[phase_int + 1];
        return y0 + (((int64_t)(y1 - y0) * (int64_t)lowbits) >> SHIFT);
        #endif
    }
    static int32_t compute(int32_t phase);

    // A more accurate sine, both input and output Q30
    static int32_t compute10(int32_t phase);

private:
#ifdef SIN_DELTA
    static int32_t s_sintab[SIN_N_SAMPLES << 1];
#else
    static int32_t s_sintab[SIN_N_SAMPLES + 1];
#endif
};


#endif
