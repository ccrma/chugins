/*
 *  mdaTalkboxProcessor.cpp
 *
 *  mda chugin
 *  Created by Dana Batali around 4/22
 *
 *  mda VST Plug-ins
 *  Created by Arne Scheffler on 6/14/08.
 *
 *  mda plugins
 *  Copyright (c) 2008 Paul Kellett
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "mdaTalkboxProcessor.h"

#include <cmath>
#include <iostream>
#include <cassert>

#define BUF_MAX         1600
#define ORD_MAX           50
#define TWO_PI     6.2831853f /* float has < 8 digits of precision, (double:15) */

ParamDef TalkboxProcessor::s_paramDefs[kNumParams] = 
{
    ParamDef("Wet", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kWet),
    ParamDef("Dry", nullptr, .0, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kDry),
    ParamDef("Quality", nullptr, 1, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kQuality),
};

//-----------------------------------------------------------------------------
TalkboxProcessor::TalkboxProcessor(double sampleRate) :
    BaseProcessor(sampleRate, kNumParams, s_paramDefs)
{
    m_params[0].value = 0.5f; // wet
    m_params[1].value = 0.0f; // dry
    m_params[2].value = 1.0f; // quality

    buf0 = new float[BUF_MAX];
    buf1 = new float[BUF_MAX];
    window = new float[BUF_MAX];
    car0 = new float[BUF_MAX];
    car1 = new float[BUF_MAX];
    N = 1; //trigger window recalc
    K = 0;

    pos = K = 0;
    emphasis = 0.0f;
    FX = 0;

    u0 = u1 = u2 = u3 = u4 = 0.0f;
    d0 = d1 = d2 = d3 = d4 = 0.0f;

    memset(buf0, 0, BUF_MAX * sizeof(float));
    memset(buf1, 0, BUF_MAX * sizeof(float));
    memset(car0, 0, BUF_MAX * sizeof(float));
    memset(car1, 0, BUF_MAX * sizeof(float));
    memset(window, 0, BUF_MAX * sizeof(float));
}

//-----------------------------------------------------------------------------
TalkboxProcessor::~TalkboxProcessor()
{
	delete [] buf0;
	delete [] buf1;
	delete [] window;
	delete [] car0;
	delete [] car1;
	buf0=buf1=window=car0=car1=nullptr;
}

//-----------------------------------------------------------------------------
void TalkboxProcessor::multiProcessing(
    float *in, int inCh, float *out, int outCh, int sampleFrames)
{
    assert(sampleFrames == 1 && inCh == 2);

	float* in1 = in;
	float* in2 = in+1;
	float* out1 = out;
	float* out2 = (outCh == 1) ? out : out+1;

	int  p0=pos, p1 = (pos + N/2) % N;
	float e=emphasis, w, o, x, dr, fx=FX;
	float p, q, h0=0.3f, h1=0.77f;

	--in1;
	--in2;
	--out1;
	--out2;
	while(--sampleFrames >= 0)
	{
		o = *++in1;
		x = *++in2;
		dr = o;

		p = d0 + h0 *  x; d0 = d1;  d1 = x  - h0 * p;
		q = d2 + h1 * d4; d2 = d3;  d3 = d4 - h1 * q;  
		d4 = x;
		x = p + q;

		if(K++) // even/odd
		{
			K = 0;

			car0[p0] = car1[p1] = x; //carrier input

			x = o - e;  e = o;  //6dB/oct pre-emphasis

            // 50% overlapping hanning windows
			w = window[p0]; 
            fx = buf0[p0] * w;  
            buf0[p0] = x * w;  
			if(++p0 >= N) 
            { 
                lpc(buf0, car0, N, O);  
                p0 = 0; 
            }

			w = 1.0f - w;  
            fx += buf1[p1] * w;  
            buf1[p1] = x * w;
			if(++p1 >= N) 
            { 
                lpc(buf1, car1, N, O);  
                p1 = 0; 
            }
		}

		p = u0 + h0 * fx; u0 = u1;  u1 = fx - h0 * p;
		q = u2 + h1 * u4; u2 = u3;  u3 = u4 - h1 * q;  
		u4 = fx;
		x = p + q;

		o = wet * x + dry * dr;
		*++out1 = o; // we're really mono
		*++out2 = o;
	}
	emphasis = e;
	pos = p0;
	FX = fx;

	float den = 1.0e-10f; //(float)pow (10.0f, -10.0f * params[4]);
	if(fabs(d0) < den) d0 = 0.0f; //anti-denormal (doesn't seem necessary but P4?)
	if(fabs(d1) < den) d1 = 0.0f;
	if(fabs(d2) < den) d2 = 0.0f;
	if(fabs(d3) < den) d3 = 0.0f;
	if(fabs(u0) < den) u0 = 0.0f;
	if(fabs(u1) < den) u1 = 0.0f;
	if(fabs(u2) < den) u2 = 0.0f;
	if(fabs(u3) < den) u3 = 0.0f;
}

