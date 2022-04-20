#ifndef dx7note_h
#define dx7note_h
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

// This is the logic to put together a note from the MIDI description
// and run the low-level modules.

// It will continue to evolve a bit, as note-stealing logic, scaling,
// and real-time control of parameters live here.

#include "env.h"
#include "pitchenv.h"
#include "fm_core.h"
#include "synth.h"

class Dx7Note 
{
public:
    void init(const char patch[156], int midinote, int velocity);

    // NB: compute _adds_ to the buf. 
    // Interesting question whether it's worth it...
    void compute(int32_t *buf, int32_t lfo_val, int32_t lfo_delay,
        const Controllers *ctrls);

    void keyup();

    // TODO: parameter changes

    // TODO: some way of indicating end-of-note. Maybe should be a return
    // value from the compute method? (Having a count return from keyup
    // is also tempting, but if there's a dynamic parameter change after
    // keyup, that won't work.

 private:
    FmCore m_core;
    Env m_env[k_NumFMOps];
    FmOpParams m_params[k_NumFMOps];
    PitchEnv m_pitchenv;
    int32_t m_basepitch[k_NumFMOps];
    int32_t m_fb_buf[2];
    int32_t m_fb_shift;

    int m_algorithm;
    int m_pitchmoddepth;
    int m_pitchmodsens;
};

#endif  // dx7note_h
