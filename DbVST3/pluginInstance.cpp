#include "pluginInstance.h"
#include "host.h"

#include <public.sdk/source/common/memorystream.h>
#include <iostream>
#include <unordered_map>
#include <cassert>

VST3PluginInstance::VST3PluginInstance()
{
    this->host = VST3Host::Singleton();
    assert(this->host->IsWorkerThread());
    this->error = 0;
    this->debug = 0;
    this->verbosity = 0;
    this->component = nullptr;
    this->controller = nullptr;
    this->activated = false;
}

VST3PluginInstance::~VST3PluginInstance()
{
    if(this->debug)
        std::cerr << "VST3PluginInstance deleted\n";
    this->deinitProcessing();
}

void
VST3PluginInstance::Init(
    const dbPlugProvider::PluginFactory &factory,
    VST3::Hosting::ClassInfo &classInfo,
    std::vector<ParamInfo> &parameters)
{
    // VST3Provider aka: PlugProvider here:
    //  source/vst/hosting/plugprovider.cpp
    this->provider.reset(new dbPlugProvider(this, factory, classInfo, true/*useGlobalInstance*/));
    this->initComponent();
    this->synchronizeStates();
    this->initParams(parameters);

    if(this->debug)
    {
        std::cerr << "VST3PluginInstance.Init " << this->provider->GetName() << "\n";
    }
}

void
VST3PluginInstance::InitProcessing(float sampleRate, 
                    char const *inputBusRouting, 
                    char const *outputBusRouting)
{
    assert(this->component);
    assert(this->host->IsWorkerThread());

    // this->controllerEx1 = Steinberg::FUnknownPtr<Steinberg::Vst::EditControllerEx1>(this->controller);
    if(this->verbosity)
        std::cerr << "PluginInstance.initialize nparams: " << this->controller->getParameterCount() << "\n";

	Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor> processor = component;
	if(!processor)
    {
        std::cerr << "oops: no audioeffect in " <<
            this->provider->GetName() << "\n";
        this->error = 1;;
    }
    else
    {
        memset(&this->processSetup, 0, sizeof(Steinberg::Vst::ProcessSetup));
        this->processSetup.processMode = Steinberg::Vst::kRealtime;
            // from: kRealtime, kPrefetch, kOffline
        this->processSetup.symbolicSampleSize = Steinberg::Vst::kSample32;
            // from: kSample32, kSample64
        this->processSetup.sampleRate = sampleRate; // a double
        this->processSetup.maxSamplesPerBlock = 256; // usually 1

        // Our job is to deliver the number of busses required
        // by the plugin.  An audio bus can be mono or stereo and
        // indications are that we should also follow the guidance
        // of the plugin rather than request Speaker-Arrangement 
        // override by the host.
        if(this->debug)
            fprintf(stderr, "SetupProcessing\n");
        if(processor->setupProcessing(this->processSetup) 
            == Steinberg::kResultTrue)
        {
            // synchronizeState has already been called
            this->initBuses(inputBusRouting, outputBusRouting);
            // initialize process data (allocate AudioBuffers, etc)
            // nb: processData owns busUsage which we've just initialized above.
            this->processData.Initialize(this->processSetup); 
            // syncState between controller and processor
            // via component->getState(&stream), stream.rewind
            //     controller->setComponentState()
        }
        else
        {
            std::cerr << "Problem setting up audioEffect for "
                << this->provider->GetName() << "\n";
        }
    }
    this->activate();
    if(this->debug)
    {
        std::cerr << "VST3PluginInstance.InitProcessing " 
                  << this->provider->GetName() << "\n";
    }

}

