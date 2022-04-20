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

#include "synth.h"
#include "env.h"

#include <algorithm> // min, max

void 
Env::init(const int r[4], const int l[4], int32_t ol, int rate_scaling) 
{
    for(int i = 0; i < 4; i++) 
    {
        m_rates[i] = r[i];
        m_levels[i] = l[i];
    }
    m_outlevel = ol;
    m_rate_scaling = rate_scaling;
    m_level = 0;
    m_down = true;
    advance(0);
}

int32_t
Env::getsample() 
{
    if(m_ix < 3 || (m_ix < 4) && !m_down) 
    {
        if(m_rising) 
        {
            const int jumptarget = 1716;
            if(m_level < (jumptarget << 16)) 
            {
                m_level = jumptarget << 16;
            }
            m_level += (((17 << 24) - m_level) >> 24) * m_inc;
            // TODO: should probably be more accurate when inc is large
            if(m_level >= m_targetlevel) 
            {
                m_level = m_targetlevel;
                advance(m_ix + 1);
            }
        } 
        else 
        {  // !rising
            m_level -= m_inc;
            if(m_level <= m_targetlevel) 
            {
                m_level = m_targetlevel;
                advance(m_ix + 1);
            }
        }
    }
    // TODO: this would be a good place to set level to 0 when under threshold
    return m_level; // result require/expected by Exp2::lookup
}

void 
Env::keydown(bool d) 
{
    if(m_down != d) 
    {
        m_down = d;
        advance(d ? 0 : 3);
    }
}

void 
Env::setparam(int param, int value) 
{
    if(param < 4) 
    {
        m_rates[param] = value;
    } 
    else 
    if(param < 8) 
    {
        m_levels[param - 4] = value;
    }
    // Unknown parameter, ignore for now
}

int 
Env::scaleoutlevel(int outlevel) 
{
    static const int levellut[] = 
    {
        0, 5, 9, 13, 17, 20, 23, 25, 27, 29, 31, 
        33, 35, 37, 39, 41, 42, 43, 45, 46
    };
    return outlevel >= 20 ? 28 + outlevel : levellut[outlevel];
}

void 
Env::advance(int newix)
{
    m_ix = newix;
    if(m_ix < 4) 
    {
        int newlevel = m_levels[m_ix];
        int actuallevel = scaleoutlevel(newlevel) >> 1;
        actuallevel = (actuallevel << 6) + m_outlevel - 4256;
        actuallevel = actuallevel < 16 ? 16 : actuallevel;
        // level here is same as Java impl
        m_targetlevel = actuallevel << 16;
        m_rising = (m_targetlevel > m_level);

        // rate
        int qrate = (m_rates[m_ix] * 41) >> 6;
        qrate += m_rate_scaling;
        qrate = std::min(qrate, 63);
        m_inc = (4 + (qrate & 3)) << (2 + k_LogChunkSize + (qrate >> 2));
    }
}
