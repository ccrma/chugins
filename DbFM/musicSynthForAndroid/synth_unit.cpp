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
#include "sin.h"
#include "exp2.h"
#include "pitchenv.h"
#include "patch.h"
#include "synth_unit.h"
#include "aligned_buf.h"

#include <iostream>
#include <cstring>
#include <algorithm>

static char s_epiano[] = 
{
    95, 29, 20, 50, 99, 95, 0, 0, 41, 0, 19, 0, 115, 24, 79, 2, 0,
    95, 20, 20, 50, 99, 95, 0, 0, 0, 0, 0, 0, 3, 0, 99, 2, 0,
    95, 29, 20, 50, 99, 95, 0, 0, 0, 0, 0, 0, 59, 24, 89, 2, 0,
    95, 20, 20, 50, 99, 95, 0, 0, 0, 0, 0, 0, 59, 8, 99, 2, 0,
    95, 50, 35, 78, 99, 75, 0, 0, 0, 0, 0, 0, 59, 28, 58, 28, 0,
    96, 25, 25, 67, 99, 75, 0, 0, 0, 0, 0, 0, 83, 8, 99, 2, 0,
  
    94, 67, 95, 60, 50, 50, 50, 50, 4, 6, 34, 33, 0, 0, 56, 24,
    69, 46, 80, 73, 65, 78, 79, 32, 49, 32
};

void SynthUnit::Init(double sample_rate) 
{
    Freqlut::init(sample_rate);
    Exp2::init();
    Tanh::init();
    Sin::init();
    Lfo::init(sample_rate);
    PitchEnv::init(sample_rate);
}

SynthUnit::SynthUnit(RingBuffer *ring_buffer) 
{
    m_ring_buffer = ring_buffer;
    for(int note = 0; note < max_active_notes; ++note) 
    {
        m_active_note[note].dx7_note = new Dx7Note;
        m_active_note[note].keydown = false;
        m_active_note[note].sustained = false;
        m_active_note[note].live = false;
    }
    m_input_buffer_index = 0;
    memcpy(m_patch_data, s_epiano, sizeof(s_epiano));
    ProgramChange(0);
    m_current_note = 0;
    m_filter_control[0] = 258847126;
    m_filter_control[1] = 0;
    m_filter_control[2] = 0;
    m_controllers.m_values[kControllerPitch] = 0x2000;
    m_sustain = false;
    m_extra_buf_size = 0;
}

// Transfer as many bytes as possible from ring buffer to input buffer.
// Note that this implementation has a fair amount of copying - we'd probably
// do it a bit differently if it were bulk data, but in this case we're
// optimizing for simplicity of implementation.
void 
SynthUnit::TransferInput() 
{
    size_t bytes_available = m_ring_buffer->BytesAvailable();
    int bytes_to_read = std::min(bytes_available,
                            sizeof(m_input_buffer) - m_input_buffer_index);
    if (bytes_to_read > 0) 
    {
        m_ring_buffer->Read(bytes_to_read, m_input_buffer + m_input_buffer_index);
        m_input_buffer_index += bytes_to_read;
    }
}

void 
SynthUnit::ConsumeInput(int n_input_bytes) 
{
    if(n_input_bytes < m_input_buffer_index) 
    {
        memmove(m_input_buffer, m_input_buffer + n_input_bytes,
                m_input_buffer_index - n_input_bytes);
    }
    m_input_buffer_index -= n_input_bytes;
}

int 
SynthUnit::AllocateNote() 
{
    int note = m_current_note;
    for(int i = 0; i < max_active_notes; i++) 
    {
        if(!m_active_note[note].keydown) 
        {
            m_current_note = (note + 1) % max_active_notes;
            return note;
        }
        note = (note + 1) % max_active_notes;
    }
    return -1;
}

void 
SynthUnit::ProgramChange(int p) 
{
    m_current_patch = p;
    const uint8_t *patch = m_patch_data + 128 * m_current_patch;
    UnpackPatch((const char *)patch, m_unpacked_patch);
    m_lfo.reset(m_unpacked_patch + 137);
}

void 
SynthUnit::SetController(int controller, int value) 
{
    m_controllers.m_values[controller] = value;
}

