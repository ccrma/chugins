#ifndef ringbuffer_h
#define ringbuffer_h
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

class RingBuffer 
{
public:
    RingBuffer();

    // Returns number of bytes available for reading.
    int BytesAvailable();

    // Returns number of bytes that can be written without blocking.
    int WriteBytesAvailable();

    // Reads bytes. It is the caller's responsibility to make sure that
    // size <= a previous value of BytesAvailable().
    int Read(unsigned size, uint8_t *bytes);

    // Writes bytes into the buffer. If the buffer is full, the method will
    // block until space is available.
    void Write(const uint8_t *bytes, int size);

private:
    static const unsigned int kBufSize = 65536;
    uint8_t m_buf[kBufSize];
    volatile unsigned int m_rd_ix;
    volatile unsigned int m_wr_ix;
};

#endif
