#include "processingData.h"
#include "public.sdk/samples/vst-hosting/audiohost/source/media/miditovst.h"
// (nb: this header allocates an object and therefore must reside in .cpp)

#include <iostream>

ProcessingData::ProcessingData()
{
    this->debug = 0;
    this->verbosity = this->debug;
    this->eventSampleOffset = 0;
}

ProcessingData::~ProcessingData()
{
    // inputs, outputs are array of AudioBusBuffers
    // inside each AudioBusBuffer is an array of float* (of lenght numChannels)
    if(this->inputs)
    {
        for(int i=0;i<this->numInputs;i++)
        {
            Steinberg::Vst::AudioBusBuffers *x = this->inputs + i;
            #if 0
            for(int j=0;j<x->numChannels;j++)
            {
                /* currently we plug chuck data into these slots 
                */
                if(x->channelBuffers32[j])
                    delete [] x->channelBuffers32[j]; // array of float
            }
            #endif

            delete [] x->channelBuffers32; // array of float *
        }
        delete [] this->inputs; // array of AudioBusBuffers
    }
    if(this->outputs)
    {
        for(int i=0;i<this->numOutputs;i++)
        {
            Steinberg::Vst::AudioBusBuffers *x = this->outputs + i;
            #if 0
            for(int j=0;j<x->numChannels;j++)
            {
                /* currently we plug chuck data into these slots 
                 * or we free them via chucKInstBuffer
                 */
                if(x->channelBuffers32[j])
                    delete [] x->channelBuffers32[j]; // array of float
            }
            #endif
            delete [] x->channelBuffers32;
        }
        delete [] this->outputs;
    }
}

void
ProcessingData::SetVerbosity(int v)
{
    this->verbosity = v;
}

void 
ProcessingData::Initialize(Steinberg::Vst::ProcessSetup &pd)
{
    // these are superclass member variables (Vst::ProcessData)
    this->processMode = pd.processMode;
    this->symbolicSampleSize = pd.symbolicSampleSize;
    this->numInputs = this->busUsage.inAudioChan.size(); // measured in buses
    this->numOutputs = this->busUsage.outAudioChan.size();

    // Regardless of our use of the buses (controlled via active-state and 
    // speaker arrangement), we must still allocate AudioBusBuffers for all.
    if(this->numInputs > 0)
    {
        this->inputs = new Steinberg::Vst::AudioBusBuffers[this->numInputs];
        for(int i=0;i<this->numInputs;i++) // foreach bus
        {
            int nchan = this->busUsage.inAudioChan[i].nch;
            char const *bt = this->busUsage.inAudioChan[i].isAux ? "Aux" : "Main";
            if(this->verbosity)
            {
                std::cerr << "AudioIn bus: " 
                    << bt << "." << i << " nchan:" << nchan << "\n";
            }
            this->inputs[i].silenceFlags = 0;
            this->inputs[i].numChannels = nchan;
            this->inputs[i].channelBuffers32 = new float*[nchan];
            for(int j=0;j<nchan;j++)
            {
                this->inputs[i].channelBuffers32[j] = nullptr;
            }
        }
    }

    if(this->numOutputs > 0)
    {
        bool bufferInst = (this->numInputs == 0
                           && this->numOutputs == 1 
                           && this->busUsage.outAudioChan[0].nch == 2);
        if(bufferInst)
            this->instBuffer.Init(pd.maxSamplesPerBlock);

        this->outputs = new Steinberg::Vst::AudioBusBuffers[this->numOutputs];
        for(int i=0;i<this->numOutputs;i++) // foreach bus
        {
            // bool active = u->IsOutputBusActive(i);
            int nchan = this->busUsage.outAudioChan[i].nch;
            char const *bt = this->busUsage.outAudioChan[i].isAux ? "Aux" : "Main";
            if(this->verbosity)
            {
                std::cerr << "AudioOut bus: " 
                    << bt << "." << i << " nchan:" << nchan << "\n";
            }
            this->outputs[i].silenceFlags = 0;
            this->outputs[i].numChannels = nchan;
            this->outputs[i].channelBuffers32 = new float*[nchan];
            if(bufferInst)
            {
                for(int j=0;j<nchan;j++)
                    this->outputs[i].channelBuffers32[j] = this->instBuffer.buffers[j];
            }
            else
            {
                for(int j=0;j<nchan;j++)
                    this->outputs[i].channelBuffers32[j] = nullptr;
            }
        }
    }
    this->inputParameterChanges = &this->inPChanges;
    this->inputEvents = &this->inEvents;
    this->outputEvents = &this->outEvents;

    /* setup processCtx, may be needed by, eg LABS (apparently not) ---- */
    this->processCtx.tempo = 120.0f;
    this->processCtx.projectTimeSamples = 0; // update on each call
    this->processCtx.timeSigNumerator = 4;
    this->processCtx.timeSigDenominator = 4;
    this->processCtx.frameRate.framesPerSecond = 30;
    this->processCtx.frameRate.flags = 0; // for frame pulldown, etc
    this->processCtx.state = Steinberg::Vst::ProcessContext::kPlaying |
                            Steinberg::Vst::ProcessContext::kTempoValid |
                            Steinberg::Vst::ProcessContext::kTimeSigValid |
                            Steinberg::Vst::ProcessContext::kSmpteValid; 
    this->processContext = &this->processCtx;

    if(this->verbosity)
    {
        // AudioIn routing ''
        // AudioOut routing '2'
        std::cerr << "AudioIn routing '";
        for(int i=0;i<this->busUsage.inputRouting.size();i++)
            std::cerr << char(this->busUsage.inputRouting[i] + '0');
        std::cerr << "'\n";

        std::cerr << "AudioOut routing '";
        for(int i=0;i<this->busUsage.outputRouting.size();i++)
            std::cerr << char(this->busUsage.outputRouting[i] + '0');
        std::cerr << "'\n";
    }
}