int 
SynthUnit::ProcessMidiMessage(const uint8_t *buf, int buf_size) 
{
    uint8_t cmd = buf[0];
    uint8_t cmd_type = cmd & 0xf0;
    //LOGI("got %d midi: %02x %02x %02x", buf_size, buf[0], buf[1], buf[2]);
    if(cmd_type == 0x80 || (cmd_type == 0x90 && buf[2] == 0)) 
    {
        if(buf_size >= 3) 
        {
            // note off
            for(int note = 0; note < max_active_notes; ++note) 
            {
                if(m_active_note[note].midi_note == buf[1] && 
                    m_active_note[note].keydown) 
                {
                    if(m_sustain) 
                    {
                        m_active_note[note].sustained = true;
                    } 
                    else 
                    {
                        m_active_note[note].dx7_note->keyup();
                    }
                    m_active_note[note].keydown = false;
                }
            }
            return 3;
        }
        return 0;
    } 
    else 
    if(cmd_type == 0x90) 
    {
        if(buf_size >= 3) 
        {
            // note on
            int note_ix = AllocateNote();
            if (note_ix >= 0) 
            {
                m_lfo.keydown();  // TODO: should only do this if # keys down was 0
                m_active_note[note_ix].midi_note = buf[1];
                m_active_note[note_ix].keydown = true;
                m_active_note[note_ix].sustained = m_sustain;
                m_active_note[note_ix].live = true;
                m_active_note[note_ix].dx7_note->init(m_unpacked_patch, buf[1], buf[2]);
            }
            return 3;
        }
        return 0;
    } 
    else 
    if(cmd_type == 0xb0) 
    {
        if(buf_size >= 3) 
        {
            // controller
            // TODO: move more logic into SetController
            int controller = buf[1];
            int value = buf[2];
            if(controller == 1) 
            {
                m_filter_control[0] = 142365917 + value * 917175;
            } 
            else 
            if(controller == 2) 
            {
                m_filter_control[1] = value * 528416;
            } 
            else 
            if(controller == 3) 
            {
                m_filter_control[2] = value * 528416;
            } 
            else 
            if(controller == 64) 
            {
                m_sustain = value != 0;
                if(!m_sustain) 
                {
                    for(int note = 0; note < max_active_notes; note++) 
                    {
                        if(m_active_note[note].sustained && 
                          !m_active_note[note].keydown) 
                        {
                            m_active_note[note].dx7_note->keyup();
                            m_active_note[note].sustained = false;
                        }
                    }
                }
            }
            return 3;
        } 
        return 0;
    } 
    else 
    if(cmd_type == 0xc0) // program change
    {
        if(buf_size >= 2) 
        {
            int program_number = buf[1];
            ProgramChange(std::min(program_number, 31));
            char name[11];
            memcpy(name, m_unpacked_patch + 145, 10);
            name[10] = 0;
#ifdef VERBOSE
            std::cout << "Loaded patch " << current_patch_ << ": " << name << "\r";
            std::cout.flush();
#endif
            return 2;
        }
        return 0;
    } 
    else 
    if(cmd == 0xe0) // pitch bend
    {
        SetController(kControllerPitch, buf[1] | (buf[2] << 7));
        return 3;
    } 
    else 
    if(cmd == 0xf0) // sysex
    {
        if(buf_size >= 6 && buf[1] == 0x43 && buf[2] == 0x00 && 
           buf[3] == 0x09 && buf[4] == 0x20 && buf[5] == 0x00) 
        {
            if(buf_size >= 4104) 
            {
                // TODO: check checksum?
                memcpy(m_patch_data, buf + 6, 4096);
                ProgramChange(m_current_patch);
                return 4104;
            }
            return 0;
        }
    }

    // TODO: more robust handling
#ifdef VERBOSE
    std::cout << "Unknown message " << std::hex << (int)cmd <<
        ", skipping " << std::dec << buf_size << " bytes" << std::endl;
#endif
    return buf_size;
} // end ProcessMidiData

void 
SynthUnit::GetSamples(int n_samples, int16_t *buffer) 
{
    TransferInput();
    size_t input_offset;
    for(input_offset = 0; input_offset < m_input_buffer_index; ) 
    {
        int bytes_available = m_input_buffer_index - input_offset;
        int bytes_consumed = ProcessMidiMessage(m_input_buffer + input_offset,
                                                bytes_available);
        if(bytes_consumed == 0) 
            break;
        input_offset += bytes_consumed;
    }
    ConsumeInput(input_offset);

    int i;
    for(i = 0; i < n_samples && i < m_extra_buf_size; i++) 
    {
        buffer[i] = m_extra_buf[i];
    }
    if(m_extra_buf_size > n_samples) 
    {
        for(int j = 0; j < m_extra_buf_size - n_samples; j++) 
        {
            m_extra_buf[j] = m_extra_buf[j + n_samples];
        }
        m_extra_buf_size -= n_samples;
        return;
    }

    for (; i < n_samples; i += N) 
    {
        AlignedBuf<int32_t, N> audiobuf;
        AlignedBuf<int32_t, N> audiobuf2;
        for(int j = 0; j < N; ++j) 
        {
            audiobuf.get()[j] = 0;
        }
        int32_t lfovalue = m_lfo.getsample();
        int32_t lfodelay = m_lfo.getdelay();
        for(int note = 0; note < max_active_notes; ++note) 
        {
            if(m_active_note[note].live) 
            {
                m_active_note[note].dx7_note->compute(audiobuf.get(), 
                                lfovalue, lfodelay, &m_controllers);
            }
        }
        const int32_t *bufs[] = { audiobuf.get() };
        int32_t *bufs2[] = { audiobuf2.get() };
        m_filter.process(bufs, m_filter_control, m_filter_control, bufs2);
        int jmax = n_samples - i;
        for (int j = 0; j < k_MaxSamples; ++j) 
        {
            int32_t val = audiobuf2.get()[j] >> 4;
            int clip_val = val < -(1 << 24) ? 0x8000 : val >= (1 << 24) ? 0x7fff :
            val >> 9;
            // TODO: maybe some dithering?
            if (j < jmax) 
            {
                buffer[i + j] = clip_val;
            } 
            else 
            {
                m_extra_buf[j - jmax] = clip_val;
            }
        }
    }
    m_extra_buf_size = i - n_samples;
}
