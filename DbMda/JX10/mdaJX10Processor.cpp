/*
 *  mdaJX10Processor.cpp
 *  mda-vst3
 *
 *  Created by Arne Scheffler on 6/14/08.
 *
 *  mda VST Plug-ins
 *
 *  Copyright (c) 2008 Paul Kellett
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "mdaJX10Processor.h"

#include <cmath>
#include <iostream>
#include <cassert>

#define SILENCE  0.001f  //voice choking
#define PI       3.1415927f /* floats have < 8 digits of precision */
#define TWOPI    6.2831853f
#define ANALOG   0.002f  //oscillator drift

float JX10Processor::s_programParams[][kNumParams] = 
{ 
	{1.0f, 0.37f, 0.25f, 0.3f, 0.32f, 0.5f, 0.9f, 0.6f, 0.12f, 0.0f, 0.5f, 0.9f, 0.89f, 0.9f, 0.73f, 0.0f, 0.5f, 1.0f, 0.71f, 0.81f, 0.65f, 0.0f, 0.5f, 0.5f},
	{0.88f, 0.51f, 0.5f, 0.0f, 0.49f, 0.5f, 0.46f, 0.76f, 0.69f, 0.1f, 0.69f, 1.0f, 0.86f, 0.76f, 0.57f, 0.3f, 0.8f, 0.68f, 0.66f, 0.79f, 0.13f, 0.25f, 0.45f, 0.5f},
	{0.88f, 0.51f, 0.5f, 0.16f, 0.49f, 0.5f, 0.49f, 0.82f, 0.66f, 0.08f, 0.89f, 0.85f, 0.69f, 0.76f, 0.47f, 0.12f, 0.22f, 0.55f, 0.66f, 0.89f, 0.34f, 0.0f, 1.0f, 0.5f},
	{1.0f, 0.26f, 0.14f, 0.0f, 0.35f, 0.5f, 0.3f, 0.25f, 0.7f, 0.0f, 0.63f, 0.0f, 0.35f, 0.0f, 0.25f, 0.0f, 0.5f, 1.0f, 0.3f, 0.81f, 0.5f, 0.5f, 0.5f, 0.5f},
	{0.41f, 0.5f, 0.79f, 0.0f, 0.08f, 0.32f, 0.49f, 0.01f, 0.34f, 0.0f, 0.93f, 0.61f, 0.87f, 1.0f, 0.93f, 0.11f, 0.48f, 0.98f, 0.32f, 0.81f, 0.5f, 0.0f, 0.5f, 0.5f},
	{0.29f, 0.76f, 0.26f, 0.0f, 0.18f, 0.76f, 0.35f, 0.15f, 0.77f, 0.14f, 0.54f, 0.0f, 0.42f, 0.13f, 0.21f, 0.0f, 0.56f, 0.0f, 0.32f, 0.2f, 0.58f, 0.22f, 0.53f, 0.5f},
	{1.0f, 0.65f, 0.24f, 0.4f, 0.34f, 0.85f, 0.65f, 0.63f, 0.75f, 0.16f, 0.5f, 0.0f, 0.3f, 0.0f, 0.25f, 0.17f, 0.5f, 1.0f, 0.03f, 0.81f, 0.5f, 0.0f, 0.68f, 0.5f},
	{0.0f, 0.25f, 0.5f, 1.0f, 0.46f, 0.5f, 0.51f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.25f, 0.37f, 0.5f, 1.0f, 0.38f, 0.81f, 0.62f, 0.0f, 0.5f, 0.5f},
	{0.84f, 0.51f, 0.15f, 0.45f, 0.41f, 0.42f, 0.54f, 0.01f, 0.58f, 0.21f, 0.67f, 0.0f, 0.09f, 1.0f, 0.25f, 0.2f, 0.85f, 1.0f, 0.3f, 0.83f, 0.09f, 0.4f, 0.49f, 0.5f},
	{0.71f, 0.75f, 0.53f, 0.18f, 0.24f, 1.0f, 0.56f, 0.52f, 0.69f, 0.19f, 0.7f, 1.0f, 0.14f, 0.65f, 0.95f, 0.07f, 0.91f, 1.0f, 0.15f, 0.84f, 0.33f, 0.0f, 0.49f, 0.5f},
	{0.0f, 0.25f, 0.43f, 0.0f, 0.71f, 0.48f, 0.23f, 0.77f, 0.8f, 0.32f, 0.63f, 0.4f, 0.18f, 0.66f, 0.14f, 0.0f, 0.38f, 0.65f, 0.16f, 0.48f, 0.5f, 0.0f, 0.67f, 0.5f},
	{0.62f, 0.26f, 0.51f, 0.79f, 0.35f, 0.54f, 0.64f, 0.39f, 0.51f, 0.65f, 0.0f, 0.07f, 0.52f, 0.24f, 0.84f, 0.13f, 0.3f, 0.76f, 0.21f, 0.58f, 0.3f, 0.0f, 0.36f, 0.5f},
	{0.81f, 1.0f, 0.21f, 0.78f, 0.15f, 0.35f, 0.39f, 0.17f, 0.69f, 0.4f, 0.62f, 0.0f, 0.47f, 0.19f, 0.37f, 0.0f, 0.5f, 0.2f, 0.33f, 0.38f, 0.53f, 0.0f, 0.12f, 0.5f},
	{0.0f, 0.51f, 0.52f, 0.96f, 0.44f, 0.5f, 0.41f, 0.46f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.25f, 0.15f, 0.5f, 1.0f, 0.32f, 0.81f, 0.49f, 0.0f, 0.83f, 0.5f},
	{0.48f, 0.51f, 0.22f, 0.0f, 0.0f, 0.5f, 0.5f, 0.47f, 0.73f, 0.3f, 0.8f, 0.0f, 0.1f, 0.0f, 0.07f, 0.0f, 0.42f, 0.0f, 0.22f, 0.21f, 0.59f, 0.16f, 0.98f, 0.5f},
	{0.0f, 0.51f, 0.5f, 0.83f, 0.49f, 0.5f, 0.55f, 0.75f, 0.69f, 0.35f, 0.5f, 0.0f, 0.56f, 0.0f, 0.56f, 0.0f, 0.8f, 1.0f, 0.24f, 0.26f, 0.49f, 0.0f, 0.07f, 0.5f},
	{0.75f, 0.51f, 0.5f, 0.83f, 0.49f, 0.5f, 0.55f, 0.75f, 0.69f, 0.35f, 0.5f, 0.14f, 0.49f, 0.0f, 0.39f, 0.0f, 0.8f, 1.0f, 0.24f, 0.26f, 0.49f, 0.0f, 0.07f, 0.5f},
	{1.0f, 0.25f, 0.2f, 0.81f, 0.19f, 0.5f, 0.3f, 0.51f, 0.85f, 0.09f, 0.0f, 0.0f, 0.88f, 0.0f, 0.21f, 0.0f, 0.5f, 1.0f, 0.46f, 0.81f, 0.5f, 0.0f, 0.27f, 0.5f},
	{1.0f, 0.25f, 0.2f, 0.72f, 0.19f, 0.86f, 0.48f, 0.43f, 0.94f, 0.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.61f, 1.0f, 0.32f, 0.81f, 0.5f, 0.0f, 0.27f, 0.5f},
	{0.97f, 0.26f, 0.3f, 0.0f, 0.35f, 0.5f, 0.8f, 0.4f, 0.52f, 0.0f, 0.5f, 0.0f, 0.77f, 0.0f, 0.25f, 0.0f, 0.5f, 1.0f, 0.3f, 0.81f, 0.16f, 0.0f, 0.0f, 0.5f},
	{0.0f, 0.25f, 0.5f, 0.65f, 0.35f, 0.5f, 0.33f, 0.76f, 0.53f, 0.0f, 0.5f, 0.0f, 0.3f, 0.0f, 0.25f, 0.0f, 0.55f, 0.25f, 0.3f, 0.81f, 0.52f, 0.0f, 0.14f, 0.5f},
	{1.0f, 0.26f, 0.22f, 0.64f, 0.82f, 0.59f, 0.72f, 0.47f, 0.34f, 0.34f, 0.82f, 0.2f, 0.69f, 1.0f, 0.15f, 0.09f, 0.5f, 1.0f, 0.07f, 0.81f, 0.46f, 0.0f, 0.24f, 0.5f},
	{1.0f, 0.26f, 0.22f, 0.71f, 0.35f, 0.5f, 0.67f, 0.7f, 0.26f, 0.0f, 0.5f, 0.48f, 0.69f, 1.0f, 0.15f, 0.0f, 0.5f, 1.0f, 0.07f, 0.81f, 0.46f, 0.0f, 0.24f, 0.5f},
	{0.49f, 0.25f, 0.66f, 0.81f, 0.35f, 0.5f, 0.36f, 0.15f, 0.75f, 0.2f, 0.5f, 0.0f, 0.38f, 0.0f, 0.25f, 0.0f, 0.6f, 1.0f, 0.22f, 0.19f, 0.5f, 0.0f, 0.17f, 0.5f},
	{0.37f, 0.51f, 0.77f, 0.71f, 0.22f, 0.5f, 0.33f, 0.47f, 0.71f, 0.16f, 0.59f, 0.0f, 0.0f, 0.0f, 0.25f, 0.04f, 0.58f, 0.0f, 0.22f, 0.15f, 0.44f, 0.33f, 0.15f, 0.5f},
	{0.5f, 0.51f, 0.17f, 0.8f, 0.34f, 0.5f, 0.51f, 0.0f, 0.58f, 0.0f, 0.67f, 0.0f, 0.09f, 0.0f, 0.25f, 0.2f, 0.85f, 0.0f, 0.3f, 0.81f, 0.7f, 0.0f, 0.0f, 0.5f},
	{0.23f, 0.51f, 0.38f, 0.0f, 0.35f, 0.5f, 0.33f, 1.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.29f, 0.0f, 0.25f, 0.68f, 0.39f, 0.58f, 0.36f, 0.81f, 0.64f, 0.38f, 0.92f, 0.5f},
	{0.39f, 0.51f, 0.27f, 0.38f, 0.12f, 0.5f, 0.35f, 0.78f, 0.5f, 0.0f, 0.5f, 0.0f, 0.3f, 0.0f, 0.25f, 0.35f, 0.5f, 0.8f, 0.7f, 0.81f, 0.5f, 0.0f, 0.5f, 0.5f},
	{0.0f, 0.25f, 0.5f, 0.0f, 0.35f, 0.5f, 0.23f, 0.2f, 0.75f, 0.0f, 0.5f, 0.0f, 0.22f, 0.0f, 0.25f, 0.0f, 0.47f, 0.0f, 0.3f, 0.81f, 0.5f, 0.8f, 0.5f, 0.5f},
	{1.0f, 0.51f, 0.24f, 0.0f, 0.0f, 0.35f, 0.42f, 0.26f, 0.75f, 0.14f, 0.69f, 0.0f, 0.67f, 0.55f, 0.97f, 0.82f, 0.7f, 1.0f, 0.42f, 0.84f, 0.67f, 0.3f, 0.47f, 0.5f},
	{0.75f, 0.51f, 0.29f, 0.0f, 0.49f, 0.5f, 0.55f, 0.16f, 0.69f, 0.08f, 0.2f, 0.76f, 0.29f, 0.76f, 1.0f, 0.46f, 0.8f, 1.0f, 0.39f, 0.79f, 0.27f, 0.0f, 0.68f, 0.5f},
	{0.0f, 0.5f, 0.53f, 0.0f, 0.13f, 0.39f, 0.38f, 0.74f, 0.54f, 0.2f, 0.0f, 0.0f, 0.55f, 0.52f, 0.31f, 0.0f, 0.17f, 0.73f, 0.28f, 0.87f, 0.24f, 0.0f, 0.29f, 0.5f},
	{0.5f, 0.77f, 0.52f, 0.0f, 0.35f, 0.5f, 0.44f, 0.5f, 0.65f, 0.16f, 0.0f, 0.0f, 0.0f, 0.18f, 0.0f, 0.0f, 0.75f, 0.8f, 0.0f, 0.81f, 0.49f, 0.0f, 0.44f, 0.5f},
	{0.89f, 0.91f, 0.37f, 0.0f, 0.35f, 0.5f, 0.51f, 0.62f, 0.54f, 0.0f, 0.0f, 0.0f, 0.37f, 0.0f, 1.0f, 0.04f, 0.08f, 0.72f, 0.04f, 0.77f, 0.49f, 0.0f, 0.58f, 0.5f},
	{1.0f, 0.51f, 0.51f, 0.37f, 0.0f, 0.5f, 0.51f, 0.1f, 0.5f, 0.11f, 0.5f, 0.0f, 0.0f, 0.0f, 0.25f, 0.35f, 0.65f, 0.65f, 0.32f, 0.79f, 0.49f, 0.2f, 0.35f, 0.5f},
	{0.0f, 0.51f, 0.51f, 0.82f, 0.06f, 0.5f, 0.57f, 0.0f, 0.32f, 0.15f, 0.5f, 0.21f, 0.15f, 0.0f, 0.25f, 0.24f, 0.6f, 0.8f, 0.1f, 0.75f, 0.55f, 0.25f, 0.69f, 0.5f},
	{0.12f, 0.9f, 0.67f, 0.0f, 0.35f, 0.5f, 0.5f, 0.21f, 0.29f, 0.12f, 0.6f, 0.0f, 0.35f, 0.36f, 0.25f, 0.08f, 0.5f, 1.0f, 0.27f, 0.83f, 0.51f, 0.1f, 0.25f, 0.5f},
	{0.43f, 0.76f, 0.23f, 0.0f, 0.28f, 0.36f, 0.5f, 0.0f, 0.59f, 0.0f, 0.5f, 0.24f, 0.16f, 0.91f, 0.08f, 0.17f, 0.5f, 0.8f, 0.45f, 0.81f, 0.5f, 0.0f, 0.58f, 0.5f},
	{0.4f, 0.51f, 0.25f, 0.0f, 0.3f, 0.28f, 0.39f, 0.15f, 0.75f, 0.0f, 0.5f, 0.39f, 0.3f, 0.82f, 0.25f, 0.33f, 0.74f, 0.76f, 0.41f, 0.81f, 0.47f, 0.23f, 0.5f, 0.5f},
	{0.68f, 0.5f, 0.93f, 0.0f, 0.31f, 0.62f, 0.26f, 0.07f, 0.85f, 0.0f, 0.66f, 0.0f, 0.83f, 0.0f, 0.05f, 0.0f, 0.75f, 0.54f, 0.32f, 0.76f, 0.37f, 0.29f, 0.56f, 0.5f},
	{1.0f, 0.27f, 0.22f, 0.0f, 0.35f, 0.5f, 0.82f, 0.13f, 0.75f, 0.0f, 0.0f, 0.24f, 0.3f, 0.88f, 0.34f, 0.0f, 0.5f, 1.0f, 0.48f, 0.71f, 0.37f, 0.0f, 0.35f, 0.5f},
	{0.76f, 0.51f, 0.35f, 0.0f, 0.49f, 0.5f, 0.87f, 0.67f, 1.0f, 0.32f, 0.09f, 0.95f, 0.56f, 0.72f, 1.0f, 0.04f, 0.76f, 0.11f, 0.46f, 0.88f, 0.72f, 0.0f, 0.38f, 0.5f},
	{0.75f, 0.51f, 0.24f, 0.45f, 0.16f, 0.48f, 0.38f, 0.58f, 0.75f, 0.16f, 0.81f, 0.0f, 0.3f, 0.4f, 0.31f, 0.37f, 0.5f, 1.0f, 0.54f, 0.85f, 0.83f, 0.43f, 0.46f, 0.5f},
	{0.31f, 0.51f, 0.43f, 0.0f, 0.35f, 0.5f, 0.34f, 0.26f, 0.53f, 0.0f, 0.63f, 0.0f, 0.22f, 0.0f, 0.39f, 0.0f, 0.8f, 0.0f, 0.44f, 0.81f, 0.51f, 0.0f, 0.5f, 0.5f},
	{0.72f, 0.82f, 1.0f, 0.0f, 0.35f, 0.5f, 0.37f, 0.47f, 0.54f, 0.0f, 0.5f, 0.0f, 0.45f, 0.0f, 0.39f, 0.0f, 0.39f, 0.0f, 0.48f, 0.81f, 0.6f, 0.0f, 0.71f, 0.5f},
	{0.81f, 0.76f, 0.19f, 0.0f, 0.18f, 0.7f, 0.4f, 0.3f, 0.54f, 0.17f, 0.4f, 0.0f, 0.42f, 0.23f, 0.47f, 0.12f, 0.48f, 0.0f, 0.49f, 0.53f, 0.36f, 0.34f, 0.56f, 0.5f},       
	{0.57f, 0.49f, 0.31f, 0.0f, 0.35f, 0.5f, 0.46f, 0.0f, 0.68f, 0.0f, 0.5f, 0.46f, 0.3f, 1.0f, 0.23f, 0.3f, 0.5f, 1.0f, 0.31f, 1.0f, 0.38f, 0.0f, 0.5f, 0.5f},
	{0.0f, 0.25f, 0.5f, 0.0f, 0.35f, 0.5f, 0.08f, 0.36f, 0.69f, 1.0f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.96f, 0.5f, 1.0f, 0.92f, 0.97f, 0.5f, 1.0f, 0.0f, 0.5f},
	{0.0f, 0.25f, 0.5f, 0.0f, 0.35f, 0.5f, 0.16f, 0.85f, 0.5f, 0.28f, 0.5f, 0.37f, 0.3f, 0.0f, 0.25f, 0.89f, 0.5f, 1.0f, 0.89f, 0.24f, 0.5f, 1.0f, 1.0f, 0.5f},
	{1.0f, 0.37f, 0.51f, 0.0f, 0.35f, 0.5f, 0.0f, 1.0f, 0.97f, 0.0f, 0.5f, 0.02f, 0.2f, 0.0f, 0.2f, 0.0f, 0.46f, 0.0f, 0.3f, 0.81f, 0.5f, 0.78f, 0.48f, 0.5f},
	{0.0f, 0.25f, 0.5f, 0.0f, 0.76f, 0.94f, 0.3f, 0.33f, 0.76f, 0.0f, 0.68f, 0.0f, 0.59f, 0.0f, 0.59f, 0.1f, 0.5f, 0.0f, 0.5f, 0.81f, 0.5f, 0.7f, 0.0f, 0.5f},
	{0.5f, 0.41f, 0.23f, 0.45f, 0.77f, 0.0f, 0.4f, 0.65f, 0.95f, 0.0f, 0.5f, 0.33f, 0.5f, 0.0f, 0.25f, 0.0f, 0.7f, 0.65f, 0.18f, 0.32f, 1.0f, 0.0f, 0.06f, 0.5f},
};