// 'param id' it's paramID !== index
// NB: callers (vst3Ctx.h, readAllParameters) know the logic behind 
//   setting pushToProcessor (and have access to the flags 
//   (kCanAutomate, ~kIsProgramChange)
int 
VST3PluginInstance::SetParamValue(Steinberg::Vst::ParamID pid,
    float value, bool pushToProcessor) /* as automation means register it via processData */
{
    // https://developer.steinberg.help/display/VST/Parameters+and+Automation
    int err = 0;

    // NB: the following case occurs when a preset is requested.
    //  Since we'd like to group-change the parameters we require
    //  that the processingLock be acquired in readAllParameters(andPush=true).
    // case: pushToProcessor && this->host->IsWorkerThread()

    if(this->debug > 1)
        std::cerr << "SetParamValue " << pid << "\n";

    if(this->controller)
    {
        if(pushToProcessor)
        {
            // assert kCanAutomate && kIsProgramChange
            if(this->verbosity > 1)
                std::cerr << "Update " << pid << ": " << value << "\n";
            this->processData.PrepareParamChange(pid, value);
        }
        else
        {
            this->controller->setParamNormalized(pid, value);
        }
    }
    else
        err = -1;
    return err;
}

Steinberg::Vst::ParamID
VST3PluginInstance::GetMidiMapping(int data1)
{
    // method is part of the controller and should therefore only
    // be invoked in the same thread it was created (WorkerThread)
    assert(this->host->IsWorkerThread());
    if(this->midiMapping)
    {
        Steinberg::Vst::ParamID pid;
        int bus=0, ch=0;
        this->midiMapping->getMidiControllerAssignment(bus, ch, data1, pid);
        return pid;
    }
    else
        return Steinberg::Vst::kNoParamId;
}

int 
VST3PluginInstance::MidiEvent(int status, int data1, int data2)
{
    int err = 0;
    if(this->host->IsWorkerThread())
    {
        if(this->controller)
        {
            const std::lock_guard<LockType> lk(this->processingLock); 
            this->processData.PrepareMidiEvent(status, data1, data2, this->midiMapping);
            err = 0;
        }
        else
        {
            if(this->debug)
                std::cerr << "NO controller for MidiEvent " << status << " "  << data1 << "\n";
            err = -1;
        }
    }
    else
    {
        std::function<void()> fn = std::bind(&VST3PluginInstance::MidiEvent, 
                                            this, status, data1, data2);
        this->host->Delegate(fn);
    }
    return err; 
}

void 
VST3PluginInstance::Process(float *in, int inCh, float *out, int outCh, int nframes)
{
    // lk guards against changes to MidiEventList and Parameter Automation.
    assert(this->host->IsProcessingThread());
    const std::lock_guard<LockType> lk(this->processingLock); 
    this->activate(); 
	Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor> processor = component;
    this->processData.Process(processor, in, inCh, out, outCh, nframes);
    // this->deactivate(); // only needed when we modify processMode, sampleRate,
}

void
VST3PluginInstance::deinitProcessing()
{
    this->provider->releasePlugIn(this->component, this->controller);
}

// After scratching heads for many days, trying to get
// JUCE plugins working, I encountered this intriguing comment 
// from ardour developer. After deploying this change, I still
// get no joy...
//
/* The official Steinberg SDK's source/vst/hosting/plugprovider.cpp
 * only initializes the controller if it is separate of the component.
 *
 * However some plugins expect an unconditional call and other
 * hosts incl. JUCE-based ones initialize a controller separately 
 * because FUnknownPtr<> cast may return a new obeject.
 *
 * So do not check for errors.
 * if Vst::IEditController is-a Vst::IComponent the Controller
 * may or may not already be initialized.
 */
void
VST3PluginInstance::initComponent()
{
    assert(this->host->IsWorkerThread());

    // provider takes care of proper initialization and ref-counting.
    this->component = this->provider->getComponent();	
    this->controller = this->provider->getController();

    // we're the component handler means that we must handle
    // begin/endEdit and restartComponent.
    if(Steinberg::kResultOk != this->controller->setComponentHandler(this))
    {
        std::cerr << "Problem setting componenthandler for " 
                  << this->provider->GetName() << "\n";
    }

    // midiMapping methods should be invoked in the same thread as the
    // controller was created, eg WorkerThread.
    this->midiMapping = Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping>(this->controller);
}

