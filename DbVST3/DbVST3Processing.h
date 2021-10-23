#ifndef DbVST3Processing_h
#define DbVST3Processing_h

#include "DbVST3ProcessData.h"
#include <public.sdk/source/common/memorystream.h>

class DbVST3ProcessingCtx : 
    public Steinberg::Vst::IComponentHandler,
	public Steinberg::Vst::IComponentHandler2,
	public Steinberg::Vst::IUnitHandler // notification of program change
    /*
	public Steinberg::IPlugFrame
    */
{
public:
    int error;
    int verbosity;

private:
    VST3App::ProviderPtr provider;
    Steinberg::Vst::IComponent* component; // aka vstPlug
    Steinberg::Vst::IEditController* controller;
    // Steinberg::FUnknownPtr<Steinberg::Vst::EditControllerEx1> controllerEx1;
    Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping> midiMapping;
    Steinberg::Vst::IAudioProcessor* audioEffect;
    std::vector<float> paramValues;
    Steinberg::Vst::ProcessSetup processSetup;
    DbVST3ProcessData processData; // our audio buffers are within
    bool activated;

public:
    using tresult = Steinberg::tresult;
    using ParamID = Steinberg::Vst::ParamID;
    using ParamValue = Steinberg::Vst::ParamValue;
    using int32 = Steinberg::int32;
    using uint32 = Steinberg::uint32;
    using TUID = Steinberg::TUID;

    DbVST3ProcessingCtx()
    {
        this->verbosity = 0;
        this->error = 0;
        this->component = nullptr;
        this->controller = nullptr;
        this->audioEffect = nullptr;
        this->activated = false;
    }

    ~DbVST3ProcessingCtx()
    {
        this->endProcessing();
    }

    void
    SetVerbosity(int v)
    {
        this->verbosity = v;
        this->processData.SetVerbosity(v);
    }

    /* -------------------------------------------------------------------- */
    VST3App::ProviderPtr 
    initProvider(
        const Steinberg::Vst::PlugProvider::PluginFactory &factory,
        VST3::Hosting::ClassInfo &classInfo)
    {
        this->provider = Steinberg::owned(
                new VST3App::Provider(factory, classInfo, 
                true/*useGlobalInstance*/));
        return this->provider;
    }

    bool
    synchronizeStates()
    {
        if(!this->component)
            this->initComponent();

        Steinberg::MemoryStream stream;
        if(this->component->getState(&stream) == Steinberg::kResultTrue) 
        {
            if(this->verbosity)
                std::cerr << "Read " << stream.getSize() << " bytes from component state\n";

            // Steinberg::int64 newpos;
            // stream.seek(0, Steinberg::IBStream::kIBSeekSet, &newpos);
            Steinberg::tresult res = this->controller->setComponentState(&stream);
            if(!(res == Steinberg::kResultOk || 
                 res == Steinberg::kNotImplemented))
            {
                if(this->verbosity)
                {
                    std::cerr << "Couldn't synchronize VST3 component with controller state\n";
                }
                this->readAllParameters(false);
            }
            return res == Steinberg::kResultOk;
		}
        else
        {
            std::cerr << "problem reading state from plugin\n";
            return false;
        }
	}

    void
    initComponent()
    {
        this->component = this->provider->getComponent();	
        this->controller = this->provider->getController();
        this->controller->setComponentHandler(this);
        this->midiMapping = Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping>(this->controller);

#if 0
        // mda-synth plus LABS both didn't allow
        Steinberg::FUnknownPtr<Steinberg::Vst::IEditControllerHostEditing> 
            host_editing(this->controller);
        if(host_editing)
            std::cerr << "HostEditing allowed\n";
        else
            std::cerr << "No HostEditing allowed\n";
#endif
    }

    void 
    beginProcessing(float sampleRate, 
                    char const *inputBusRouting, 
                    char const *outputBusRouting)
    {
        if(!this->component)
            this->initComponent();

        // this->controllerEx1 = Steinberg::FUnknownPtr<Steinberg::Vst::EditControllerEx1>(this->controller);
        if(this->verbosity)
            std::cerr << "initialize nparams: " << this->controller->getParameterCount() << "\n";

        if(Steinberg::kResultTrue != this->component->queryInterface(
                                        Steinberg::Vst::IAudioProcessor::iid, 
                                        (void**)&this->audioEffect))
        {
            std::cerr << "oops: no audioeffect here\n";
            this->error = 1;;
        }
        else
        {
            memset (&this->processSetup, 0, sizeof(Steinberg::Vst::ProcessSetup));
            this->processSetup.processMode = Steinberg::Vst::kRealtime;
                // from: kRealtime, kPrefetch, kOffline
            this->processSetup.symbolicSampleSize = Steinberg::Vst::kSample32;
                // from: kSample32, kSample64
            this->processSetup.sampleRate = sampleRate; // a double
            this->processSetup.maxSamplesPerBlock = 256; // usually 1

            // Our job is to deliver the number of busses required
            // by the plugin.  An audio bus can be mono or stereo and
            // indications are that we should also follow the guidance
            // of the plugin rather that request Speaker-Arrangement 
            // override by the host.

            if(this->audioEffect->setupProcessing(this->processSetup) 
                == Steinberg::kResultTrue)
            {
                // synchronizeState has already been called
                this->initBuses(inputBusRouting, outputBusRouting);
                // initialize process data (allocate AudioBuffers, etc)
                // nb: processData owns busUsage which we've just initialized above.
                this->processData.initialize(this->processSetup); 
                // syncState between controller and processor
                // via component->getState(&stream), stream.rewind
                //     controller->setComponentState()
            }
            else
            {
                std::cerr << "Problem setting up audioEffect\n";
            }
        }
    }

    void
    endProcessing()
    {
        if(this->audioEffect)
        {
            this->audioEffect->release();
            // buffers are freed in processData destructor
            // std::cout << "Unwinding!!!\n";
            this->audioEffect = nullptr;
        }
        #if 0
        if(this->controller)
        {
            this->controller->setComponentHandler(0);
            this->controller->terminate();
            this->controller->release();
        }
        if(this->component)
        {
            this->component->setActive(false); 
            this->component->terminate();
            this->component->release();
        }
        #else
        this->provider->releasePlugIn(this->component, this->controller);
        #endif
        this->controller = nullptr;
        this->component = nullptr;
    }

    bool activate()
    {
        if(this->activated)
            return true;
        Steinberg::tresult res = this->component->setActive(true);
        if (!(res == Steinberg::kResultOk || res == Steinberg::kNotImplemented)) 
            return false;

        res = this->audioEffect->setProcessing(true);
        if (!(res == Steinberg::kResultOk || res == Steinberg::kNotImplemented))
            return false;
        this->activated = true;
        return true;
    }

    bool deactivate()
    {
        if(!this->activated)
            return true;
        Steinberg::tresult res = this->audioEffect->setProcessing(false);
        if(!(res == Steinberg::kResultOk || res == Steinberg::kNotImplemented)) 
            return false;

        res = this->component->setActive(false);
        if(!(res == Steinberg::kResultOk || res == Steinberg::kNotImplemented))
            return false;

        this->activated = false;
        return true; 
    }

    void
    readAllParameters(bool andPush)
    {
        int nparams = this->controller->getParameterCount();
        this->paramValues.resize(nparams);
        for(int i=0;i<nparams;i++)
        {
            Steinberg::Vst::ParameterInfo info;
            this->controller->getParameterInfo(i, info);
            float val = this->controller->getParamNormalized(info.id);
            this->paramValues[i] = val;
            if(info.flags & Steinberg::Vst::ParameterInfo::kCanAutomate)
            {
                if(this->verbosity > 1)
                    std::cerr << i << ": " << val << "\n";
                this->SetParamValue(info.id, val, true);
            }
        }
    }

	tresult PLUGIN_API 
    restartComponent(int32 flags) override
    {
        bool forcePush = true;
        if(flags & Steinberg::Vst::kReloadComponent)
        {
            if(this->verbosity)
                std::cerr << "Reload component (program change)\n";
            forcePush = false;
            this->deactivate();
            this->activate();
        }
        if(flags & Steinberg::Vst::kParamValuesChanged)
        {
            if(this->verbosity)
                std::cerr << "ParamValuesChanged (program change)\n";
            this->readAllParameters(forcePush /* proven to be needed */);
            /* tried this .. 
                this->deactivate();
                this->activate();
             */
        }
        if(flags & Steinberg::Vst::kLatencyChanged)
        {
            if(this->verbosity)
                std::cerr << "LatencyChanged " << forcePush << "\n";
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
    initBuses(char const *inputBusRouting, char const *outputBusRouting)
    {
        // countChannels initializes busConfig.
        int nbusIn, nbusOut;
        this->processData.busUsage.Reset();

        /* audio buses -- */
        this->countChannels(Steinberg::Vst::kAudio, 
                            Steinberg::Vst::kInput, 
                            this->processData.busUsage.inAudioChan,
                            this->processData.busUsage.numInputChannels);
        nbusIn = this->processData.busUsage.inAudioChan.size();
        if(nbusIn > 0)
            this->processData.busUsage.inputRouting.resize(nbusIn, 0);

        this->countChannels(Steinberg::Vst::kAudio, 
                            Steinberg::Vst::kOutput, 
                            this->processData.busUsage.outAudioChan,
                            this->processData.busUsage.numOutputChannels);
        nbusOut = this->processData.busUsage.outAudioChan.size();
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

        DbVST3BusUsage &busUsage = this->processData.busUsage;
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
                    DbVST3BusUsage::Bus &binfo = busUsage.inAudioChan[i];
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
                    DbVST3BusUsage::Bus &binfo = busUsage.outAudioChan[i];
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
                this->component->activateBus(Steinberg::Vst::kAudio,
                                        Steinberg::Vst::kInput, i, false);
            }
            else
            {
                int sa = 0;
                for(int k=0;k<nch;k++)
                    sa |= (1 << k);
                inSA[i] = sa;
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
                this->component->activateBus(Steinberg::Vst::kAudio,
                                        Steinberg::Vst::kOutput, i, false);
            }
            else
            {
                int sa = 0;
                for(int k=0;k<nch;k++)
                    sa |= (1 << k);
                outSA[i] = sa;
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
        if(this->audioEffect->setBusArrangements(
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

        bool enable = true;
        int inEvt = this->component->getBusCount(Steinberg::Vst::kEvent, Steinberg::Vst::kInput);
	    int outEvt = this->component->getBusCount(Steinberg::Vst::kEvent, Steinberg::Vst::kOutput);
        for(int i = 0; i < inEvt; ++i) 
            this->component->activateBus(Steinberg::Vst::kEvent, Steinberg::Vst::kInput, i, enable);
        for(int i = 0; i < outEvt; ++i)
            this->component->activateBus(Steinberg::Vst::kEvent, Steinberg::Vst::kOutput, i, enable);

    }

    // 'param id' it's paramID !== index
    int SetParamValue(Steinberg::Vst::ParamID pid, float value, bool asAutomation)
    {
        // https://developer.steinberg.help/display/VST/Parameters+and+Automation
        int err = 0;
        if(this->controller)
        {
            if(asAutomation)
                this->processData.prepareParamChange(pid, value);
            else
                this->controller->setParamNormalized(pid, value);
        }
        else
            err = -1;
        return err;
    }

    Steinberg::Vst::ParamID
    GetMidiMapping(int data1)
    {
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

    int MidiEvent(int status, int data1, int data2)
    {
       int err;
        if(this->controller)
        {
            this->processData.prepareMidiEvent(status, data1, data2, 
                                            this->midiMapping);
            err = 0;
        }
        else
            err = -1;
       return err; 
    }

    void Process(float *in, int inCh, float *out, int outCh, int nframes)
    {
        this->activate(); 
        // beginAudioProcessing plugins in chuck pointers to the arrays
        this->processData.beginAudioProcessing(in, inCh, out, outCh, nframes);
        // active: true, processing: true
        VST3App::tresult result = this->audioEffect->process(this->processData);
        this->processData.endAudioProcessing();

        if(result != Steinberg::kResultOk)
            std::cerr << "Problem processing data...\n";

        // this->deactivate(); // only needed when we modify processMode, sampleRate,
        
    }

    /* --------------------------------------------------------------------- */

	tresult PLUGIN_API beginEdit(ParamID id) override
	{
		//std::cout << "beginEdit called " << id << "\n";
		return Steinberg::kNotImplemented;
	}
	tresult PLUGIN_API performEdit(ParamID id, ParamValue valueNormalized) override
	{
		//std::cout << "performEdit called " << id << " " << valueNormalized << "\n";
		return Steinberg::kNotImplemented;
	}
	tresult PLUGIN_API endEdit(ParamID id) override
	{
		//std::cout << "endEdit called " << id << "\n";
		return Steinberg::kNotImplemented;
	}

	tresult PLUGIN_API queryInterface(const Steinberg::TUID _iid, void** obj) override
	{
        QUERY_INTERFACE(_iid, obj, Steinberg::FUnknown::iid, Steinberg::Vst::IComponentHandler);
        QUERY_INTERFACE(_iid, obj, Steinberg::Vst::IComponentHandler::iid, Steinberg::Vst::IComponentHandler);
        QUERY_INTERFACE(_iid, obj, Steinberg::Vst::IComponentHandler2::iid, Steinberg::Vst::IComponentHandler2);
        // QUERY_INTERFACE(_iid, obj, Steinberg::Vst::IUnitHandler::iid, Steinberg::Vst::IUnitHandler);
        // QUERY_INTERFACE(_iid, obj, IPlugFrame::iid, IPlugFrame)
        *obj = nullptr;
        return Steinberg::kNoInterface;
	}
	uint32 PLUGIN_API addRef() override { return 1000; }
	uint32 PLUGIN_API release() override { return 1000; }

    /* from ComponentHandler2 ------------------------------------------- */
    /** Tells host that the plug-in is dirty (something besides parameters 
     * has changed since last save), if true the host should apply a save 
     * before quitting. 
     */
    tresult PLUGIN_API setDirty(Steinberg::TBool state) override
    {
        return Steinberg::kResultOk;
    }

    /** Tells host that it should open the plug-in editor the next time it's 
     * possible.  You should use this instead of showing an alert and blocking 
     * the program flow (especially on loading projects). 
     */
    tresult PLUGIN_API requestOpenEditor(Steinberg::FIDString name) override
    { 
        return Steinberg::kNotImplemented;
    }

    /** Starts the group editing (call before a \ref IComponentHandler::beginEdit),
     * the host will keep the current timestamp at this call and will use it for 
     * all IComponentHandler::beginEdit.
     */
    
    tresult PLUGIN_API startGroupEdit() override
    {
        return Steinberg::kNotImplemented;
    }

    tresult PLUGIN_API finishGroupEdit()  override
    {
        return Steinberg::kNotImplemented;
    }

    /* IUnitHandler API */
	tresult PLUGIN_API 
    notifyUnitSelection(Steinberg::Vst::UnitID id) override
    {
        std::cout << "unit selected: " << id << "\n";
        return Steinberg::kResultOk;
    }

	tresult PLUGIN_API 
    notifyProgramListChange(
        Steinberg::Vst::ProgramListID listId, 
        Steinberg::int32 programIndex) override
    {

        std::cout << "notifyProgramListChange: " << listId 
                  << " " << programIndex << "\n";
        return Steinberg::kResultOk;
    }

    /* --------------------------------------------------------------------- */
    void
    countChannels(Steinberg::Vst::MediaType media, 
        Steinberg::Vst::BusDirection dir, 
        std::vector<DbVST3BusUsage::Bus> &chansPerBus,
        int &totalChannels)
    {
        int nbus = this->component->getBusCount(media, dir);
        for(int i=0; i<nbus; ++i) 
        {
            Steinberg::Vst::BusInfo binfo;
            if(this->component->getBusInfo(media, dir, i, binfo) == Steinberg::kResultTrue)
            {
                DbVST3BusUsage::Bus b;
                b.nch = binfo.channelCount;
                b.isAux = (binfo.busType == Steinberg::Vst::kAux);
                chansPerBus.push_back(b);
                totalChannels += b.nch;
            }
        }
    }
};

#endif