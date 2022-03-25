#include "processingData.h"
#include "public.sdk/samples/vst-hosting/audiohost/source/media/miditovst.h"
// (nb: this header allocates an object and therefore must reside in .cpp)

#include <iostream>

ProcessingData::ProcessingData()
{
    this->debug = 0;
    this->verbosity = this->debug;
}

ProcessingData::~ProcessingData()
{
    if(this->inputs)
    {
        delete [] this->inputs->channelBuffers32; // array of float *
        delete [] this->inputs; // array of AudioBusBuffers
    }
    if(this->outputs)
    {
        delete [] this->outputs->channelBuffers32;
        delete [] this->outputs;
    }
}

void
ProcessingData::SetVerbosity(int v)
{
    this->verbosity = v;
}

void 
ProcessingData::initialize(Steinberg::Vst::ProcessSetup &pd)
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
                this->inputs[i].channelBuffers32[j] = nullptr;
        }
    }
    if(this->numOutputs > 0)
    {
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
            for(int j=0;j<nchan;j++)
                this->outputs[i].channelBuffers32[j] = nullptr;
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
                                Steinberg::Vst::ProcessContext::kTimeSigValid; 
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
ProcessingData::prepareParamChange(Steinberg::Vst::ParamID paramId, 
                Steinberg::Vst::ParamValue value)
{
    Steinberg::int32 queueIndex;
    Steinberg::Vst::IParamValueQueue *q = 
        this->inPChanges.addParameterData(paramId, queueIndex);
    if(this->verbosity > 2)
    {
        std::cerr << "prepareParamChange " << paramId << " " << value 
            << " len:" <<  q->getPointCount() <<  "\n";
    }
    
    Steinberg::int32 ptIndex;
    q->addPoint(0, value, ptIndex);
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
ProcessingData::prepareMidiEvent(int status, int data1, int data2,
            Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping> midiMap)
{
    int ch = 0; // tbd
    auto evt = Steinberg::Vst::midiToEvent(status, ch, data1, data2);
    if(evt)
    {
        /* events are NoteOn/Off & PolyPressure ------------------- */ 
        evt->busIndex = 0; // the midi event bus..
        evt->flags = Steinberg::Vst::Event::kIsLive;
        // sample frames related to the current block start sample position 
        evt->sampleOffset = 0; 
        if(this->verbosity || this->debug)
        {
            std::cout << "MIDI event: " << status 
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
        Steinberg::int16 ch = 0;  // some plugins support multiple channels

        // Rirst look for non CCs like PitchBend and AfterTouch ---------
        // these are converted to CC-format (since ParamValues are double)
        auto callback = [bus, midiMap](Steinberg::int32 ch, 
                                        uint8_t data1) -> Steinberg::Vst::ParamID 
        {
            Steinberg::Vst::ParamID tag;
            midiMap->getMidiControllerAssignment(bus, ch, data1, tag);
            return tag;
        };
        auto optParamChange = Steinberg::Vst::midiToParameter(status, ch, 
                                data1, data2, callback);
        if(optParamChange)
        {
            this->prepareParamChange(optParamChange->first, optParamChange->second);
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

// caller (Processing) simply does
//      this->beginAudioProcessing()
//      caller->plugin->process()
//      this->endAudioProcessing()
// so all the goodies are in this file.
void 
ProcessingData::beginAudioProcessing(
    float *in, int inCh, 
    float *out, int outCh, int nframes)
{
    static float zero = 0.f;
    this->numSamples = nframes;

    // this->processCtx.systemTime += nframe;
    // this->processCtx.state |= Steinberg::Vst::ProcessContext::kSystemTimeValid;
    if(nframes == 1)
    {
        // For now we'll only fill the first in and out bus
        // this approach uses ChucK's buffers. In theory inCh
        // could be larger than 2 and in this way we'd support
        // side-chaining, etc. For now, we're hard-wired for 
        // inCh == 2, outCh == 2,
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
    }
    else
        std::cerr << "Need to copy in+out of allocated buffers";
}

void 
ProcessingData::endAudioProcessing()
{
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