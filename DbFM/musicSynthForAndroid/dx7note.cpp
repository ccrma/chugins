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
#include "freqlut.h"
#include "patch.h"
#include "exp2.h"
#include "controllers.h"
#include "dx7note.h"

#include <iostream>
#include <cmath>
#include <algorithm> // min, max

static int32_t 
midinote_to_logfreq(int midinote) 
{
    const int base = 50857777;  // (1 << 24) * (log(440) / log(2) - 69/12)
    const int step = (1 << 24) / 12;
    return base + step * midinote;
}

static int32_t 
osc_freq(int midinote, int mode, int coarse, int fine, int detune)
{
    static const int32_t coarsemul[32] =  
    {
        -16777216, 0, 16777216, 26591258, 33554432, 38955489, 43368474, 47099600,
        50331648, 53182516, 55732705, 58039632, 60145690, 62083076, 63876816,
        65546747, 67108864, 68576247, 69959732, 71268397, 72509921, 73690858,
        74816848, 75892776, 76922906, 77910978, 78860292, 79773775, 80654032,
        81503396, 82323963, 83117622
    };

    // TODO: pitch randomization
    int32_t logfreq;
    if (mode == 0)
    {
        logfreq = midinote_to_logfreq(midinote);
        logfreq += coarsemul[coarse & 31];
        if (fine) 
        {
            // (1 << 24) / log(2)
            logfreq += (int32_t)floor(24204406.323123 * log(1 + 0.01 * fine) + 0.5);
        }
        // This was measured at 7.213Hz per count at 9600Hz, but the exact
        // value is somewhat dependent on midinote. Close enough for now.
        logfreq += 12606 * (detune - 7);
    } 
    else 
    {
        // ((1 << 24) * log(10) / log(2) * .01) << 3
        logfreq = (4458616 * ((coarse & 3) * 100 + fine)) >> 3;
        logfreq += detune > 7 ? 13457 * (detune - 7) : 0;
    }
    return logfreq;
}

// See "velocity" section of notes. Returns velocity delta in microsteps.
static int 
ScaleVelocity(int velocity, int sensitivity) 
{
    static const uint8_t velocity_data[64] = 
    {
        0, 70, 86, 97, 106, 114, 121, 126, 132, 138, 142, 148, 152, 156, 160, 163,
        166, 170, 173, 174, 178, 181, 184, 186, 189, 190, 194, 196, 198, 200, 202,
        205, 206, 209, 211, 214, 216, 218, 220, 222, 224, 225, 227, 229, 230, 232,
        233, 235, 237, 238, 240, 241, 242, 243, 244, 246, 246, 248, 249, 250, 251,
        252, 253, 254
    };

    int clamped_vel = std::max(0, std::min(127, velocity));
    int vel_value = velocity_data[clamped_vel >> 1] - 239;
    int scaled_vel = ((sensitivity * vel_value + 7) >> 3) << 4;
    return scaled_vel;
}

static int
ScaleRate(int midinote, int sensitivity) 
{
    int x = std::min(31, std::max(0, midinote / 3 - 7));
    int qratedelta = (sensitivity * x) >> 3;
#ifdef SUPER_PRECISE
    int rem = x & 7;
    if (sensitivity == 3 && rem == 3) 
    {
        qratedelta -= 1;
    } 
    else 
    if (sensitivity == 7 && rem > 0 && rem < 4) 
    {
        qratedelta += 1;
    }
#endif
  return qratedelta;
}

static int 
ScaleCurve(int group, int depth, int curve)
{
    static const uint8_t exp_scale_data[] = 
    {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 14, 16, 19, 
        23, 27, 33, 39, 47, 56, 66, 80, 94, 110, 126, 
        142, 158, 174, 190, 206, 222, 238, 250
    };

    int scale;
    if (curve == 0 || curve == 3) 
    {
        // linear
        scale = (group * depth * 329) >> 12;
    } 
    else 
    {
        // exponential
        int n_scale_data = sizeof(exp_scale_data);
        int raw_exp = exp_scale_data[std::min(group, n_scale_data - 1)];
        scale = (raw_exp * depth * 329) >> 15;
    }
    if (curve < 2) 
    {
        scale = -scale;
    }
    return scale;
}

