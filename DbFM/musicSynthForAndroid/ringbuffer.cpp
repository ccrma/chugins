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
#include "ringbuffer.h"

#include <time.h>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <thread>

RingBuffer::RingBuffer() 
{
    m_rd_ix = 0;
    m_wr_ix = 0;
}

int 
RingBuffer::BytesAvailable() 
{
    return (m_wr_ix - m_rd_ix) & (kBufSize - 1);
}

int 
RingBuffer::WriteBytesAvailable() 
{
    return (m_rd_ix - m_wr_ix - 1) & (kBufSize - 1);
}

int 
RingBuffer::Read(unsigned size, uint8_t *bytes) 
{
    int rd_ix = m_rd_ix;
    // SynthMemoryBarrier();  // read barrier, make sure data is committed before ix
    unsigned int fragment_size = std::min((unsigned int)size, kBufSize - rd_ix);
    memcpy(bytes, m_buf + rd_ix, fragment_size);
    if(size > fragment_size) {
        memcpy(bytes + fragment_size, m_buf, size - fragment_size);
    }
    // SynthMemoryBarrier();  // full barrier, make sure read commits before updating
    m_rd_ix = (rd_ix + size) & (kBufSize - 1);
    return size;
}

void 
RingBuffer::Write(const uint8_t *bytes, int size) 
{
    unsigned int remaining = (unsigned int)size;
    while(remaining > 0) 
    {
        int rd_ix = m_rd_ix;
        int wr_ix = m_wr_ix;
        unsigned int space_available = (rd_ix - wr_ix - 1) & (kBufSize - 1);
        if (space_available == 0) 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000000));
        } 
        else 
        {
            unsigned int wr_size = std::min(remaining, space_available);
            unsigned int fragment_size = std::min(wr_size, kBufSize - wr_ix);
            memcpy(m_buf + wr_ix, bytes, fragment_size);
            if(wr_size > fragment_size) 
            {
                memcpy(m_buf, bytes + fragment_size, wr_size - fragment_size);
            }
            // SynthMemoryBarrier();  // write barrier, make sure data commits
            m_wr_ix = (wr_ix + wr_size) & (kBufSize - 1);
            remaining -= wr_size;
            bytes += wr_size;
        }
    }
}