void 
ProcessingData::PrepareParamChange(Steinberg::Vst::ParamID paramId, 
                Steinberg::Vst::ParamValue value)
{
    Steinberg::int32 queueIndex;
    Steinberg::Vst::IParamValueQueue *q = 
        this->inPChanges.addParameterData(paramId, queueIndex);
    if(this->verbosity > 2)
    {
        std::cerr << "pchange " << paramId << " " << value 
            << " len:" <<  q->getPointCount() 
            << " soff:" << this->eventSampleOffset 
            << "\n";
    }
    
    Steinberg::int32 ptIndex;
    q->addPoint(this->eventSampleOffset, value, ptIndex);
}

// LABS:
//    initialize nparams: 2129
//    Input event busses
//      - 0 MIDI Input
//    AudioOut bus: Main.0 nchan:2
//    AudioIn routing ''
//    AudioOut routing '2'
// MDA DX10:
//    initialize nparams: 5
//    No event-in busses (apparently common to not indicate MIDI input support)
//    AudioIn bus: Main.0 nchan:2
//    AudioOut bus: Main.0 nchan:2
//    AudioIn routing '2'
//    AudioOut routing '2'
void
ProcessingData::PrepareMidiEvent(int status, int data1, int data2,
            Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping> midiMap)
{
    Steinberg::int16 ch = status & 0x0F;
    status &= 0xF0;
    auto evt = Steinberg::Vst::midiToEvent(status, ch, data1, data2);
    if(evt)
    {
        /* events are NoteOn/Off & PolyPressure ------------------- */ 
        evt->busIndex = 0; // the midi event bus..
        evt->flags = Steinberg::Vst::Event::kIsLive;
        // sample frames related to the current block start sample position 
        evt->sampleOffset = this->eventSampleOffset; 
        if(this->verbosity)
        {
            std::cerr << "ProcessingData MIDI event: " << status 
                << " note: " << data1 << " vel: " << data2 
                << " qlen: " << this->inEvents.getEventCount() 
                << " flags: " << evt->flags
                << " sampleOffset: " << evt->sampleOffset
                << " busIndex: " << evt->busIndex
                << "\n";
        }
        if(this->inEvents.addEvent(*evt) != Steinberg::kResultOk)
            std::cerr  << "Problem adding MIDI event to eventlist\n";
        return;
    }

    // else all else is converted to CC ----------------------------
    if(midiMap)
    {
        int bus = 0; // some plugins support multiple Event busses?

        // First look for non CCs like PitchBend and AfterTouch ---------
        // these are converted to CC-format (since ParamValues are double)
        auto callback = [bus, midiMap](Steinberg::int32 ch, 
                                        uint8_t data1) -> Steinberg::Vst::ParamID 
        {
            Steinberg::Vst::ParamID tag;
            midiMap->getMidiControllerAssignment(bus, ch, data1, tag);
            return tag;
        };
        // This routine applies remapping of standard non-cc (betd, pressure)
        // as well as CC:
        auto optParamChange = Steinberg::Vst::midiToParameter(status, ch, 
                                data1, data2, callback);
        if(optParamChange)
        {
            this->PrepareParamChange(optParamChange->first, optParamChange->second);
            return;
        }
        else
        {
            std::cout << "no midi mapping for " 
                << status << "/" << data1 <<"\n";
        }
    }
    else
        std::cout << "no midimapping found\n";
}

