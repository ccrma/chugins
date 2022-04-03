/*
 *  mdaTalkboxProcessor.cpp
 *
 *  mda chugin
 *  Created by Dana Batali around 4/22
 *
 *  mda VST Plug-ins
 *  Created by Arne Scheffler on 6/14/08.
 *
 *  mda plugins
 *  Copyright (c) 2008 Paul Kellett
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#ifndef mdaTalkboxProcessor_h
#define mdaTalkboxProcessor_h

#include "mdaBaseProcessor.h"
#include "mdaParam.h"

class TalkboxProcessor : public BaseProcessor
{
private:
    enum paramIds
    {
        kWet = 0,
        kDry = 1,
        kQuality = 2,
        /* currently we require/assume Carrier on left, Modulator on right */
        kNumParams = 3
    };
    static ParamDef s_paramDefs[kNumParams];
    // no presets, no voices, we're an effect (stereo in, mono out)

public:
	TalkboxProcessor(double sampleRate);
	~TalkboxProcessor();

protected:
    void processParamChanges() override;
    void multiProcessing(float *in, int nIn, float *out, int nOut, int nframes) override;
    void monoProcessing(float *in, float *out, int nframes) override {} /* no-op */
	
private:
    void update();
    // no midi events

	void lpc(float *buf, float *car, int n, int o);
	void lpc_durbin(float *r, int p, float *k, float *g);

	float *car0, *car1;
	float *window;
	float *buf0, *buf1;

	float emphasis;
	int K, N, O, pos, swap;
	float wet, dry, FX;

	float d0, d1, d2, d3, d4;
	float u0, u1, u2, u3, u4;
};

#endif