#ifndef fm_core_h
#define fm_core_h

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
#include "aligned_buf.h"

struct FmOpParams
{
    int32_t gain[2]; // Q24
    int32_t freq;
    int32_t phase;
};

class FmCore 
{
public:
    static void dump();
    void compute(int32_t *output, FmOpParams *params, int algorithm,
                 int32_t *fb_buf, int32_t feedback_gain);
private:
    AlignedBuf<int32_t, k_MaxSamples> m_buf[2];
};

#endif  // __FM_CORE_H
