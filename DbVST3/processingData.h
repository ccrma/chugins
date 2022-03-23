#ifndef ProcessingData_h
#define ProcessingData_h

#include "vst3.h"

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
struct BusUsage
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


// We employ local implementation (rather than the same found in vst/hosting)
// in order to afford us a debugging hook.  Instruments should invoke our
// getEvent method.
class EventList : public Steinberg::Vst::IEventList
{
public:
	EventList()
	{
		m_events.reserve(128);
	}

	Steinberg::tresult PLUGIN_API 
    queryInterface(const Steinberg::TUID _iid, void** obj) override 
    { 
        QUERY_INTERFACE (_iid, obj, FUnknown::iid, Steinberg::Vst::IEventList) 
        QUERY_INTERFACE (_iid, obj, Steinberg::Vst::IEventList::iid, Steinberg::Vst::IEventList) *obj = nullptr; 
        return Steinberg::kNoInterface; 
    }

	Steinberg::uint32 PLUGIN_API 
    addRef() override
	{
		return 1;
	}

	Steinberg::uint32 PLUGIN_API 
    release() override
	{
		return 1;
	}

	Steinberg::int32 PLUGIN_API PLUGIN_API
    getEventCount() override
	{
		return m_events.size();
	}

	Steinberg::tresult PLUGIN_API
    getEvent(Steinberg::int32 index, Steinberg::Vst::Event& e) override
	{
        // std::cerr << "dbEventList.getEvent " << index << "\n";
		if(index >= 0 && index < (Steinberg::int32)m_events.size()) 
        {
			e = m_events[index];
			return Steinberg::kResultTrue;
		} 
        else 
			return Steinberg::kResultFalse;
	}
    
	Steinberg::tresult PLUGIN_API
    addEvent(Steinberg::Vst::Event& e) override
	{
		m_events.push_back(e);
		return Steinberg::kResultTrue;
	}

	void clear()
	{
		m_events.clear();
	}

protected:
	std::vector<Steinberg::Vst::Event> m_events;
};

class ProcessingData : public Steinberg::Vst::ProcessData
{
private:
    // we subclass ProcessData and thus can manipulate
    // inputEvents, etc..
    Steinberg::Vst::ParameterChanges inPChanges; // outPChanges are optional.
    Steinberg::Vst::ProcessContext processCtx; // assigned to this->processContext
    EventList inEvents;
    EventList outEvents;
    int verbosity;
    int debug;

public:
    BusUsage busUsage; 

    ProcessingData() 
    {
        this->debug = 1;
        this->verbosity = this->debug;
    }

    virtual ~ProcessingData() 
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

    void initialize(Steinberg::Vst::ProcessSetup &pd);

    void prepareParamChange(Steinberg::Vst::ParamID paramId, 
                        Steinberg::Vst::ParamValue value);

    void prepareMidiEvent(int status, int data1, int data2,
            Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping> midiMap);

    // caller (Processing) simply does
    //      this->beginAudioProcessing()
    //      caller->plugin->process()
    //      this->endAudioProcessing()
    // so all the goodies are in this file.
    void beginAudioProcessing(float *in, int inCh, 
        float *out, int outCh, int nframes);
    void endAudioProcessing();
};

#endif