const char* 
JX10Processor::s_presetNames[kNumPrograms] = 
{
	"5th Sweep Pad",
	"Echo Pad [SA]",
	"Space Chimes [SA]",
	"Solid Backing",
	"Velocity Backing [SA]",
	"Rubber Backing [ZF]",
	"808 State Lead",
	"Mono Glide",
	"Detuned Techno Lead",
	"Hard Lead [SA]",
	"Bubble",
	"Monosynth",
	"Moogcury Lite",
	"Gangsta Whine",
	"Higher Synth [ZF]",
	"303 Saw Bass",
	"303 Square Bass",
	"Analog Bass",
	"Analog Bass 2",
	"Low Pulses",
	"Sine Infra-Bass",
	"Wobble Bass [SA]",
	"Squelch Bass",
	"Rubber Bass [ZF]",
	"Soft Pick Bass",
	"Fretless Bass",
	"Whistler",
	"Very Soft Pad",
	"Pizzicato",
	"Synth Strings",
	"Synth Strings 2",
	"Leslie Organ",
	"Click Organ",
	"Hard Organ",
	"Bass Clarinet",
	"Trumpet",
	"Soft Horn",
	"Brass Section",
	"Synth Brass",
	"Detuned Syn Brass [ZF]",
	"Power PWM",
	"Water Velocity [SA]",
	"Ghost [SA]",
	"Soft E.Piano",
	"Thumb Piano",
	"Steel Drums [ZF]",
	"Car Horn",
	"Helicopter",
	"Arctic Wind",
	"Thip",
	"Synth Tom",
	"Squelchy Frog"
};