bool
VST3PluginInstance::synchronizeStates()
{
    std::cerr << "synchronizeStates\n";
    assert(this->component && this->host->IsWorkerThread());
    Steinberg::MemoryStream stream;
    if(this->component->getState(&stream) == Steinberg::kResultTrue) 
    {
        stream.seek(0, Steinberg::IBStream::kIBSeekSet, nullptr);
        // setComponentState triggers restartComponent (below)
        Steinberg::tresult res = this->controller->setComponentState(&stream);
        if((res != Steinberg::kResultOk) && (res != Steinberg::kNotImplemented))
        {
            std::cerr << "Couldn't synchronize VST3 component with controller state\n";
            this->readAllParameters(false);
        }
        if(this->verbosity || this->debug)
        {
            std::cerr << this->provider->GetName() << 
                " synchronized " << stream.getSize() << " bytes of state\n";
        }
        return res == Steinberg::kResultOk;
    }
    else
    {
        std::cerr << "problem reading state from plugin\n";
        return false;
    }
}

bool
VST3PluginInstance::activate()
{
    if(this->activated)
        return true;
    if(this->debug)
        std::cerr << "Activating " << this->provider->GetName() << "\n";
    Steinberg::tresult res = this->component->setActive(true);
    if (!(res == Steinberg::kResultOk || res == Steinberg::kNotImplemented)) 
        return false;

	Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor> processor = component;
    res = processor->setProcessing(true);
    if (!(res == Steinberg::kResultOk || res == Steinberg::kNotImplemented))
        return false;
    if(this->debug)
        std::cerr << "(activating succeeded)\n";
    this->activated = true;
    return true;
}

bool
VST3PluginInstance::deactivate()
{
    if(!this->activated)
        return true;
    if(this->debug)
        std::cerr << "Deactivating\n";
	Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor> processor = component;
    Steinberg::tresult res = processor->setProcessing(false);
    if(!(res == Steinberg::kResultOk || res == Steinberg::kNotImplemented)) 
        return false;

    res = this->component->setActive(false);
    if(!(res == Steinberg::kResultOk || res == Steinberg::kNotImplemented))
        return false;

    this->activated = false;
    return true; 
}

// readAllParameters is called in two conditions:
//  1) after state synchronization (andPush == false)
//  2) after program change (andPush == true)
void
VST3PluginInstance::readAllParameters(bool andPush)
{
    int nparams = this->controller->getParameterCount();
    this->paramValues.resize(nparams);
    for(int i=0;i<nparams;i++)
    {
        Steinberg::Vst::ParameterInfo info;
        this->controller->getParameterInfo(i, info);
        float val = this->controller->getParamNormalized(info.id);
        this->paramValues[i] = val;
    }
    if(andPush)
    {
        // a preset has changed and we need to update the processor.
        assert(this->host->IsWorkerThread());
        // NB: we shouldn't call SetParamValue(..., true) without a lock.
        const std::lock_guard<LockType> lk(this->processingLock); 
        for(int i=0;i<nparams;i++)
        {
            Steinberg::Vst::ParameterInfo info;
            this->controller->getParameterInfo(i, info);
            float val = this->controller->getParamNormalized(info.id);
            if(info.flags & Steinberg::Vst::ParameterInfo::kCanAutomate)
            {
                if(this->verbosity > 1)
                    std::cerr << i << ": " << val << "\n";
                this->SetParamValue(info.id, val, true /*updateProc*/);
            }
            else
            {
                // We don't register changes to program since it can trigger 
                // infinite restarts.
                if(!(info.flags & Steinberg::Vst::ParameterInfo::kIsProgramChange))
                    this->SetParamValue(info.id, val, true);
            }
        }
    }
}

// restartComponent is commonly triggered by VST when we issue
// this->controller->setComponentState(&stream) (above).
// Our job is merely to pull the values of its update params
// into our brain (aka shadow parameters). Things get a little
// more complex when it comes to a special parameter known as
// the program change parameter. Changing it triggers a 
// restartComponent. In theory we shouldn't ever need to
// "push" newly changed parameters.
tresult PLUGIN_API 
VST3PluginInstance::restartComponent(int32 flags)
{
    if(!this->host->IsWorkerThread())
    {
        std::function<void()> fn = std::bind(&VST3PluginInstance::restartComponent, 
                                                this, flags);
        this->host->Delegate(fn);
        return Steinberg::kResultOk;
    }

    if(flags & Steinberg::Vst::kReloadComponent)
    {
        if(this->verbosity)
            std::cerr << "VSTPluginInstance.restartComponent ReloadComponent\n";
        this->deactivate();
        this->activate();
    }
    if(flags & Steinberg::Vst::kParamValuesChanged)
    {
        bool pushToProcessor = false;
        if(this->verbosity)
        {
            std::cerr << "VST3PluginInstance.restartComponent ParamValuesChanged, "
                << "updateProcessor:" << pushToProcessor
                << "\n";
        }
        this->readAllParameters(pushToProcessor);
    }
    if(flags & Steinberg::Vst::kLatencyChanged)
    {
        if(this->verbosity)
            std::cerr << "VST3PluginInstance.restartComponent LatencyChanged\n";
        this->deactivate();
        this->activate();
    }
    return Steinberg::kResultOk;
}

