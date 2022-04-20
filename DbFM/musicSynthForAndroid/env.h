#ifndef env_h
#define env_h
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

// DX7 envelope generation

class Env 
{
public:
    // The rates and levels arrays are calibrated to match the Dx7 parameters
    // (ie, value 0..99). The outlevel parameter is calibrated in microsteps
    // (ie units of approx .023 dB), with 99 * 32 = nominal full scale. The
    // rate_scaling parameter is in qRate units (ie 0..63).
    void init(const int rates[4], const int levels[4], int outlevel,
                int rate_scaling);

    // Result is in Q24/doubling log format. Also, result is subsampled
    // for every N samples.  A couple more things need to happen for 
    // this to be used as a gain value. First, the # of outputs scaling 
    // needs to be applied. Also, modulation. Then, of course, log to linear.
    // Format assumed/required by Exp2::lookup.
    int32_t getsample();

    void keydown(bool down);
    void setparam(int param, int value);
    static int scaleoutlevel(int outlevel);

private:
    int m_rates[4];
    int m_levels[4];
    int m_outlevel;
    int m_rate_scaling;

    // m_level is stored so that 2^24 is one doubling, ie 16 more bits than
    // the DX7 itself (fraction is stored in level rather than separate
    // counter)
    int32_t m_level;
    int m_targetlevel;
    bool m_rising;
    int m_ix;
    int m_inc;

    bool m_down;

    void advance(int newix);
};

#endif  // env_h
