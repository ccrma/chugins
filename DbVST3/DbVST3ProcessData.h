#ifndef DbVST3ProcessData_h
#define DbVST3ProcessData_h

#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include <vector>
#include <algorithm>
#include <cassert>

/* DbVST3ProcessData manages state around the all-important process
 * method invoked in DbVST3Processing.h. This includes audio buffer
 * management.
 * 
 *    processMode (0)
 *    symbolicSampleSize (kSample32)
 *    numSamples (0)
 *    numInputs (0)
 *    numOutputs (0)
 *    inputs (nullptr)
 *    outputs (nullptr)
 *    inputParameterChanges (nullptr)
 *    outputParameterChanges (nullptr)
 *    inputEvents (nullptr)
 *    outputEvents (nullptr)
 *    processContext (nullptr)
 * 
 */
struct DbVST3BusUsage
{
    // These arrays are indexed by bus and describe the number
    // of channels associated that that bus. The distinction 
    // between Main and Aux is also encoded in the second field 
    // of pair. Note that this just *describes* the component and 
    // doesn't characterize the current bus activation (routing).
    struct Bus
    {
        Bus() {}
        Bus(int ch, bool isaux)
        {
            this->nch = ch;
            this->isAux = isaux;
        }
        int nch;
        bool isAux;
    };
    std::vector<Bus> inAudioChan;  
    std::vector<Bus> outAudioChan;
    int numInputChannels;
    int numOutputChannels;
    int numInputEventBuses;
    int numOutputEventBuses;

    // These strings represent the signal routing.
    // Length is num of buses, value is number of channels
    // associated with that bus. Total should sum to <= 2 since
    // we're currently operating in the context of a stereo ugen.
    std::vector<int> inputRouting;
    std::vector<int> outputRouting;

    void Reset()
    {
        this->numInputChannels = 0;
        this->numOutputChannels = 0;
        this->inAudioChan.clear();
        this->outAudioChan.clear();
        this->inputRouting.clear();
        this->outputRouting.clear();

        this->numInputEventBuses = 0;
        this->numOutputEventBuses = 0;
    }

    bool IsInputBusActive(int busI) const
    {
        return this->inputRouting[busI] > 0;
    }

    bool IsOutputBusActive(int busI) const
    {
        return this->outputRouting[busI] > 0;
    }
};

class DbVST3ProcessData : public Steinberg::Vst::ProcessData
{
public:
    DbVST3BusUsage busUsage;

    DbVST3ProcessData() 
    {
        this->verbosity = 0;
    }

    virtual ~DbVST3ProcessData() 
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
    SetVerbosity(int v)
    {
        this->verbosity = v;
    }

    void initialize(Steinberg::Vst::ProcessSetup &pd)
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
                    // in attempting to get sound out of LABS we
                    // tried the experiment of pointing to real memory
                    // here... no-go. (on output too)
                    this->inputs[i].channelBuffers32[j] = nullptr;
                }
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
                {
                    this->outputs[i].channelBuffers32[j] = nullptr;
                }
            }
        }
        this->inputParameterChanges = &this->inPChanges;
        this->inputEvents = &this->inEvents;
        if(this->verbosity)
        {
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

    void prepareParamChange(Steinberg::Vst::ParamID paramId, 
                        Steinberg::Vst::ParamValue value)
    {
        Steinberg::int32 queueIndex;
        Steinberg::Vst::IParamValueQueue *q = 
            this->inPChanges.addParameterData(paramId, queueIndex);
        if(this->verbosity > 2)
            std::cerr << "prepareParamChange " << paramId << " " << value 
                << " len:" <<  q->getPointCount() <<  "\n";
        
        Steinberg::int32 ptIndex;
        q->addPoint(0, value, ptIndex);
    }

    void prepareMidiEvent(int status, int data1, int data2,
            Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping> midiMap)
    {
        int ch = 0; // tbd
        auto evt = Steinberg::Vst::midiToEvent(status, ch, data1, data2);
        if(evt)
        {
            /* events are NoteOn/Off & PolyPressure ------------------- */ 
            evt->busIndex = 0; // ??
            if(this->inEvents.addEvent(*evt) != Steinberg::kResultOk)
                std::cerr  << "Problem adding MIDI event to eventlist\n";
            if(this->verbosity)
            {
                std::cout << "midi event: " << status 
                    << " note: " << data1 << " vel: " << data2 
                    <<  " qlen: " << this->inEvents.getEventCount() << "\n";
            }
            return;
        }
        // else all else is converted to CC

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

    void beginAudioProcessing(float *in, int inCh, 
        float *out, int outCh, int nframes)
    {
        static float zero = 0.f;
        this->numSamples = nframes;
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

    void endAudioProcessing()
    {
        this->inEvents.clear();
        this->inPChanges.clearQueue();
    }

private: // ------------------------------------------------------------------
    Steinberg::Vst::ParameterChanges inPChanges;
    Steinberg::Vst::EventList inEvents;

    int verbosity;
};

#endif