ParamDef JX10Processor::s_paramDefs[kNumParams] = 
{
    ParamDef("OSC Mix", nullptr, .15, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kOSCMix),
    ParamDef("OSC Tune", nullptr, .6, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kOSCTune),
    ParamDef("OSC Fine", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kOSCFine),
    ParamDef("Glide", nullptr, .5, ParamDef::Range(0, 1, .166),  // six choices
            ParamDef::kInt, ParamDef::kAutomatable, kGlide),
    ParamDef("Gld Rate", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kRatio, ParamDef::kAutomatable, kGldRate),
    ParamDef("Gld Bend", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kGldBend),
    ParamDef("VCF Freq", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kVCFFreq),
    ParamDef("VCF Reso", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kVCFReso),
    ParamDef("VCF Env", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kVCFEnv),
    ParamDef("VCF LFO", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kVCFLFO),
    ParamDef("VCF Vel", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kVCFVel),
    ParamDef("VCF Att", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kHz, ParamDef::kAutomatable, kVCFAtt),
    ParamDef("VCF Dec", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kHz, ParamDef::kAutomatable, kVCFDec),
    ParamDef("VCF Sus", nullptr, .5, ParamDef::Range(0, 1, .01),
            ParamDef::kHz, ParamDef::kAutomatable, kVCFSus),
    ParamDef("VCF Rel", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kHz, ParamDef::kAutomatable, kVCFRel),
    ParamDef("ENV Att", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kHz, ParamDef::kAutomatable, kENVAtt),
    ParamDef("ENV Dec", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kHz, ParamDef::kAutomatable, kENVDec),
    ParamDef("ENV Sus", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kHz, ParamDef::kAutomatable, kENVSus),
    ParamDef("ENV Rel", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kHz, ParamDef::kAutomatable, kENVRel),
    ParamDef("LFO Rate", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kHz, ParamDef::kAutomatable, kLFORate),
    ParamDef("Vibrato", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kPct, ParamDef::kAutomatable, kVibrato),
    ParamDef("Noise", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kUInt, ParamDef::kAutomatable, kNoise),
    ParamDef("Octave", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kUInt, ParamDef::kAutomatable, kOctave),
    ParamDef("Tuning", nullptr, .5, ParamDef::Range(0, 1, .01), 
            ParamDef::kUInt, ParamDef::kAutomatable, kTuning)
};

