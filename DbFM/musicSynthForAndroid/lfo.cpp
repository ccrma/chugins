/*
 * Copyright 2013 Google Inc.
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

// Low frequency oscillator, compatible with DX7
#include "lfo.h"
#include "sin.h"
#include "synth.h"

#include <algorithm> // min, max

uint32_t Lfo::s_unit;

/*static*/ void 
Lfo::init(double sample_rate) 
{
    // constant is 1 << 32 / 15.5s / 11
    Lfo::s_unit = (int32_t)(k_MaxSamples * 25190424 / sample_rate + 0.5);
}

void 
Lfo::reset(const char params[6])
{
    int rate = params[0];  // 0..99
    int sr = rate == 0 ? 1 : (165 * rate) >> 6;
    sr *= sr < 160 ? 11 : (11 + ((sr - 160) >> 4));
    m_delta = s_unit * sr;
    int a = 99 - params[1];  // LFO delay
    if (a == 99) 
    {
        m_delayinc = ~0u;
        m_delayinc2 = ~0u;
    } 
    else 
    {
        a = (16 + (a & 15)) << (1 + (a >> 4));
        m_delayinc = s_unit * a;
        a &= 0xff80;
        a = std::max(0x80, a);
        m_delayinc2 = s_unit * a;
    }
    m_waveform = (Waveform) params[5];
    m_sync = params[4] != 0;
}

int32_t 
Lfo::getsample() 
{
    m_phase += m_delta;
    int32_t x;
    switch(m_waveform) 
    {
    case k_Triangle:
        x = m_phase >> 7;
        x ^= -(m_phase >> 31);
        x &= (1 << 24) - 1;
        return x;
    case k_SawDown:
        return (~m_phase ^ (1U << 31)) >> 8;
    case k_SawUp:
        return (m_phase ^ (1U << 31)) >> 8;
    case k_Square:
        return ((~m_phase) >> 7) & (1 << 24);
    case k_Sine: 
        return (1 << 23) + (Sin::lookup(m_phase >> 8) >> 1);
    case k_SampleAndHold:  
        if(m_phase < m_delta) 
            m_randstate = (m_randstate * 179 + 17) & 0xff;
        x = m_randstate ^ 0x80;
        return (x + 1) << 16;
    }
    return 1 << 23;
}

int32_t 
Lfo::getdelay() 
{
    uint32_t delta = m_delaystate < (1U << 31) ? m_delayinc : m_delayinc2;
    uint32_t d = m_delaystate + delta;
    if(d < m_delayinc) 
        return 1 << 24;
    m_delaystate = d;
    if (d < (1U << 31))
        return 0;
    else 
        return (d >> 7) & ((1 << 24) - 1);
}

void 
Lfo::keydown() 
{
    if(m_sync) 
        m_phase = (1U << 31) - 1; // ie: to 1
    m_delaystate = 0;
}
