#ifndef synth_h
#define synth_h
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

typedef float t_sample;

#define k_NumFMOps 6 /* number in a true DX7 is 6 */

//#define LG_N 6
////#define N (1 << LG_N) /* 64, maxsamples per invocation */

#define k_LogChunkSize 6
#define k_RenderChunkSize (1 << k_LogChunkSize)

#endif 
