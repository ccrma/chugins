#ifndef aligned_buf_h
#define aligned_buf_h
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

// A convenient wrapper for buffers with alignment constraints.
// Such constraints are useful in SIMD/SSE settings.

template<typename T, size_t size, size_t alignment = 16>
class AlignedBuf 
{
public:
    T *get() 
    {
        return (T *)((((intptr_t)m_storage) + alignment - 1) & ~alignment);
    }
private:
    unsigned char m_storage[size * sizeof(T) + alignment];
};

#endif  // aligned_buf_h
