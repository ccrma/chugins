#ifndef DbVST3Processing_h
#define DbVST3Processing_h

#include "DbVST3ProcessData.h"

#ifndef VERBOSE
#define VERBOSE 0
#endif

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
    DbVST3ProcessData::BusUsage busUsage;
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

    void 
    beginProcessing(float sampleRate)
    {
        this->controller = this->provider->getController();
        this->controller->setComponentHandler(this);
        this->component = this->provider->getComponent();	
        this->midiMapping = Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping>(this->controller);
        // this->controllerEx1 = Steinberg::FUnknownPtr<Steinberg::Vst::EditControllerEx1>(this->controller);
        #if VERBOSE
        std::cout << "initialize nparams: " << this->controller->getParameterCount() << "\n";
        #endif

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
                this->initBuses();
                this->readAllParameters(false/*don't push*/);

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
            if(andPush && (info.flags & Steinberg::Vst::ParameterInfo::kCanAutomate))
                this->SetParamValue(info.id, val, true);
        }
    }

	tresult PLUGIN_API 
    restartComponent(int32 flags) override
    {
        bool forcePush = true;
        if(flags & Steinberg::Vst::kReloadComponent)
        {
            std::cout << "Reload component (program change)\n";
            forcePush = false;
            this->deactivate();
            this->activate();
        }
        if(flags & Steinberg::Vst::kParamValuesChanged)
        {
            std::cout << "ParamValuesChanged (program change)\n";
            this->readAllParameters(forcePush /* proven to be needed */);
            /* tried this .. 
                this->deactivate();
                this->activate();
             */
        }
        if(flags & Steinberg::Vst::kLatencyChanged)
        {
            std::cout << "latency change " << forcePush << "\n";
            this->deactivate();
            this->activate();
        }
		return Steinberg::kResultOk;
    }

    void 
    initBuses()
    {
        // countChannels builds up BusConfig.
        this->busUsage.Reset();
        this->countChannels(Steinberg::Vst::kAudio, 
                            Steinberg::Vst::kInput, 
                            this->busUsage.inAudioChan,
                            this->busUsage.numInputChannels);
        this->countChannels(Steinberg::Vst::kAudio, 
                            Steinberg::Vst::kOutput, 
                            this->busUsage.outAudioChan,
                            this->busUsage.numOutputChannels);
        this->busUsage.numInputEventBuses = this->component->getBusCount(
                Steinberg::Vst::kEvent, Steinberg::Vst::kInput);
        this->busUsage.numOutputEventBuses = this->component->getBusCount(
                Steinberg::Vst::kEvent, Steinberg::Vst::kOutput);

        if(this->busUsage.numInputEventBuses > 0)
        {
            #if VERBOSE
            std::cout << this->busUsage.numInputEventBuses 
                      << " input event busses\n";
            Steinberg::Vst::BusInfo binfo;
			if(this->component->getBusInfo(Steinberg::Vst::kEvent,
                Steinberg::Vst::kInput, 0, binfo) == Steinberg::kResultTrue)
			{
                auto busName = VST3::StringConvert::convert(binfo.name);
                std::cout << "Event bus 0: " << busName << "\n";
            }
            #endif
        }
        else
            std::cerr << "No event-in busses\n";

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

        std::vector<Steinberg::Vst::SpeakerArrangement> inSA;
        std::vector<Steinberg::Vst::SpeakerArrangement> outSA;

        inSA.resize(this->busUsage.inAudioChan.size());
        outSA.resize(this->busUsage.outAudioChan.size());

        int inch = 0;
        for(int j=0;j<2;j++) // prioritize main (j==0) over aux
        {
            for(int i=0;i<this->busUsage.inAudioChan.size();i++)
            {
                int cinfo = this->busUsage.inAudioChan[i];
                if(j==0 && this->busUsage.IsAux(cinfo))
                    continue;

                if(j==1 && !this->busUsage.IsAux(cinfo))
                    continue;
                int busch = this->busUsage.GetNChan(cinfo);
                inch += busch;
                if(inch <= 2)
                {
                    int sa = 0;
                    for(int k=0;k<busch;k++)
                        sa |= (1 << k);
                    inSA[i] = sa;
                    this->busUsage.activeInputBuses.push_back(i);
                    this->component->activateBus(Steinberg::Vst::kAudio,
                                    Steinberg::Vst::kInput, i, true);
                }
                else
                {
                    inSA[i] = 0;
                    this->component->activateBus(Steinberg::Vst::kAudio,
                                    Steinberg::Vst::kInput, i, false);
                }
            }
        }

        int outch = 0;
        for(int j=0;j<2;j++) // prioritize main (j==0) over aux
        {
            for(int i=0;i<this->busUsage.outAudioChan.size();i++)
            {
                int cinfo = this->busUsage.outAudioChan[i];
                if(j==0 && this->busUsage.IsAux(cinfo))
                    continue;
                else
                if(j==1 && !this->busUsage.IsAux(cinfo))
                    continue;
                int busch = this->busUsage.GetNChan(cinfo);
                outch += busch;
                if(outch <= 2)
                {
                    int sa = 0;
                    for(int k=0;k<busch;k++)
                        sa |= (1 << k);
                    outSA[i] = sa;
                    this->busUsage.activeOutputBuses.push_back(i);
                    this->component->activateBus(Steinberg::Vst::kAudio,
                                    Steinberg::Vst::kOutput, i, true);
                }
                else
                {
                    outSA[i] = 0;
                    this->component->activateBus(Steinberg::Vst::kAudio,
                                    Steinberg::Vst::kOutput, i, false);
                }
            }
        }

        #if VERBOSE || 1
        std::cerr << "Configuring audio buses, in:" << inSA.size()
                  << " out:" << outSA.size() << "\n";
        std::cerr << "Configure audio channels, in:" << inch
                  << " out:" << outch << "\n";
        #endif
        if(this->audioEffect->setBusArrangements(
            inSA.size() ? &inSA[0] : nullptr, inSA.size(),
            outSA.size() ? &outSA[0] : nullptr , outSA.size())
            != Steinberg::kResultTrue)
        {
            #if VERBOSE
            // some plugins return error but seem to act "okay".
            std::cerr << "Problem configuring bus arrangement.\n";
            #endif
        }

        this->processData.initialize(this->processSetup,  &this->busUsage);
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
            int bus=0,ch=0;
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
        this->processData.beginAudioProcessing(in, inCh, out, outCh, nframes);
        this->audioEffect->setProcessing(true);
        VST3App::tresult result = this->audioEffect->process(this->processData);
        this->processData.endAudioProcessing();
        this->audioEffect->setProcessing(false);
        if(result != Steinberg::kResultOk)
            std::cerr << "Problem processing data...\n";
    }

    /* --------------------------------------------------------------------- */

	tresult PLUGIN_API beginEdit(ParamID id) override
	{
		std::cout << "beginEdit called " << id << "\n";
		return Steinberg::kNotImplemented;
	}
	tresult PLUGIN_API performEdit(ParamID id, ParamValue valueNormalized) override
	{
		std::cout << "performEdit called " << id << " " << valueNormalized << "\n";
		return Steinberg::kNotImplemented;
	}
	tresult PLUGIN_API endEdit(ParamID id) override
	{
		std::cout << "endEdit called " << id << "\n";
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
        std::vector<int> &chansPerBus,
        int &totalChannels)
    {
        int nbus = this->component->getBusCount(media, dir);
        for(int i=0; i<nbus; ++i) 
        {
            Steinberg::Vst::BusInfo binfo;
            if(this->component->getBusInfo(media, dir, i, binfo) == Steinberg::kResultTrue)
            {
                int nch = binfo.channelCount;
                if(binfo.busType == Steinberg::Vst::kAux)
                    nch = this->busUsage.SetAuxBit(nch);
                chansPerBus.push_back(nch);
                totalChannels += binfo.channelCount; // nch may have aux bit set
            }
        }
    }
};

#endif