// optional routing hints
//  - string with len == num input or output bus.
//  - in each position '0' || '1' || '2' represent channel allocation
//    for each bus.  This differs for each component, so there is no
//    universally correct answer.
// default behavior:
// - greedily allocate 2 channels prioritizing Main over Aux buses.
void 
VST3PluginInstance::initBuses(char const *inputBusRouting, char const *outputBusRouting)
{
    std::cerr << "PluginInstance.InitBuses\n";

    // countChannels initializes busConfig.
    this->processData.busUsage.Reset();

    /* audio buses -- */
    this->countChannels(Steinberg::Vst::kAudio, 
                        Steinberg::Vst::kInput, 
                        this->processData.busUsage.inAudioChan,
                        this->processData.busUsage.numInputChannels);
    int nbusIn = this->processData.busUsage.inAudioChan.size();
    if(nbusIn > 0)
        this->processData.busUsage.inputRouting.resize(nbusIn, 0);

    this->countChannels(Steinberg::Vst::kAudio, 
                        Steinberg::Vst::kOutput, 
                        this->processData.busUsage.outAudioChan,
                        this->processData.busUsage.numOutputChannels);
    int nbusOut = this->processData.busUsage.outAudioChan.size();
    if(nbusOut > 0)
        this->processData.busUsage.outputRouting.resize(nbusOut, 0);

    // Now that we understand the global bus picture, we need to
    // assign speaker-arrangements to characterize the multi-channel
    // configurations.  It's up to us to signal to the plugin that
    // a particular bus is active, so we'll adopt the convention
    // that we only handle: 0, 1, 2 input and output (Audio) buses
    // with no more than 2 channels (total) on each side. ie:
    // can't handle two, stereo inputs at-the-moment. This is driven
    // largely by the chuck chugin architecture that disallows dynamic
    // specification of the in & out ugen.
    // Note that the speaker-arrangement for inactive buses is set to 0.
    // Also, we prioritize Main buses over Aux buses which accounts
    // for the two loops through each.
    //
    // A bus can be understood as a "collection of data channels" belonging 
    // together. It describes a data input or a data output of the plug-in. 
    // A VST component can define any desired number of busses. Dynamic 
    // usage of busses is handled in the host by activating and deactivating 
    // busses. All busses are initially inactive. The component has to 
    // define the maximum number of supported busses and it has to define 
    // which of them have to be activated by default after instantiation 
    // of the plug-in (This is only a wish, the host is allow to not follow 
    // it, and only activate the first bus for example). A host that can 
    // handle multiple busses, allows the user to activate busses which 
    // are initially all inactive. The kMain busses have to place before 
    // any others kAux busses.
    //
    //  side-chaining scenarios
    //        
    //   A (TAL-Vocoder) kMain0 | kMain1
    //   input ch/bus:   1      | 1
    //   output ch/bus:  2      | 0
    //
    //                   kMain | kAux
    //   input ch/bus:   1     | 1
    //   output ch/bus:  2     | 0

    BusUsage &busUsage = this->processData.busUsage;
    std::vector<Steinberg::Vst::SpeakerArrangement> inSA;
    std::vector<Steinberg::Vst::SpeakerArrangement> outSA;
    inSA.resize(nbusIn);
    outSA.resize(nbusOut);
    int nchanIn=0, nchanOut=0;

    if(inputBusRouting && inputBusRouting[0] != '\0') 
    {
        if(strlen(inputBusRouting) != nbusIn)
        {
            std::cerr << "Incorrect length " << strlen(inputBusRouting) 
                << " for input channel routing (nbuses == " 
                << nbusIn
                << ")\n";
            inputBusRouting = nullptr; // force default
        }
        else
        {
            for(int i=0;i<nbusIn;i++)
            {
                int nch = inputBusRouting[i] - '0';
                nchanIn += nch;
                if(nchanIn <= 2)
                    busUsage.inputRouting[i] = nch;
                else
                {
                    std::cerr << "Custom input routing request exceeds channel allocation "
                        << inputBusRouting << "\n";
                    inputBusRouting = nullptr;
                    break;
                }
            }
        }
    }

    // if not provided, create the input routing map.
    if(!inputBusRouting || *inputBusRouting == '\0')
    {
        nchanIn = 0;
        for(int j=0;j<2;j++) // prioritize Main (j==0) over Aux
        {
            for(int i=0;i<nbusIn;i++)
            {
                BusUsage::Bus &binfo = busUsage.inAudioChan[i];
                if(j==0 && binfo.isAux)
                    continue;

                if(j==1 && !binfo.isAux)
                    continue;
                
                int busch = binfo.nch;
                nchanIn += busch;
                if(nchanIn <= 2)
                    busUsage.inputRouting[i] = busch;
                else
                    busUsage.inputRouting[i] = 0;
            }
        }
    }

    // error-check optional outputRouting request
    if(outputBusRouting && outputBusRouting[0] != '\0')
    {
        if(strlen(outputBusRouting) != nbusOut)
        {
            std::cerr << "Incorrect length " << strlen(outputBusRouting) 
                    << " for output channel routing (nbuses == " 
                    << nbusOut
                    << ")\n";
            outputBusRouting = nullptr;
        }
        else
        {
            nchanOut = 0;
            for(int i=0;i<nbusOut;i++)
            {
                int nch = outputBusRouting[i] - '0';
                nchanOut += nch;
                if(nchanOut <= 2)
                    busUsage.outputRouting[i] = nch;
                else
                {
                    std::cerr << "Custom output routing request exceeds channel allocation "
                        << outputBusRouting << "\n";
                    outputBusRouting = nullptr;
                    break;
                }
            }
        }
    }

    if(!outputBusRouting || *outputBusRouting == '\0')
    {
        nchanOut = 0;
        for(int j=0;j<2;j++) // prioritize main (j==0) over aux
        {
            for(int i=0;i<busUsage.outAudioChan.size();i++)
            {
                BusUsage::Bus &binfo = busUsage.outAudioChan[i];
                if(j == 0 && binfo.isAux)
                    continue;
                else
                if(j==1 && !binfo.isAux)
                    continue;
                int busch = binfo.nch;
                nchanOut += busch;
                if(nchanOut <= 2)
                    busUsage.outputRouting[i] = busch;
                else
                    busUsage.outputRouting[i] = 0;
            }
        }
    }

    // apply the routing requests ---------------------
    for(int i=0;i<nbusIn;i++)
    {
        int nch = busUsage.inputRouting[i];
        if(nch == 0)
        {
            inSA[i] = 0;
            std::cerr << "activate inbus " << i << "false\n";
            this->component->activateBus(Steinberg::Vst::kAudio,
                                    Steinberg::Vst::kInput, i, false);
        }
        else
        {
            int sa = 0;
            for(int k=0;k<nch;k++)
                sa |= (1 << k);
            inSA[i] = sa;
            std::cerr << "activate inbus " << i << "true\n";
            this->component->activateBus(Steinberg::Vst::kAudio,
                                    Steinberg::Vst::kInput, i, true);
        }
    }

    for(int i=0;i<nbusOut;i++)
    {
        int nch = busUsage.outputRouting[i];
        if(nch == 0)
        {
            outSA[i] = 0;
            std::cerr << "activate outbus " << i << "false\n";
            this->component->activateBus(Steinberg::Vst::kAudio,
                                    Steinberg::Vst::kOutput, i, false);
        }
        else
        {
            int sa = 0;
            for(int k=0;k<nch;k++)
                sa |= (1 << k);
            outSA[i] = sa;
            if(this->debug)
            {
                // stereo is 0x03  (ie: 0b00000011)
                std::cerr << "activate output bus " << i << " sa:" << sa << "\n";
            }
            this->component->activateBus(Steinberg::Vst::kAudio,
                                    Steinberg::Vst::kOutput, i, true);
        }
    }

    // Here we communicate the desired bus behavior of the effect.
    // The host should always deliver the same number of input and 
    // output busses that the plug-in needs. The plug-in has 3 
    // potential responses to our request:
    // 1. accept our request (return true)
    // 2. reject but attempt to negotiate (return false)
    // 3. reject outright, revert to default behavior (return false)
    std::cerr << "setBusArrangements in:(" << inSA.size() << "), out(" << outSA.size() <<")\n";
	Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor> processor = component;
    if(processor->setBusArrangements(
        inSA.size() ? &inSA[0] : nullptr, inSA.size(),
        outSA.size() ? &outSA[0] : nullptr , outSA.size())
        != Steinberg::kResultTrue)
    {
        if(this->verbosity)
        {
            // some plugins return error but seem to act "okay".
            std::cerr << "Problem configuring bus arrangement.\n";
        }
    }

    // event buses ------------------------------------------------------
    this->processData.busUsage.numInputEventBuses = this->component->getBusCount(
            Steinberg::Vst::kEvent, Steinberg::Vst::kInput);
    this->processData.busUsage.numOutputEventBuses = this->component->getBusCount(
            Steinberg::Vst::kEvent, Steinberg::Vst::kOutput);
    if(this->processData.busUsage.numInputEventBuses > 0)
    {
        if(this->verbosity)
        {
            std::cerr << "Input event busses\n";
            Steinberg::Vst::BusInfo binfo;
            for(int i=0;i<this->processData.busUsage.numInputEventBuses;i++)
            {
                if(this->component->getBusInfo(Steinberg::Vst::kEvent,
                    Steinberg::Vst::kInput, 0, binfo) == Steinberg::kResultTrue)
                {
                    auto busName = VST3::StringConvert::convert(binfo.name);
                    std::cerr << " - " << i << " " << busName << "\n";
                }
            }
        }
    }
    else
    {
        if(this->verbosity)
            std::cerr << "No event-in busses\n"; // usually accept events anyway
    }
    setEventBusState(true);
}

