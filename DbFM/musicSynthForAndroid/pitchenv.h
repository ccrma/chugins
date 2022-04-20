#ifndef pitchenv_h
#define pitchenv_h
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

// Computation of the DX7 pitch envelope

class PitchEnv 
{
public:
    static void init(double sample_rate);

    // The rates and levels arrays are calibrated to match the Dx7 parameters
    // (ie, value 0..99).
    void set(const int rates[4], const int levels[4]);

    int32_t getsample(); // Result is in Q24/octave

    void keydown(bool down);

 private:
    static int s_unit;
    int m_rates[4];
    int m_levels[4];
    int32_t m_level;
    int m_targetlevel;
    bool m_rising;
    int m_ix;
    int m_inc;
    bool m_down;

    void advance(int newix);
};

#endif  