static int 
ScaleLevel(int midinote, int break_pt, int left_depth, int right_depth,
            int left_curve, int right_curve) 
{
    int offset = midinote - break_pt - 17;
    if (offset >= 0)
        return ScaleCurve(offset / 3, right_depth, right_curve);
    else
        return ScaleCurve((-offset) / 3, left_depth, left_curve);
}

static const uint8_t s_pitchmodsenstab[] = 
{
    0, 10, 20, 33, 55, 92, 153, 255
};

/* ----------------------------------------------------------------------- */

void 
Dx7Note::init(const char patch[156], int midinote, int velocity) 
{
    int rates[4];
    int levels[4];
    for(int op = 0; op < 6; op++) 
    {
        int off = op * 21;
        for(int i = 0; i < 4; i++) 
        {
            rates[i] = patch[off + i];
            levels[i] = patch[off + 4 + i];
        }
        int outlevel = patch[off + 16];
        outlevel = Env::scaleoutlevel(outlevel);
#ifdef VERBOSE
        for(int j = 8; j < 12; j++) 
            cout << (int)patch[off + j] << " ";
#endif
        int level_scaling = ScaleLevel(midinote, 
                                patch[off + 8], patch[off + 9],
                                patch[off + 10], patch[off + 11], 
                                patch[off + 12]);
        outlevel += level_scaling;
        outlevel = std::min(127, outlevel);
#ifdef VERBOSE
        cout << op << ": " std::<< level_scaling << " " << outlevel << endl;
#endif
        outlevel = outlevel << 5;
        outlevel += ScaleVelocity(velocity, patch[off + 15]);
        outlevel = std::max(0, outlevel);
        int rate_scaling = ScaleRate(midinote, patch[off + 13]);
        m_env[op].init(rates, levels, outlevel, rate_scaling);

        int mode = patch[off + 17];
        int coarse = patch[off + 18];
        int fine = patch[off + 19];
        int detune = patch[off + 20];
        int32_t freq = osc_freq(midinote, mode, coarse, fine, detune);
        m_basepitch[op] = freq;
        // cout << op << " freq: " << freq << endl;
        m_params[op].phase = 0;
        m_params[op].gain[1] = 0;
    } // foreach op
    for(int i = 0; i < 4; i++) 
    {
        rates[i] = patch[126 + i];
        levels[i] = patch[130 + i];
    }
    m_pitchenv.set(rates, levels);
    m_algorithm = patch[134];
    int feedback = patch[135];
    m_fb_shift = feedback != 0 ? 8 - feedback : 16;
    m_pitchmoddepth = (patch[139] * 165) >> 6;
    m_pitchmodsens = s_pitchmodsenstab[patch[143] & 7];
}

void 
Dx7Note::compute(int32_t *buf, int32_t lfo_val, int32_t lfo_delay,
                const Controllers *ctrls) 
{
    t_sample pitchmod = m_pitchenv.getsample();
    t_sample pmd = m_pitchmoddepth * lfo_delay;  // Q32
    // TODO: add modulation sources (mod wheel, etc)
    float senslfo = m_pitchmodsens * lfo_val;
    pitchmod += pmd * senslfo;

    // hardcodes a pitchbend range of 3 semitones, TODO make configurable
    int pitchbend = ctrls->m_values[kControllerPitch];
    int32_t pb = (pitchbend - 0x2000) << 9;
    pitchmod += pb;
    for(int op = 0; op < k_NumFMOps; op++) 
    {
        m_params[op].gain[0] = m_params[op].gain[1];
        int32_t level = m_env[op].getsample();
        int32_t gain = Exp2::lookup(level - (14 * (1 << 24))); // xxx
        //int32_t gain = pow(2, 10 + level * (1.0 / (1 << 24)));
        m_params[op].freq = Freqlut::lookup(m_basepitch[op] + pitchmod);
        m_params[op].gain[1] = gain;
    }
    m_core.compute(buf, m_params, m_algorithm, m_fb_buf, m_fb_shift);
}

void 
Dx7Note::keyup() 
{
    for(int op = 0; op < k_NumFMOps; op++)
    {
        m_env[op].keydown(false);
        m_pitchenv.keydown(false);
    }
}