void
VST3PluginInstance::setEventBusState(bool enable)
{
    int inEvt = this->component->getBusCount(Steinberg::Vst::kEvent, Steinberg::Vst::kInput);
    int outEvt = this->component->getBusCount(Steinberg::Vst::kEvent, Steinberg::Vst::kOutput);

    if(this->debug)
    {
        std::cerr << this->provider->GetName() << " setEventBusState " 
            << enable << " nin:" << inEvt << " nout:" << outEvt << "\n";
    }

    for(int i = 0; i < inEvt; ++i) 
        this->component->activateBus(Steinberg::Vst::kEvent, Steinberg::Vst::kInput, i, enable);
    for(int i = 0; i < outEvt; ++i)
        this->component->activateBus(Steinberg::Vst::kEvent, Steinberg::Vst::kOutput, i, enable);
}

/* --------------------------------------------------------------------- */

tresult PLUGIN_API 
VST3PluginInstance::beginEdit(ParamID id)
{
    if(this->debug)
        std::cout << "beginEdit called " << id << "\n";
    return Steinberg::kNotImplemented;
}

tresult PLUGIN_API 
VST3PluginInstance::performEdit(ParamID id, ParamValue valueNormalized)
{
    if(this->debug)
    {
        // Happens when, eg, TALvocoder changes program.  Our job should
        // be to convey this information back to the host program.
        std::cout << "performEdit called " << id << " " << valueNormalized << "\n";
    }
    return Steinberg::kNotImplemented;
}

