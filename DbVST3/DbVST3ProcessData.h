#ifndef DbVST3ProcessData_h
#define DbVST3ProcessData_h

#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include <vector>

#define VERBOSE 1

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
 *    processContext (nullptr)2
 * 
 */
class DbVST3ProcessData : public Steinberg::Vst::ProcessData
{
public:
    struct BusUsage
    {
        // each vector contains the number of channels associated with
        // each bus.  Since there are only expected to be small numbers
        // of channels in each bus (say 16, but typically 1 or 2), we
        // also include the Main vs Aux hint by setting bit. We take
        // this approach because the activateBus method requires the
        // bus-index and there's no guarantee that main and aux buses
        // have any order.

        int SetAuxBit(int nch) const
        {
            return nch & 0x80;
        }
        int GetNChan(int nch) const
        {
            return nch & 0x7F;
        }
        bool IsAux(int nch) const
        {
            return nch & 0x80;
        }

        std::vector<int> inAudioChan; 
        std::vector<int> outAudioChan;
        std::vector<int> activeInputBuses;
        std::vector<int> activeOutputBuses;

        int numInputChannels;
        int numOutputChannels;
        int numInputEventBuses;
        int numOutputEventBuses;

        void Reset()
        {
            this->numInputChannels = 0;
            this->numOutputChannels = 0;
            this->inAudioChan.clear();
            this->outAudioChan.clear();
            this->activeInputBuses.clear();
            this->activeOutputBuses.clear();

            this->numInputEventBuses = 0;
            this->numOutputEventBuses = 0;
        }
    };

    DbVST3ProcessData() 
    {
    }

    virtual ~DbVST3ProcessData() 
    {
        if(this->inputs)
        {
            delete [] this->inputs->channelBuffers32;
            delete [] this->inputs;
        }
        if(this->outputs)
        {
            delete [] this->outputs->channelBuffers32;
            delete [] this->outputs;
        }
    }

    void initialize(Steinberg::Vst::ProcessSetup &pd,  BusUsage const *u)
    {
        this->busUsage = u;
        this->processMode = pd.processMode;
        this->symbolicSampleSize = pd.symbolicSampleSize;
        this->numInputs = u->inAudioChan.size(); // measured in buses
        this->numOutputs = u->outAudioChan.size();

        if(this->numInputs > 0)
        {
            this->inputs = new Steinberg::Vst::AudioBusBuffers[this->numInputs];
            for(int i=0;i<this->numInputs;i++) // foreach bus
            {
                int nchan = u->GetNChan(u->inAudioChan[i]);
                this->inputs[i].numChannels = nchan;
                this->inputs[i].channelBuffers32 = new float*[nchan];
                for(int j=0;j<nchan;j++)
                {
                    // #XYZ
                    // currently nullptr whether active or not, we play
                    // with pointers during process to read & write directly
                    // to chuck buffers;
                    this->inputs[i].channelBuffers32[j] = nullptr;
                }
            }
        }
        if(this->numOutputs > 0)
        {
            this->outputs = new Steinberg::Vst::AudioBusBuffers[this->numOutputs];
            for(int i=0;i<this->numOutputs;i++) // foreach bus
            {
                int nchan = u->GetNChan(u->outAudioChan[i]);
                this->outputs[i].numChannels = nchan;
                this->outputs[i].channelBuffers32 = new float*[nchan];
                for(int j=0;j<nchan;j++)
                {
                    // see #XYZ above
                    this->outputs[i].channelBuffers32[j] = nullptr;
                }
            }
        }
        this->inputParameterChanges = &this->inPChanges;
        this->inputEvents = &this->inEvents;
    }

    void prepareParamChange(Steinberg::Vst::ParamID paramId, 
                        Steinberg::Vst::ParamValue value)
    {
        Steinberg::int32 queueIndex;
        Steinberg::Vst::IParamValueQueue *q = 
            this->inPChanges.addParameterData(paramId, queueIndex);
        #if VERBOSE && 0
        std::cout << "prepareParamChange " << paramId << " " << value 
            << " len:" <<  q->getPointCount() <<  "\n";
        #endif
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
            #if VERBOSE
            std::cout << "midi event: " << status 
                << " note: " << data1 << " vel: " << data2 
                <<  " qlen: " << this->inEvents.getEventCount() << "\n";
            #endif
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
                for(int i=0;i<this->inputs[0].numChannels;i++)
                    this->inputs[0].channelBuffers32[i] = in+i;
            }
            if(this->numOutputs > 0)
            {
                for(int i=0;i<this->outputs[0].numChannels;i++)
                    this->outputs[0].channelBuffers32[i] = out+i;
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

    BusUsage const *busUsage;
};

#endif