/* Process is called from ChucK multitick.
 * Currently nframes is always 1 and inCh and outCh are always 2 
 */
void
ProcessingData::Process(
    Steinberg::Vst::IAudioProcessor* audioEffect,
    float *in, int inCh, float *out, int outCh, int nframes)
{
    assert((inCh == 2 || inCh == 0) && outCh == 2 && nframes == 1);
    static float zero = 0.f;
    bool clearEvents = true;
    if(nframes == 1)
    {
        if(this->instBuffer.Active())
        {
            if(this->instBuffer.Empty())
            {
                this->numSamples = this->instBuffer.size;
                tresult result = audioEffect->process(*this);
                if(result != Steinberg::kResultOk)
                    std::cerr << "VST3 audioEffect process error\n";
                else
                    this->instBuffer.index = 0;
                this->eventSampleOffset = 0;
            }
            else
            {
                // Incoming midi events must preserve relative ordering.
                // They are only processed by the audioEffect (above).
                clearEvents = false;
                this->eventSampleOffset++;
            }
            int i = this->instBuffer.index;
            out[0] = this->instBuffer.buffers[0][i];
            out[1] = this->instBuffer.buffers[1][i];
            this->instBuffer.index++;
        }
        else
        {

            // For now we'll only fill the first in and out bus
            // this approach uses ChucK's buffers. In theory inCh
            // could be larger than 2 and in this way we'd support
            // side-chaining, etc. For now, we're hard-wired for 
            // inCh == 2, outCh == 2,
            this->numSamples = nframes;
            if(this->numInputs > 0)
            {
                int nbind=0;
                for(int i=0;i<this->busUsage.inputRouting.size();i++)
                {
                    int nch = this->busUsage.inAudioChan[i].nch;
                    int nused = this->busUsage.inputRouting[i];
                    for(int j=0;j<nch;j++)
                    {
                        if(j < nused && nbind < 2)
                            this->inputs[i].channelBuffers32[j] = in+nbind++;
                        else
                            this->inputs[i].channelBuffers32[j] = &zero;
                    }
                }
                assert(nbind <= 2);
            }
            if(this->numOutputs > 0)
            {
                int nbind=0;
                for(int i=0;i<this->busUsage.outputRouting.size();i++)
                {
                    int nch = this->busUsage.outAudioChan[i].nch;
                    int nused = this->busUsage.outputRouting[i];
                    for(int j=0;j<nch;j++)
                    {
                        if(j < nused && nbind < 2)
                            this->outputs[i].channelBuffers32[j] = out+nbind++;
                        else
                            this->outputs[i].channelBuffers32[j] = &zero;
                    }
                }
                assert(nbind <= 2);
            }
            // active: true, processing: true
            tresult result = audioEffect->process(*this);
            if(result != Steinberg::kResultOk)
                std::cerr << "VST3 audioEffect process error\n";
        }
    }
    else
        std::cerr << "Need to copy in+out of allocated buffers";

	if (clearEvents)
	{
		if (this->inPChanges.getParameterCount() > 0)
		{
            // Parameter changes commonly occur during "startup",
            // due to the fact that the .ck file requests them.
            if(this->verbosity)
            {
                std::cerr << "Processed " 
                    << this->inPChanges.getParameterCount() 
                    << " pchanges\n";
            }
		}
        if(this->outEvents.getEventCount())
        {
            if(this->verbosity)
            {
                std::cerr << "endAudioProcessing ignoring output events " 
                    << this->outEvents.getEventCount() << "\n";
            }
            this->outEvents.clear();
        }
        this->inEvents.clear();
        this->inPChanges.clearQueue();
    }
}