tresult PLUGIN_API 
VST3PluginInstance::endEdit(ParamID id)
{
    //std::cout << "endEdit called " << id << "\n";
    return Steinberg::kNotImplemented;
}

tresult PLUGIN_API 
VST3PluginInstance::queryInterface(const Steinberg::TUID iid, void** obj)
{
    // TUID's are 16-char arrays

    if(this->debug)
        dumpTUID("VST3PluginInstance query ", iid);

    QUERY_INTERFACE(iid, obj, Steinberg::Vst::IComponentHandler::iid, Steinberg::Vst::IComponentHandler);

    QUERY_INTERFACE(iid, obj, Steinberg::Vst::IComponentHandler2::iid, Steinberg::Vst::IComponentHandler2);

    QUERY_INTERFACE(iid, obj, Steinberg::Vst::IHostApplication::iid, Steinberg::Vst::IHostApplication);

    QUERY_INTERFACE(iid, obj, Steinberg::Vst::IUnitHandler::iid, Steinberg::Vst::IUnitHandler);

    QUERY_INTERFACE(iid, obj, Steinberg::FUnknown::iid, Steinberg::Vst::IComponentHandler);

    // nb: we could do more as in hostclasses.cpp
    dumpTUID("VST3PluginInstance doesn't support ", iid);
    *obj = nullptr;
    return Steinberg::kNoInterface;
}