//-----------------------------------------------------------------------------
JX10Processor::JX10Processor(double sampleRate) :
    BaseProcessor(sampleRate, kNumParams, s_paramDefs)
{
    this->selectPreset(0);
}

//-----------------------------------------------------------------------------
JX10Processor::~JX10Processor()
{
}

void 
JX10Processor::printPresets() 
{ 
    for(int i=0;i<kNumPrograms;i++)
        std::cerr << i << ": " << s_presetNames[i] << "\n";
}

//-----------------------------------------------------------------------------
void
JX10Processor::selectPreset(int k)
{
    // std::cerr << "selectPreset " << k << "\n";
    for(int i = 0; i < kNumParams; i++)
    {
        m_params[i].value = s_programParams[k][i];
    }

    //initialise...
    for(int v=0; v<m_synthData.numVoices; v++) 
    {
        memset(&m_synthData.voice[v], 0, sizeof(VOICE));
        m_synthData.voice[v].dp   = m_synthData.voice[v].dp2   = 1.0f;
        m_synthData.voice[v].saw  = m_synthData.voice[v].p     = m_synthData.voice[v].p2    = 0.0f;
        m_synthData.voice[v].env  = m_synthData.voice[v].envd  = m_synthData.voice[v].envl  = 0.0f;
        m_synthData.voice[v].fenv = m_synthData.voice[v].fenvd = m_synthData.voice[v].fenvl = 0.0f;
        m_synthData.voice[v].f0   = m_synthData.voice[v].f1    = m_synthData.voice[v].f2    = 0.0f;
        m_synthData.voice[v].note = 0;
        m_synthData.voice[v].noteID = kInvalidNote;
    }
    lfo = modwhl = filtwhl = press = fzip = 0.0f; 
    rezwhl = pbend = ipbend = 1.0f;
    volume = 0.0005f;
    K = mode = lastnote = m_synthData.sustain = m_synthData.activevoices = 0;
    noise = 22222;
    m_paramsDirty = true;
    processParamChanges();
    this->m_currentProgram = k;
}

