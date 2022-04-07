#ifndef processingUtil_h
#define processingUtil_h

#include "vst3.h"

#include <vector>
#include <cstdio>
#include <iostream>

inline void dumpTUID(char const *msg, const Steinberg::TUID x, bool ret=1)
{
    char buf[128];
    Steinberg::FUID xx = Steinberg::FUID::fromTUID(x);
    if(xx == Steinberg::Vst::IHostApplication::iid)
        sprintf(buf, "%s %s (%d)", msg, "IHostApplication", ret);
    else
    if(xx == Steinberg::Vst::IPlugInterfaceSupport::iid)
        sprintf(buf, "%s %s (%d)", msg, "IPlugInterfaceSupport", ret);
    else
    if(xx == Steinberg::Vst::IComponentHandler::iid)
        sprintf(buf, "%s %s (%d)", msg, "IComponentHandler", ret);
    else
    if(xx == Steinberg::Vst::IComponentHandler2::iid)
        sprintf(buf, "%s %s (%d)", msg, "IComponentHandler2", ret);
    else
    if(xx == Steinberg::Vst::IComponentHandler3::iid)
        sprintf(buf, "%s %s (%d)", msg, "IComponentHandler3", ret);
    else
    if(xx == Steinberg::Vst::IComponentHandlerBusActivation::iid)
        sprintf(buf, "%s %s (%d)", msg, "IComponentHandlerBusActivation", ret);
    else
    if(xx == Steinberg::Vst::IContextMenuTarget::iid)
        sprintf(buf, "%s %s (%d)", msg, "IContextMenuTarget", ret);
    else
    if(xx == Steinberg::Vst::IUnitHandler::iid)
        sprintf(buf, "%s %s (%d)", msg, "IUnithandler", ret);
    else
	if(xx == Steinberg::Vst::IAudioProcessor::iid)
        sprintf(buf, "%s %s (%d)", msg, "IAudioProcessor", ret);
    else
	if(xx == Steinberg::Vst::IEditController::iid)
        sprintf(buf, "%s %s (%d)", msg, "IEditController", ret);
    else
	if(xx == Steinberg::Vst::IEditController2::iid)
        sprintf(buf, "%s %s (%d)", msg, "IEditController2", ret);
    else
	if(xx == Steinberg::Vst::IConnectionPoint::iid)
        sprintf(buf, "%s %s (%d)", msg, "IConnectionPoint", ret);
    else
	if(xx == Steinberg::Vst::IUnitInfo::iid)
        sprintf(buf, "%s %s (%d)", msg, "IUnitInfo", ret);
    else
	if(xx == Steinberg::Vst::IUnitData::iid)
        sprintf(buf, "%s %s (%d)", msg, "IUnitData", ret);
    else
	if(xx == Steinberg::Vst::IMidiMapping::iid)
        sprintf(buf, "%s %s (%d)", msg, "IMidiMapping", ret);
    else
	if(xx == Steinberg::Vst::IProgress::iid)
        sprintf(buf, "%s %s (%d)", msg, "IProgress", ret);
    else
	if(xx == Steinberg::Vst::IVst3WrapperMPESupport::iid)
        sprintf(buf, "%s %s (%d)", msg, "IVst3WrapperMPESupport", ret);
    else
	if(xx == Steinberg::Vst::IVst3ToVst2Wrapper::iid)
        sprintf(buf, "%s %s (%d)", msg, "IVst3ToVst2Wrapper", ret);
    else
	if(xx == Steinberg::Vst::IVst3ToAAXWrapper::iid)
        sprintf(buf, "%s %s (%d)", msg, "IVst3ToAAXWrapper", ret);
    else
    if(xx == Steinberg::FUnknown::iid)
        sprintf(buf, "%s %s (%d)", msg, "Unknown", ret);
    else
    {
        /* can be matched against defs in pluinterfaces/vst */
        Steinberg::uint8 * const f = (Steinberg::uint8 *) x;
        sprintf(buf, "%s 0x%X%X%X%X 0x%X%X%X%X 0x%X%X%X%X 0x%X%X%X%X (%d)\n", 
            msg, 
            f[0], f[1], f[2], f[3], 
            f[4], f[5], f[6], f[7], 
            f[8], f[9], f[10], f[11], 
            f[12], f[13], f[14], f[15], 
            ret);
    }
    std::cerr << buf << "\n";
}

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

#endif