/* from ComponentHandler2 ------------------------------------------- */
/** Tells host that the plug-in is dirty (something besides parameters 
 * has changed since last save), if true the host should apply a save 
 * before quitting. 
 */
tresult PLUGIN_API 
VST3PluginInstance::setDirty(Steinberg::TBool state)
{
    return Steinberg::kResultOk;
}

/** Tells host that it should open the plug-in editor the next time it's 
 * possible.  You should use this instead of showing an alert and blocking 
 * the program flow (especially on loading projects). 
 */
tresult PLUGIN_API 
VST3PluginInstance::requestOpenEditor(Steinberg::FIDString name)
{
    return Steinberg::kNotImplemented;
}

/** Starts the group editing (call before a \ref IComponentHandler::beginEdit),
 * the host will keep the current timestamp at this call and will use it for 
 * all IComponentHandler::beginEdit.
 */

tresult PLUGIN_API 
VST3PluginInstance::startGroupEdit()
{
    return Steinberg::kNotImplemented;
}

tresult PLUGIN_API 
VST3PluginInstance::finishGroupEdit()
{
    return Steinberg::kNotImplemented;
}

/* IUnitHandler API */
tresult PLUGIN_API 
VST3PluginInstance::notifyUnitSelection(Steinberg::Vst::UnitID id)
{
    std::cout << "unit selected: " << id << "\n";
    return Steinberg::kResultOk;
}

tresult PLUGIN_API 
VST3PluginInstance::notifyProgramListChange(
    Steinberg::Vst::ProgramListID listId, 
    Steinberg::int32 programIndex)
{

    std::cout << "notifyProgramListChange: " << listId 
                << " " << programIndex << "\n";
    return Steinberg::kResultOk;
}

/* --------------------------------------------------------------------- */
void
VST3PluginInstance::countChannels(Steinberg::Vst::MediaType media, 
    Steinberg::Vst::BusDirection dir, 
    std::vector<BusUsage::Bus> &chansPerBus,
    int &totalChannels)
{
    int nbus = this->component->getBusCount(media, dir);
    for(int i=0; i<nbus; ++i) 
    {
        Steinberg::Vst::BusInfo binfo;
        if(this->component->getBusInfo(media, dir, i, binfo) == Steinberg::kResultTrue)
        {
            BusUsage::Bus b;
            b.nch = binfo.channelCount;
            b.isAux = (binfo.busType == Steinberg::Vst::kAux);
            chansPerBus.push_back(b);
            totalChannels += b.nch;
        }
    }
}

/* fill parameters according to current plugin/controller state 
 *  - should be performed after we've synchronzized state.
 */