//-----------------------------------------------------------------------------
void JX10Processor::monoProcessing(
    float *in, float *out, int sampleFrames)
{
	int frame=0, framesToNextEvent, v;
	float o, e, vib, pwm, pb=pbend, ipb=ipbend, gl=glide;
	float x, y, hpf=0.997f, min=1.0f, w=0.0f, ww=noisemix;
	float ff, fe=filtenv, fq=filtq * rezwhl, fx=1.97f-0.85f*fq, fz=fzip;
	int k=K;
	unsigned r;

	vib = sinf(lfo);
	ff = filtf + filtwhl + (filtlfo + press) * vib; //have to do again here as way that
	pwm = 1.0f + vib * (modwhl + pwmdep);           //below triggers on k was too cheap!
	vib = 1.0f + vib * (modwhl + vibrato);

    //detect & bypass completely empty blocks
    int eventPos = 0;
	if(m_synthData.activevoices>0 || this->hasEvents()) 
	{
		while(frame<sampleFrames)  
		{
            // frame 'til next event (when empty frames will be >> sampleFrames)
            if(eventPos < m_eventPos)
            {
                framesToNextEvent = m_events[eventPos].sampleOffset; 
                if(framesToNextEvent>sampleFrames) 
                    framesToNextEvent = sampleFrames;
                if(framesToNextEvent == 0) //next note on/off
                    this->processEvents(eventPos, frame);
            }
            else
                framesToNextEvent = sampleFrames;

            // update active voices before potentially activating more
            // would be faster with voice loop outside frame loop!
            // but then each voice would need it's own LFO...
			while(framesToNextEvent>=0) 
            {
				VOICE *V = m_synthData.voice.data();
				o = 0.0f;

				noise = (noise * 196314165) + 907633515;
				r = (noise & 0x7FFFFF) + 0x40000000; //generate noise + fast convert to float
				w = *(float *)&r;
				w = ww * (w - 3.0f);

				if(--k<0)
				{
					lfo += dlfo;
					if(lfo>PI) lfo -= TWOPI;
					vib = (float)sin(lfo);
					ff = filtf + filtwhl + (filtlfo + press) * vib;
					pwm = 1.0f + vib * (modwhl + pwmdep);
					vib = 1.0f + vib * (modwhl + vibrato);
					k = KMAX;
				}

				for(v=0; v<m_synthData.numVoices; v++)  //for each voice
				{ 
					e = V->env;
					if(e > SILENCE)
					{ //Sinc-Loop Oscillator
						x = V->p + V->dp;
						if(x > min) 
						{
							if(x > V->pmax) 
							{ 
								x = V->pmax + V->pmax - x;  
								V->dp = -V->dp; 
							}
							V->p = x;
							x = V->sin0 * V->sinx - V->sin1; //sine osc
							V->sin1 = V->sin0;
							V->sin0 = x;
							x = x / V->p;
						}
						else
						{ 
							V->p = x = - x;  
							V->dp = V->period * vib * pb * V->snaPitchbend; //set period for next cycle
							V->pmax = floorf(0.5f + V->dp) - 0.5f;
							V->dc = -0.5f * V->lev / V->pmax;
							V->pmax *= PI;
							V->dp = V->pmax / V->dp;
							V->sin0 = V->lev * sinf(x);
							V->sin1 = V->lev * sinf(x - V->dp);
							V->sinx = 2.0f * cosf(V->dp);
							if(x*x > .1f) 
                                x = V->sin0 / x; 
                            else 
                                x = V->lev; //was 0.01f;
						}

						y = V->p2 + V->dp2; //osc2
						if(y > min) 
						{ 
							if(y > V->pmax2) 
							{ 
								y = V->pmax2 + V->pmax2 - y;  
								V->dp2 = -V->dp2; 
							}
							V->p2 = y;
							y = V->sin02 * V->sinx2 - V->sin12;
							V->sin12 = V->sin02;
							V->sin02 = y;
							y = y / V->p2;
						}
						else
						{
							V->p2 = y = - y;  
							V->dp2 = V->period * V->detune * pwm * pb * V->snaPitchbend;
							V->pmax2 = floorf(0.5f + V->dp2) - 0.5f;
							V->dc2 = -0.5f * V->lev2 / V->pmax2;
							V->pmax2 *= PI;
							V->dp2 = V->pmax2 / V->dp2;
							V->sin02 = V->lev2 * sinf(y);
							V->sin12 = V->lev2 * sinf(y - V->dp2);
							V->sinx2 = 2.0f * cosf(V->dp2);
							if(y*y > .1f) 
                                y = V->sin02 / y; 
                            else 
                                y = V->lev2;
						}
						V->saw = V->saw * hpf + V->dc + x - V->dc2 - y;  //integrated sinc = saw
						x = V->saw + w;
						V->env += V->envd * (V->envl - V->env);

						if(k==KMAX) //filter freq update at LFO rate
						{
                            //envelopes
							if((V->env+V->envl)>3.0f) 
                            { 
                                V->envd=dec; 
                                V->envl=sus; 
                            } 
							V->fenv += V->fenvd * (V->fenvl - V->fenv);
							if((V->fenv+V->fenvl)>3.0f) 
                            { 
                                V->fenvd=fdec; 
                                V->fenvl=fsus; 
                            }

							fz += 0.005f * (ff - fz); //filter zipper noise filter
							y = V->fc * expf(fz + fe * V->fenv) * ipb; //filter cutoff
							if(y<0.005f) 
                                y =0.005f;
							V->ff = y;

							V->period += gl * (V->target - V->period); //glide
							if(V->target < V->period) 
                                V->period += gl * (V->target - V->period);
						}

						if(V->ff > fx) 
                            V->ff = fx; //stability limit

						V->f0 += V->ff * V->f1; //state-variable filter
						V->f1 -= V->ff * (V->f0 + fq * V->f1 - x - V->f2);
						V->f1 -= 0.2f * V->f1 * V->f1 * V->f1; //soft limit  //was 0.08f
						V->f2 = x;

                        o += V->env * V->f0 * V->snaVolume;
					}
					V++;
				}

				*out++ = o;
                framesToNextEvent--;
                frame++;
			} // while framesToNextEvent
		} // while frame < sampleFrames

		m_synthData.activevoices = m_synthData.numVoices;
		for(v=0; v<m_synthData.numVoices; v++)
		{
			if(m_synthData.voice[v].env<SILENCE)  //choke voices
			{
				m_synthData.voice[v].env = m_synthData.voice[v].envl = 0.0f;
				m_synthData.voice[v].f0 = m_synthData.voice[v].f1 = m_synthData.voice[v].f2 = 0.0f;
				m_synthData.activevoices--;
			}
		}
	}
	else //completely empty block
	{
		while (--sampleFrames >= 0)
			*out++ = 0.0f;
	}
	fzip = fz;
	K = k;
}

