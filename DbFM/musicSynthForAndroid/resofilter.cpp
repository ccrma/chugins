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

// Resonant filter implementation. This closely follows "Non-Linear
// Digital Implementation of the Moog Ladder Filter" by Antti
// Huovilainen, 2004.

// The full implementation requires both a tuning table and 2x
// oversampling, neither of which are present yet, but we'll get there. 

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "synth.h"
#include "freqlut.h"
#include "exp2.h"
#include "aligned_buf.h"
#include "resofilter.h"

#include <algorithm> // min,max

/* static */ void 
ResoFilter::init(double sample_rate) 
{
   // unused atm 
}

ResoFilter::ResoFilter() 
{
    for (int i = 0; i < 4; i++) 
    {
        x.get()[i] = 0;
#if defined(NONLINEARITY)
        w[i] = 0;
#endif
    }
}

static int32_t compute_alpha(int32_t logf) 
{
    return std::min(1 << 24, Freqlut::lookup(logf));
}

static void matmult4(float dst[16], const float a[16], const float b[16]) 
{
    dst[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
    dst[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
    dst[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
    dst[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];
    dst[4] = a[0] * b[4] + a[4] * b[5] + a[8] * b[6] + a[12] * b[7];
    dst[5] = a[1] * b[4] + a[5] * b[5] + a[9] * b[6] + a[13] * b[7];
    dst[6] = a[2] * b[4] + a[6] * b[5] + a[10] * b[6] + a[14] * b[7];
    dst[7] = a[3] * b[4] + a[7] * b[5] + a[11] * b[6] + a[15] * b[7];
    dst[8] = a[0] * b[8] + a[4] * b[9] + a[8] * b[10] + a[12] * b[11];
    dst[9] = a[1] * b[8] + a[5] * b[9] + a[9] * b[10] + a[13] * b[11];
    dst[10] = a[2] * b[8] + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
    dst[11] = a[3] * b[8] + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];
    dst[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15];
    dst[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
    dst[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
    dst[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];
}

static void matvec4(float dst[4], const float a[16], const float b[4]) 
{
    dst[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
    dst[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
    dst[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
    dst[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];
}

static void vecupdate4(float dst[4], float x, const float a[4]) 
{
    for(int i = 0; i < 4; i++)
        dst[i] += x * a[i];
}

/* compute dst := dst + x * a */
static void matupdate4(float dst[16], float x, const float a[16]) 
{
    for(int i = 0; i < 16; i++)
        dst[i] += x * a[i];
}

static void matcopy(float *dst, const float *src, int n) 
{
    memcpy(dst, src, n * sizeof(float));
}

static void dump_matrix(const float a[20]) 
{
    for (int row = 0; row < 5; row++) 
    {
        printf("%s[", row == 0 ? "[" : " ");
        for(int col = 0; col < 5; col++) 
        {
            float x = row == 0 ? (col == 0 ? 1.0 : 0.0) : a[col * 4 + (row - 1)];
            printf("%6f ", x);
        }
        printf("]%s\n", row == 4 ? "]" : "");
    }
}

static void make_state_transition(float result[20], int32_t f0, int32_t k) 
{
    // TODO: these should depend on k, and be just enough to meet error bound
    int n1 = 4;
    int n2 = 4;
    float f = f0 * (1.0 / (1 << (24 + n2)));
    float k_f = k * (1.0 / (1 << 24));
    k_f = std::min(k_f, 3.98f);

    // these are 5x5 matrices of which we store the bottom 5x4
    // Top row of Jacobian is all zeros
    float j[20] = {0};

    // set up initial jacobian
    j[0] = f;
    j[4] = -f;
    j[5] = f;
    j[9] = -f;
    j[10] = f;
    j[14] = -f;
    j[15] = f;
    j[16] = -k_f * f;
    j[19] = -f;

    // Top row of exponential is [1 0 0 0 0]
    float a[20] = {0};
    a[4] = 1.0;
    a[9] = 1.0;
    a[14] = 1.0;
    a[19] = 1.0;
    
    float c[20];
    matcopy(c, j, 20);

    static const float scales[4] = {1.0, 1/2.0, 1/6.0, 1/24.0};

    // taylor's series to n1
    for(int i = 0; i < n1; i++) 
    {
        float scale = scales[i];
        vecupdate4(a, scale, c);
        matupdate4(a+4, scale, c + 4);
        if (i < n1 - 1) 
        {
            float tmp[20];
            matvec4(tmp, c + 4, j);
            matmult4(tmp + 4, c + 4, j + 4);
            matcopy(c, tmp, 20);
        }
    }

    // repeated squaring
    for(int i = 0; i < n2; i++) 
    {
        float tmp[20];
        matvec4(tmp, a + 4, a);
        matmult4(tmp + 4, a + 4, a + 4);
        for(int j = 0; j < 4; j++) 
        {
            a[j] += tmp[j];
        }
        matcopy(a + 4, tmp + 4, 16);
    }
    matcopy(result, a, 20);
}

#if defined(USE_MATRIX)
static float sigmoid(float x, float overdrive) 
{
    float xs = overdrive * x * (1.0 / (1 << 24));
    float isq = 1.0 / sqrtf(1 + xs * xs);
    return x * isq;
}

void ResoFilter::process(const int32_t **inbufs, const int32_t *control_in,
                         const int32_t *control_last, int32_t **outbufs) 
{
    AlignedBuf<float, 20> a;
    float overdrive = control_in[2] * (1.0 / (1 << 24));
    const int32_t *ibuf = inbufs[0];
    int32_t *obuf = outbufs[0];
    make_state_transition(a.get(), compute_alpha(control_in[0]), control_in[1]);

    if (overdrive < 0.01f) 
    {
        for(int i = 0; i < k_MaxSamples; i++)
        {
            float signal = ibuf[i];
            float tmp[4];
            matvec4(tmp, a.get() + 4, x.get());
            for (int k = 0; k < 4; k++) 
            {
                x.get()[k] = tmp[k] + signal * a.get()[k];
                obuf[i] = x.get()[3];
            }
        }
    }
    else 
    {
        float ogain = 1 + overdrive;
        float k = control_in[1] * (1.0 / (1<<24));
        for (int i = 0; i < 4; i++) 
        {
            a.get()[4 + 5 * i] -= 1.0;
            a.get()[16 + i] += k * a.get()[i];
        }
        for (int i = 0; i < k_MaxSamples; i++) 
        {
            float signal = ibuf[i];
            float tmp[4];
            float tx[4];
            for (int j = 0; j < 4; j++) 
            {
                tx[j] = sigmoid(x.get()[j], overdrive);
            }
            matvec4(tmp, a.get() + 4, tx);
            float xin = sigmoid(signal - k * x.get()[3], overdrive);
            for(int j = 0; j < 4; j++) 
            {
                x.get()[j] += tmp[j] + xin * a.get()[j];
                obuf[i] = x.get()[3] * ogain;
            }
        }
    }
}
#else
void ResoFilter::process(const int32_t **inbufs, const int32_t *control_in,
                         const int32_t *control_last, int32_t **outbufs) 
{
    int32_t alpha = compute_alpha(control_last[0]);
    int32_t alpha_in = compute_alpha(control_in[0]);
    int32_t delta_alpha = (alpha_in - alpha) >> lg_n;
    int32_t k = control_last[1];
    int32_t k_in = control_in[1];
    int32_t delta_k = (k_in - k) >> lg_n;
    if((((int64_t)alpha_in * (int64_t)k_in) >> 24) > 1 << 24)
        k_in = ((1 << 30) / alpha_in) << 18;
    if((((int64_t)alpha * (int64_t)k) >> 24) > 1 << 24)
        k = ((1 << 30) / alpha) << 18;
    const int32_t *ibuf = inbufs[0];
    int32_t *obuf = outbufs[0];
    int32_t x0 = x[0];
    int32_t x1 = x[1];
    int32_t x2 = x[2];
    int32_t x3 = x[3];
#if defined(NONLINEARITY)
    int32_t w0 = w[0];
    int32_t w1 = w[1];
    int32_t w2 = w[2];
    int32_t w3 = w[3];
    int32_t yy0 = yy;
#endif
    for(int i = 0; i < n; i++) 
    {
        alpha += delta_alpha;
        k += delta_k;
        int32_t signal = ibuf[i];
#if defined(NONLINEARITY) 
        int32_t fb = ((int64_t)k * (int64_t)(x3 + yy0)) >> 25;
        yy0 = x3;
        int32_t rx = signal - fb;
        int32_t trx = Tanh::lookup(rx);
        x0 = x0 + ((((int64_t)(trx - w0) * (int64_t)alpha)) >> 24);
        w0 = Tanh::lookup(x0);
        x1 = x1 + ((((int64_t)(w0 - w1) * (int64_t)alpha)) >> 24);
        w1 = Tanh::lookup(x1);
        x2 = x2 + ((((int64_t)(w1 - w2) * (int64_t)alpha)) >> 24);
        w2 = Tanh::lookup(x2);
        x3 = x3 + ((((int64_t)(w2 - w3) * (int64_t)alpha)) >> 24);
        w3 = Tanh::lookup(x3);
#else
        int32_t fb = ((int64_t)k * (int64_t)x3) >> 24;
        x0 = x0 + ((((int64_t)(signal - fb - x0) * (int64_t)alpha)) >> 24);
        x1 = x1 + ((((int64_t)(x0 - x1) * (int64_t)alpha)) >> 24);
        x2 = x2 + ((((int64_t)(x1 - x2) * (int64_t)alpha)) >> 24);
        x3 = x3 + ((((int64_t)(x2 - x3) * (int64_t)alpha)) >> 24);
#endif
        obuf[i] = x3;
    }
    x[0] = x0;
    x[1] = x1;
    x[2] = x2;
    x[3] = x3;
#if defined(NONLINEARITY)
    w[0] = w0;
    w[1] = w1;
    w[2] = w2;
    w[3] = w3;
    yy = yy0;
#endif
}
#endif  // USE_MATRIX