void
VST3PluginInstance::initParams(std::vector<ParamInfo> &parameters)
{
    assert(this->component); // managed by provider
    assert(this->controller); // managed by provider

    Steinberg::FUnknownPtr<Steinberg::Vst::IUnitInfo> unitInfo(controller);
    std::unordered_map<int, std::shared_ptr<std::vector<std::string>>> programMap;
    if(unitInfo)
    {
        Steinberg::int32 programListCount = unitInfo->getProgramListCount();
        if(programListCount > 0)
        {
            for(int i=0;i<programListCount;i++)
            {
                Steinberg::Vst::ProgramListInfo plInfo;
                if(unitInfo->getProgramListInfo(i, plInfo) == Steinberg::kResultOk)
                {
                    int plId = plInfo.id;
                    auto plName = VST3::StringConvert::convert(plInfo.name);

                    auto np = std::shared_ptr<std::vector<std::string>>(new std::vector<std::string>);
                    programMap[plId] = np;
                    // std::cerr << "programlist " << plName << " for " << plId << "-------------------------\n";
                    for(int j=0;j<plInfo.programCount; j++)
                    {
                        Steinberg::Vst::TChar pnm[256];
                        if(unitInfo->getProgramName(plId, j, pnm) == Steinberg::kResultOk)
                        {
                            if(pnm[0] == 0)
                                continue;
                            auto pnmStr = VST3::StringConvert::convert(pnm);
                            // std::cerr << "   " << pnmStr << "\n";
                            np->push_back(pnmStr);
                        }
                    }
                }
            }
        }
    }
    for(int i=0;i<controller->getParameterCount();i++)
    {
        Steinberg::Vst::ParameterInfo pinfo = {};
        Steinberg::tresult ret = controller->getParameterInfo(i, pinfo);
        if(ret != Steinberg::kResultOk)
        {
            std::cerr << "Parameter " << i << "has no info\n";
            continue;
        }
        if(pinfo.id < 0)
        {
            std::cerr << "Parameter " << i 
                << "has invalid id: " << pinfo.id << "\n";
            continue;
        }
        if(VST3::StringConvert::convert(pinfo.title).find("MIDI CC ")
            != std::string::npos)
            continue;

        ParamInfo paramInfo(pinfo);
        int programChangeIndex = -1;
        if(pinfo.flags & Steinberg::Vst::ParameterInfo::kIsProgramChange)
        {
            if(programMap.count(pinfo.id))
            {
                paramInfo.menuItems = programMap[pinfo.id];
                // std::cerr << paramInfo.name << " has " <<
                //   paramInfo.menuItems->size() << " menuitems\n";
            }
            else
            if(programChangeIndex == -1)
            {
                programChangeIndex = i;
                if(this->verbosity)
                    std::cerr << "Program-change index: " << i << "\n";
            }
            else
            {
                // this appears to be allowed if a module supports 
                // programlists (above).
                std::cerr << "Multiple program-changes? " << i << "\n";
            }
        }
        parameters.push_back(paramInfo); // issues copy-constructor
    }

    // this->activateMainIOBusses(vstPlug, false);
    // this->provider->releasePlugIn(this->component, this->controller);
}

tresult PLUGIN_API
VST3PluginInstance::getName(Steinberg::Vst::String128 name)
{
    return VST3::StringConvert::convert(this->host->GetName(), name) ? 
        Steinberg::kResultTrue : Steinberg::kInternalError;
}

tresult PLUGIN_API
VST3PluginInstance::createInstance(Steinberg::TUID cid, Steinberg::TUID iid, void** obj)
{
    Steinberg::FUID classID = Steinberg::FUID::fromTUID(cid);
    Steinberg::FUID interfaceID = Steinberg::FUID::fromTUID(iid);
    if(classID == Steinberg::Vst::IMessage::iid && 
        interfaceID == Steinberg::Vst::IMessage::iid)
    {
        if(this->debug)
            std::cerr << "VSTPluginInstance.createInstance HostMessage\n";
        *obj = new Steinberg::Vst::HostMessage;
        return Steinberg::kResultTrue;
    }
    else 
    if(classID == Steinberg::Vst::IAttributeList::iid && 
        interfaceID == Steinberg::Vst::IAttributeList::iid)
    {
        if(this->debug)
            std::cerr << "VSTPluginInstance.createInstance HostAttributeList\n";
        if(auto al = Steinberg::Vst::HostAttributeList::make())
        {
            *obj = al.take();
            return Steinberg::kResultTrue;
        }
        return Steinberg::kOutOfMemory;
    }
    std::cerr << "VST3PluginInstance::createInstance FAILED!!\n";
    *obj = nullptr;
    return Steinberg::kResultFalse;
}