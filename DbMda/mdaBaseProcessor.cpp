/*
 *  mdaBaseProcessor.cpp
 *
 *  Created by Arne Scheffler on 6/14/08.
 *  mda VST Plug-ins
 *  VST->Chugin by Dana Batali 4/22
 *
 *  Copyright (c) 2008 Paul Kellett
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "mdaBaseProcessor.h"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>

//-----------------------------------------------------------------------------
BaseProcessor::BaseProcessor(double sampleRate, 
                            int numParams,
                            ParamDef const *pdefs)
{
    m_sampleRate = sampleRate;
    m_samplePeriod = 1./sampleRate;
    m_params = new ParamValue[numParams];
    m_numParams = numParams;
    for(int i=0;i<numParams;i++)
    {
        m_params[i].def = pdefs+i;
        m_params[i].value = pdefs[i].dflt;
    }
    m_paramsDirty = true;
}

//-----------------------------------------------------------------------------
BaseProcessor::~BaseProcessor()
{
    delete [] m_params;
}

//-----------------------------------------------------------------------------
void BaseProcessor::printParams()
{
    for(int i=0;i<m_numParams;i++)
    {
        ParamDef const *def = m_params[i].def;
        std::cerr << i << ": " 
            << def->name << " "
            << m_params[i].value << "\n";
    }
}

//-----------------------------------------------------------------------------
void BaseProcessor::processMono(
    float *in, float *out, int nframes)
{
    if(m_paramsDirty)
    {
        this->processParamChanges();
        this->m_paramsDirty = false;
    }
    // we don't process events upfont but require that
    // instruments clear the event queue themselves
    // during doProcessing. This way we can achieve
    // "sample accuracy" for nframes > 1.
    this->monoProcessing(in, out, nframes);
    this->clearEvents();
}

void BaseProcessor::processMulti(
    float *in, int inch, float *out, int outch, int nframes)
{
    if(m_paramsDirty)
    {
        this->processParamChanges();
        this->m_paramsDirty = false;
    }
    // we don't process events upfont but require that
    // instruments clear the event queue themselves
    // during doProcessing. This way we can achieve
    // "sample accuracy" for nframes > 1.
    this->multiProcessing(in, inch, out, outch, nframes);
    this->clearEvents();
}