//-----------------------------------------------------------------------------
void 
TalkboxProcessor::processParamChanges()
{
    if(!this->m_paramsDirty) return;

	float fs = (float)this->m_sampleRate;
	if (fs <  8000.0f) fs =  8000.0f;
	if (fs > 96000.0f) fs = 96000.0f;

	int n = (int)(0.01633f * fs);
	if (n > BUF_MAX) n = BUF_MAX;

	//O = (int32)(0.0005f * fs);
	O = (int)((0.0001f + 0.0004f * m_params[kQuality].value) * fs);

	if (n != N) //recalc hanning window
	{
		N = n;
		float dp = TWO_PI / (float)N;
		float p = 0.0f;
		for(n=0; n<N; n++)
		{
			window[n] = 0.5f - 0.5f * cosf(p);
			p += dp;
		}
	}
	wet = 0.5f * m_params[kWet].value * m_params[kWet].value;
	dry = 2.0f * m_params[1].value * m_params[1].value;
}

// https://web.ece.ucsb.edu/Faculty/Rabiner/ece259/digital%20speech%20processing%20course/lectures_new/Lecture%2013_winter_2012.pdf
void 
TalkboxProcessor::lpc(float *buf, float *car, int n, int o)
{
    float z[ORD_MAX], r[ORD_MAX], k[ORD_MAX], G, x;
    r[0] = 0.f;
    int i, j, nn=n;

    for(j=0; j<=o; j++, nn--)  //buf[] is already emphasized and windowed
    {
        z[j] = r[j] = 0.0f;
        for(i = 0; i<nn; i++) 
            r[j] += buf[i] * buf[i+j]; //autocorrelation
    }
    r[0] *= 1.001f;  //stability fix

    float min = 0.00001f;
    if(r[0] < min) 
    { 
        for(i = 0; i<n; i++) 
            buf[i] = 0.0f; 
        return;
    } 

    this->lpc_durbin(r, o, k, &G);  //calc reflection coeffs

    for(i = 0; i<=o; i++) 
    {
        if(k[i] > 0.995f) 
            k[i] = 0.995f; 
        else 
        if(k[i] < -0.995f) 
            k[i] = -.995f;
    }
  
    for(i = 0; i<n; i++)
    {
        x = G * car[i];
        for(j=o; j>0; j--)  //lattice filter
        { 
            x -= k[j] * z[j-1];
            z[j] = z[j-1] + k[j] * x;
        }
        buf[i] = z[0] = x;  //output buf[] will be windowed elsewhere
    }
}

void 
TalkboxProcessor::lpc_durbin(float *r, int p, float *k, float *g)
{
    int i, j;
    float a[ORD_MAX], at[ORD_MAX], e=r[0];
    
    for(i = 0; i <= p; i++) 
        a[i] = at[i] = 0.0f; //probably don't need to clear at[] or k[]

    for(i = 1; i <= p; i++) 
    {
        k[i] = -r[i];

        for(j = 1; j < i; j++) 
        { 
            at[j] = a[j];
            k[i] -= a[j] * r[i-j]; 
        }
        if(fabs(e) < 1.0e-20f) 
        { 
            e = 0.0f;  
            break; 
        }
        k[i] /= e;
        a[i] = k[i];
        for(j = 1; j < i; j++) 
            a[j] = at[j] + k[i] * at[i-j];
    
        e *= 1.0f - k[i] * k[i];
    }

    if (e < 1.0e-20f)
        e = 0.0f;
    *g = sqrtf(e);
}

