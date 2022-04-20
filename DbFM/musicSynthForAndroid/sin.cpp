/*
 * Copyright 2012 Google Inc.
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

#include "sin.h"
#include "synth.h"

#define _USE_MATH_DEFINES
#include <cmath>

#define R (1 << 29)

#ifdef SIN_DELTA
int32_t Sin::s_sintab[SIN_N_SAMPLES << 1];
#else
int32_t Sin::s_sintab[SIN_N_SAMPLES + 1];
#endif

/*static*/ void 
Sin::init() 
{
    double dphase = 2 * M_PI / SIN_N_SAMPLES;
    int32_t c = (int32_t)floor(cos(dphase) * (1 << 30) + 0.5);
    int32_t s = (int32_t)floor(sin(dphase) * (1 << 30) + 0.5);
    int32_t u = 1 << 30;
    int32_t v = 0;
    for (int i = 0; i < SIN_N_SAMPLES / 2; i++) 
    {
#ifdef SIN_DELTA
        s_sintab[(i << 1) + 1] = (v + 32) >> 6;
        s_sintab[((i + SIN_N_SAMPLES / 2) << 1) + 1] = -((v + 32) >> 6);
#else
        s_sintab[i] = (v + 32) >> 6;
        s_sintab[i + SIN_N_SAMPLES / 2] = -((v + 32) >> 6);
#endif
        int32_t t = ((int64_t)u * (int64_t)s + (int64_t)v * (int64_t)c + R) >> 30;
        u = ((int64_t)u * (int64_t)c - (int64_t)v * (int64_t)s + R) >> 30;
        v = t;
    }
#ifdef SIN_DELTA
    for(int i = 0; i < SIN_N_SAMPLES - 1; i++) 
        s_sintab[i << 1] = s_sintab[(i << 1) + 3] - s_sintab[(i << 1) + 1];
    s_sintab[(SIN_N_SAMPLES << 1) - 2] = -s_sintab[(SIN_N_SAMPLES << 1) - 1];
#else
    s_sintab[SIN_N_SAMPLES] = 0;
#endif
}

// static lookup method is inlined

#if 0
// The following is an implementation designed not to use any lookup tables,
// based on the following implementation by Basile Graf:
// http://www.rossbencina.com/static/code/sinusoids/even_polynomial_sin_approximation.txt

#define C0 (1 << 24)
#define C1 (331121857 >> 2)
#define C2 (1084885537 >> 4)
#define C3 (1310449902 >> 6)

int32_t Sin::compute(int32_t phase) 
{
    int32_t x = (phase & ((1 << 23) - 1)) - (1 << 22);
    int32_t x2 = ((int64_t)x * (int64_t)x) >> 22;
    int32_t x4 = ((int64_t)x2 * (int64_t)x2) >> 24;
    int32_t x6 = ((int64_t)x2 * (int64_t)x4) >> 24;
    int32_t y = C0 -
        (((int64_t)C1 * (int64_t)x2) >> 24) +
        (((int64_t)C2 * (int64_t)x4) >> 24) -
        (((int64_t)C3 * (int64_t)x6) >> 24);
    y ^= -((phase >> 23) & 1);
    return y;
}
#endif

#if 1

// coefficients are Chebyshev polynomial, computed by compute_cos_poly.py
#define C8_0 16777216
#define C8_2 -331168742
#define C8_4 1089453524
#define C8_6 -1430910663
#define C8_8 950108533

/*static*/ int32_t 
Sin::compute(int32_t phase) 
{
    int32_t x = (phase & ((1 << 23) - 1)) - (1 << 22);
    int32_t x2 = ((int64_t)x * (int64_t)x) >> 16;
    int32_t y = (((((((((((((int64_t)C8_8
        * (int64_t)x2) >> 32) + C8_6)
        * (int64_t)x2) >> 32) + C8_4)
        * (int64_t)x2) >> 32) + C8_2)
        * (int64_t)x2) >> 32) + C8_0);
    y ^= -((phase >> 23) & 1);
    return y;
}
#endif

#define C10_0 (1 << 30)
#define C10_2 -1324675874  // scaled * 4
#define C10_4 1089501821
#define C10_6 -1433689867
#define C10_8 1009356886
#define C10_10 -421101352
/*static*/ int32_t 
Sin::compute10(int32_t phase) 
{
    int32_t x = (phase & ((1 << 29) - 1)) - (1 << 28);
    int32_t x2 = ((int64_t)x * (int64_t)x) >> 26;
    int32_t y = ((((((((((((((((int64_t)C10_10
        * (int64_t)x2) >> 34) + C10_8)
        * (int64_t)x2) >> 34) + C10_6)
        * (int64_t)x2) >> 34) + C10_4)
        * (int64_t)x2) >> 32) + C10_2)
        * (int64_t)x2) >> 30) + C10_0);
    y ^= -((phase >> 29) & 1);
    return y;
}