//-----------------------------------------------------------------------------
void 
JX10Processor::processEvents(int &eventPos, int frame)
{
    while(eventPos < m_eventPos)
    {
        MidiEvent &e = m_events[eventPos];
        if(e.sampleOffset > frame)
            break;
        else
        {
            this->processEvent(e);
            eventPos++;
        }
    }
}

void 
JX10Processor::processEvent(MidiEvent& event)
{
	float l = 1.0f;
	int vl=0;

	if(event.IsNoteOn())
	{
		unsigned pitch = event.GetNote();
		float velocity = event.GetVelocity(); // 0-127
		if(veloff) velocity = 80;

        float p, l=100.0f; //louder than any envelope!
        bool polyMode = (mode < 3);
        bool doglide = !(mode == 0 || mode == 3);
        bool legato = (mode == 1 || mode == 5);
        int tmp, v=0, held=0;

		if(!polyMode) // monophonic ie one voice (!= one channel)
		{
			if(m_synthData.voice[0].noteID != kInvalidNote) //legato pitch change
			{
				for(tmp=(m_synthData.numVoices-1); tmp>0; tmp--)  //queue any held notes
				{
					m_synthData.voice[tmp].note = m_synthData.voice[tmp - 1].note;
					m_synthData.voice[tmp].noteID = m_synthData.voice[tmp - 1].noteID;
				}
				p = tune * expf(-0.05776226505 * ((float)pitch + ANALOG * (float)v));
				while(p<3.0f || (p * detune)<3.0f) 
                    p += p;
				m_synthData.voice[0].target = p;
				if((doglide)==0) 
                    m_synthData.voice[v].period = p;
				m_synthData.voice[0].fc = expf(filtvel * (float)(velocity - 64)) / p;
				m_synthData.voice[0].env += SILENCE + SILENCE; ///was missed out below if returned?
				m_synthData.voice[0].note = pitch;
				m_synthData.voice[0].noteID = event.GetNoteId();
				m_synthData.voice[0].snaVolume = 1.f;
				m_synthData.voice[0].snaPitchbend = 1.f;
				return;
			}
		}
		else //polyphonic 
		{
            //replace quietest voice not in attack
			for(tmp=0; tmp<m_synthData.numVoices; tmp++)  
			{
				if(m_synthData.voice[tmp].note > 0) 
                    held++;
				if(m_synthData.voice[tmp].env<l && m_synthData.voice[tmp].envl<2.0f) 
                { 
                    l = m_synthData.voice[tmp].env;  
                    v = tmp; 
                }
			}
		}  
		p = tune * expf(-0.05776226505f * (pitch + ANALOG * v));
		while (p<3.0f || (p * detune)<3.0f) 
            p += p;
		m_synthData.voice[v].target = p;
		m_synthData.voice[v].detune = detune;

		tmp = 0;
		if(doglide || legato)
		{
			if((doglide) || held) 
                tmp = pitch - lastnote; //glide
		}
		m_synthData.voice[v].period = p * powf(1.059463094359f, (float)tmp - glidedisp);
		if(m_synthData.voice[v].period<3.0f) 
            m_synthData.voice[v].period = 3.0f; //limit min period

		m_synthData.voice[v].note = lastnote = pitch;
		m_synthData.voice[v].noteID = event.GetNoteId();

		m_synthData.voice[v].fc = expf(filtvel * (velocity - 64)) / p; //filter tracking

		m_synthData.voice[v].lev = voltrim * volume * (0.004f * (float)((velocity + 64) * (velocity + 64)) - 8.0f);
		m_synthData.voice[v].lev2 = m_synthData.voice[v].lev * oscmix;

		if(m_params[20].value<0.5f) //force 180 deg phase difference for PWM
		{
			if(m_synthData.voice[v].dp>0.0f)
			{
				p = m_synthData.voice[v].pmax + m_synthData.voice[v].pmax - m_synthData.voice[v].p;
				m_synthData.voice[v].dp2 = -m_synthData.voice[v].dp;
			}
			else
			{
				p = m_synthData.voice[v].p;
				m_synthData.voice[v].dp2 = m_synthData.voice[v].dp;
			}
			m_synthData.voice[v].p2 = m_synthData.voice[v].pmax2 = p + PI * m_synthData.voice[v].period;

			m_synthData.voice[v].dc2 = 0.0f;
			m_synthData.voice[v].sin02 = m_synthData.voice[v].sin12 = m_synthData.voice[v].sinx2 = 0.0f;
		}

		if(!polyMode) //monophonic retriggering
		{
			m_synthData.voice[v].env += SILENCE + SILENCE;
		}
		else
		{
			//if(params[15] < 0.28f) 
			//{
			//  voice[v].f0 = voice[v].f1 = voice[v].f2 = 0.0f; //reset filter
			//  voice[v].env = SILENCE + SILENCE;
			//  voice[v].fenv = 0.0f;
			//}
			//else 
			m_synthData.voice[v].env += SILENCE + SILENCE; //anti-glitching trick
		}
		m_synthData.voice[v].envl  = 2.0f;
		m_synthData.voice[v].envd  = att;
		m_synthData.voice[v].fenvl = 2.0f;
		m_synthData.voice[v].fenvd = fatt;
		m_synthData.voice[v].snaVolume = 1.f;
		m_synthData.voice[v].snaPitchbend = 1.f;
	}
	else //note off
    if(event.IsNoteOff())
	{
        float p;
        int v=0, held=0;
        bool doglide = !(mode == 0 || mode == 3);
        bool legato = (mode == 1 || mode == 5);
        bool polyMode = (mode < 3);
        unsigned pitch = event.GetNote();
		if((!polyMode) && (m_synthData.voice[0].noteID==event.GetNoteId())) //monophonic (and current note)
		{
			for(v=(m_synthData.numVoices-1); v>0; v--) 
			{
				if(m_synthData.voice[v].noteID!=kInvalidNote) 
                    held = v; //any other notes queued?
			}
			if(held>0)
			{
				m_synthData.voice[v].note = m_synthData.voice[held].note;
				m_synthData.voice[v].noteID  = m_synthData.voice[held].noteID;
				clearVoice(m_synthData.voice[held]);

				p = tune * expf(-0.05776226505f * (m_synthData.voice[v].note + ANALOG * (float)v));
				while(p<3.0f || (p * detune)<3.0f)
                    p += p;
				m_synthData.voice[v].target = p;
				if((doglide || legato)==0) 
                    m_synthData.voice[v].period = p;
				m_synthData.voice[v].fc = 1.0f / p;
			}
			else
			{
				clearVoice(m_synthData.voice[v]);
			}
		}
		else //polyphonic
		{
			for(v=0; v<m_synthData.numVoices; v++) 
            {
                //any voices playing that note?
                if(m_synthData.voice[v].noteID == event.GetNoteId())
                {
                    if(m_synthData.sustain==0)
                        clearVoice(m_synthData.voice[v]);
                    else 
                    {
                        m_synthData.voice[v].note = kInvalidNote;
                        m_synthData.voice[v].noteID = kInvalidNote;
                    }
                }
            }
		}
	}
    else
    if(event.IsPitchWheel())
    {
        float newValue = event.PitchWheelVal(); /* -8192->8192 */
		ipbend = expf(0.000014102 * newValue);
		pbend = 1.0f / ipbend;
        // std::cerr << "pbend " << newValue << " -> " << pbend << "\n";
    }
    else
    if(event.IsModWheel())
    {
        int newValue = event.ModWheelVal(); // [0-127]
        this->modwhl = 0.00000005f * (newValue * newValue);
        // std::cerr << "modwhl " << modwhl << "\n";
    }
    else
    if(event.IsBreath())
    {
		filtwhl = 0.02f * event.CCVal(); // 0-127
        // std::cerr << "filtwhl " << filtwhl << "\n";
    }
    else
    if(event.IsFilterResonance()) // kExpressionParam
    {
		rezwhl = 0.0065f * (154 - event.CCVal());
        // std::cerr << "rezwhl " << rezwhl << "\n";  (common MPE)
    }
    else
    if(event.IsChannelAftertouch())
    {
        float newValue = event.AftertouchVal(); /* 0-127 */
		press = 0.00001f * (newValue * newValue);
        // std::cerr << "press " << press << "\n";
    }
}

