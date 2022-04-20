#ifndef lfo_h
#define lfo_h
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

// Low frequency oscillator, compatible with DX7
// operates in fixed-point.

#include <cstdint>

class Lfo 
{
public:
    static void init(double sample_rate);

    void reset(const char params[6]);

    int32_t getsample(); // result is 0..1 in Q24

    int32_t getdelay(); // result is 0..1 in Q24

    void keydown();

    enum Waveform
    {
        k_Triangle,
        k_SawDown,
        k_SawUp,
        k_Square,
        k_Sine,
        k_SampleAndHold
    };

 private:
    static uint32_t s_unit;

    uint32_t m_phase;  // Q32
    uint32_t m_delta;
    Waveform m_waveform;
    uint8_t m_randstate;
    bool m_sync;

    uint32_t m_delaystate;
    uint32_t m_delayinc;
    uint32_t m_delayinc2;
};

#endif