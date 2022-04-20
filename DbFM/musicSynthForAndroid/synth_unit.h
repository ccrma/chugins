#ifndef synth_unit_h
#define synth_unit_h
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

#include "controllers.h"
#include "dx7note.h"
#include "lfo.h"
#include "ringbuffer.h"
#include "resofilter.h"

struct ActiveNote  // aka "Voice"
{
    int midi_note;
    bool keydown;
    bool sustained;
    bool live;
    Dx7Note *dx7_note;
};

class SynthUnit 
{
public:
    static void Init(double sample_rate);

    explicit SynthUnit(RingBuffer *ring_buffer);
    void GetSamples(int n_samples, int16_t *buffer);

private:
    void TransferInput();
    void ConsumeInput(int n_input_bytes);

    // Choose a note for a new key-down, returns note number, or -1 if
    // none available.
    int AllocateNote();

    // zero-based
    void ProgramChange(int p);

    void SetController(int controller, int value);

    int ProcessMidiMessage(const uint8_t *buf, int buf_size);

    RingBuffer *m_ring_buffer;
    static const int max_active_notes = 16;
    ActiveNote m_active_note[max_active_notes];
    int m_current_note;
    uint8_t m_input_buffer[8192];
    size_t m_input_buffer_index;

    uint8_t m_patch_data[4096];
    int m_current_patch;
    char m_unpacked_patch[156];

    // The original DX7 had one single LFO. Later units had an LFO per note.
    Lfo m_lfo;

    // for pitchbend: in MIDI units (0x4000 is neutral)
    Controllers m_controllers;
    
    ResoFilter m_filter;
    int32_t m_filter_control[3];
    bool m_sustain;

    // Extra buffering for when GetSamples wants a buffer not a multiple of N
    int16_t m_extra_buf[k_RenderChunkSize];
    int m_extra_buf_size;
};

#endif