void 
JX10Processor::clearVoice(VOICE& v)
{
	v.envl  = 0.0f;
	v.envd  = rel;
	v.fenvl = 0.0f;
	v.fenvd = frel;
	v.note  = 0;
	v.noteID  = kInvalidNote;
}

//-----------------------------------------------------------------------------
// input params:
//      kOctave, kCoarse, kFine, kModInit
// output params:
//    tune, rati, ratf, ratio, depth, depht2, velsens, vibrato
void 
JX10Processor::processParamChanges()
{
    if(!this->m_paramsDirty) return;
	float ifs = this->m_samplePeriod;

	mode = std::min<int>(5, (int)(m_params[kGlide].value * 6)); // [0-1] => [0-5]
    // std::cerr << "Mode " << m_params[kGlide].value << " -> " << mode << "\n";
	noisemix = m_params[21].value * m_params[kNoise].value;
	voltrim = (3.2f - m_params[kOSCMix].value - 1.5f * noisemix) * 
              (1.5f - 0.5f * m_params[kVCFReso].value);
	noisemix *= 0.06f;
	oscmix = m_params[kOSCMix].value;

	semi = floorf(48.0f * m_params[kOSCTune].value) - 24.0f;
	cent = 15.876f * m_params[kOSCFine].value - 7.938f;
	cent = 0.1f * floorf(cent * cent * cent);
	detune = powf(1.059463094359f, - semi - 0.01f * cent);
	tune = -23.376f - 2.0f * m_params[kTuning].value - 
            12.0f * floorf(m_params[kOctave].value * 4.9);
	tune = this->m_sampleRate * powf(1.059463094359f, tune);

	if(m_params[kVibrato].value<0.5f) 
        vibrato = 0.0f;
    else
    {
        vibrato = pwmdep = 0.2f * (m_params[kVibrato].value - 0.5f)
                            * (m_params[kVibrato].value - 0.5f);
    }

	lfoHz = expf(7.0f * m_params[kLFORate].value - 4.0f);
	dlfo = lfoHz * (ifs * TWOPI * KMAX); 

	filtf = 8.0f * m_params[kVCFFreq].value - 1.5f;
	filtq  = (1.0f - m_params[kVCFReso].value) * 
             (1.0f - m_params[kVCFReso].value); ////// + 0.02f;
	filtlfo = 2.5f * m_params[kVCFLFO].value * m_params[kVCFLFO].value;
	filtenv = 12.0f * m_params[kVCFEnv].value - 6.0f;
	filtvel = 0.1f * m_params[kVCFVel].value - 0.05f;
	if(m_params[kVCFVel].value<0.05f) 
    { 
        veloff = 1; 
        filtvel = 0; 
    } 
    else 
        veloff = 0;

	att = 1.0f - expf(-ifs * expf(5.5f - 7.5f * m_params[kENVAtt].value));
	dec = 1.0f - expf(-ifs * expf(5.5f - 7.5f * m_params[kENVDec].value));
	sus = m_params[kENVSus].value;
	if(m_params[kENVRel].value<0.01f) 
        rel = 0.1f; //extra fast release
    else
        rel = 1.0f - expf(-ifs * expf(5.5f - 7.5f * m_params[kENVRel].value));

	ifs *= KMAX; //lower update rate...

	fatt = 1.0f - expf(-ifs * expf(5.5f - 7.5f * m_params[kVCFAtt].value));
	fdec = 1.0f - expf(-ifs * expf(5.5f - 7.5f * m_params[kVCFDec].value));
	fsus = m_params[kVCFSus].value * m_params[kVCFSus].value;
	frel = 1.0f - expf(-ifs * expf(5.5f - 7.5f * m_params[kVCFRel].value));

	if(m_params[kGldRate].value<0.02f) 
        glide = 1.0f; 
    else
        glide = 1.0f - expf(-ifs * expf(6.0 - 7.0f * m_params[4].value));
	glidedisp = (6.604f * m_params[5].value - 3.302f);
	glidedisp *= glidedisp * glidedisp;

    m_paramsDirty = false;
}