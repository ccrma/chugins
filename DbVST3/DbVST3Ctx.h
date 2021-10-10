#ifndef DbVST3Ctx_h
#define DbVST3Ctx_h

#include "VST3App.h"

/* -------------------------------------------------------------------------- */
class DbVST3ProcessData : public Steinberg::Vst::ProcessData
{
private:
    Steinberg::Vst::ParameterChanges inPChanges;
    // Steinberg::Vst::EventList inEvents;

public:
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
    void initialize(Steinberg::Vst::ProcessSetup &pd, 
        int ninBusses, int noutBusses,
        int ninChan, int noutChan)
    {
        this->processMode = pd.processMode;
        this->symbolicSampleSize = pd.symbolicSampleSize;
        this->numInputs = ninBusses;
        this->numOutputs = noutBusses;
        // XXX: inputParameterChanges, inputEvents
        if(this->numInputs > 0)
        {
            this->inputs = new Steinberg::Vst::AudioBusBuffers[this->numInputs];
            for(int i=0;i<this->numInputs;i++) // foreach bus
            {
                this->inputs[i].numChannels = ninChan;
                this->inputs[i].channelBuffers32 = new float*[ninChan];
                for(int j=0;j<ninChan;j++)
                    this->inputs[i].channelBuffers32[j] = nullptr;
            }
        }
        if(this->numOutputs > 0)
        {
            this->outputs = new Steinberg::Vst::AudioBusBuffers[this->numOutputs];
            for(int i=0;i<this->numOutputs;i++) // foreach bus
            {
                this->outputs[i].numChannels = noutChan;
                this->outputs[i].channelBuffers32 = new float*[noutChan];
                for(int j=0;j<noutChan;j++)
                    this->outputs[i].channelBuffers32[j] = nullptr;
            }
        }
        this->inputParameterChanges = &this->inPChanges;

        #if 0
        this->inputEvents = &this->inEvents;
        #endif
    }

    void prepareParamChange(int paramId, float value)
    {
        Steinberg::int32 queueIndex;
        Steinberg::Vst::IParamValueQueue *q = 
            this->inPChanges.addParameterData((Steinberg::Vst::ParamID)paramId, 
                                              queueIndex);
        #if 0
        std::cout << "prepareParamChange " << paramId << " " << value 
            << " len:" <<  q->getPointCount() <<  "\n";
        #endif
        Steinberg::int32 ptIndex;
        q->addPoint(0, value, ptIndex);
    }

    void prepare(float *in, float *out, int nframes)
    {
        this->numSamples = nframes;
        if(nframes == 1)
        {
            // for now we'll only fill the first in and out bus
            // this approach uses ChucK's buffers
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
};

struct DbVST3ProcessingCtx
{
    DbVST3ProcessingCtx()
    {
        this->error = 0;
        this->vstPlug = nullptr;
        this->controller = nullptr;
        this->audioEffect = nullptr;
    }
    ~DbVST3ProcessingCtx()
    {
        if(this->audioEffect)
        {
            this->audioEffect->release();
            // buffers are freed in processData destructor
            // std::cout << "Unwinding!!!\n";
        }
        if(this->vstPlug && this->controller)
        {
            this->vstPlug->setActive(false); 
            this->provider->releasePlugIn(this->vstPlug, this->controller);
        }
    }

    void 
    initialize(int inNch, int outNch, float sampleRate)
    {
        this->controller = this->provider->getController();
        this->vstPlug = this->provider->getComponent();
        if(Steinberg::kResultTrue != this->vstPlug->queryInterface(
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

            // Try to set (host => plug-in) a wanted arrangement for 
            // inputs and outputs.  
            //
            // * an AudioEffect has input and/or output busses.
            // * each bus is comprised of 1 or more channels according
            //   to its speaker arrangement.
            //
            // ChucK configs we anticipate:
            //   * mono => mono
            //   * stereo => stereo
            //   * zero => mono (instrument)
            //   * zero => stereo (instrument)
            //
            // The host should always deliver the same number of input 
            // and output busses that the plug-in needs (see IComponent::getBusCount). 
            // The plug-in has 3 possibilities to react on this setBusArrangements 
            // call:
            //   * The plug-in accepts these arrangements, then it should modify, 
            //     if needed, its busses to match these new arrangements (later 
            //     on asked by the host with IComponent::getBusInfo () or 
            //     IAudioProcessor::getBusArrangement ()) and then should 
            //     return kResultTrue.
            //  * The plug-in does not accept or support these requested 
            //     arrangements for all inputs/outputs or just for some or 
            //     only one bus, but the plug-in can try to adapt its 
            //     current arrangements according to the requested ones 
            //     (requested arrangements for kMain busses should be 
            //     handled with more priority than the ones for kAux 
            //     busses), then it should modify its busses arrangements 
            //     and should return kResultFalse.
            //  * Same as the point 2 above the plug-in does not support 
            //    these requested arrangements but the plug-in cannot find 
            //    corresponding arrangements, the plug-in could keep its 
            //    current arrangement or fall back to a default arrangement 
            //    by modifying its busses arrangements and should return 
            //    kResultFalse.

            // prepareProcessing
            if(this->audioEffect->setupProcessing(this->processSetup) 
                == Steinberg::kResultTrue)
            {
                this->initBusses(inNch, outNch);
            }
            else
            {
                std::cerr << "Problem setting up audioEffect\n";
            }
        }
    }

    void initBusses(int inNch, int outNch)
    {
        // 0 means we own/manage the channel buffers
        // Each channel requires a separate array (can't be interleaved)
        // which means that if nframes != 1, we must

        // each bus can have an associated speaker arrangement.
        // our job is to produce a call to setBusArrangement
        using SA = Steinberg::Vst::SpeakerArrangement;
        SA wantin, wantout;
        if(inNch == 0)
            wantin = 0;
        else
        if(inNch == 1)
            wantin = Steinberg::Vst::SpeakerArr::kMono; // 0x01
        else
        if(inNch == 2)
            wantin = Steinberg::Vst::SpeakerArr::kStereo; //0x11 

        if(outNch == 1)
            wantout = Steinberg::Vst::SpeakerArr::kMono;
        else
        if(outNch == 2)
            wantout = Steinberg::Vst::SpeakerArr::kStereo;

        int nin = this->vstPlug->getBusCount(Steinberg::Vst::kAudio,
                                            Steinberg::Vst::kInput);
        int nout = this->vstPlug->getBusCount(Steinberg::Vst::kAudio,
                                            Steinberg::Vst::kOutput);

        auto inSpArrs = nin ? new SA[nin] : nullptr; // can be 0
        for(int i=0;i<nin;i++)
            inSpArrs[i] = wantin;

        auto outSpArrs = nout ? new SA[nout] : nullptr; // can be 0
        for(int i=0;i<nout;i++)
            outSpArrs[i] = wantout;
        
        if(this->audioEffect->setBusArrangements(inSpArrs, nin, 
            outSpArrs, nout) != Steinberg::kResultTrue)
        {
            std::cerr << "Problem configuring bus arrangement.\n";
            this->error = 1;
        }
        else
        {
            //std::cout << "Configuring vst3 plugs busses, in: " 
            // << wantin << " out: " << wantout << "\n";
            //  in: 3, out: 3 (means stereo i/o)
            this->processData.initialize(this->processSetup, 
                                nin, nout, inNch, outNch);
            for(int i=0; i<nin;i++)
            {
                this->vstPlug->activateBus(Steinberg::Vst::kAudio,
                                Steinberg::Vst::kInput, i, true);
            }
            for(int i=0; i<nout;i++)
            {
                this->vstPlug->activateBus(Steinberg::Vst::kAudio,
                                Steinberg::Vst::kOutput, i, true);
            }
        }
        if(inSpArrs)
            delete [] inSpArrs;
        if(outSpArrs)
            delete [] outSpArrs;

        // if our config is constant we can do this once 
        // upon initialization 
        this->vstPlug->setActive(true); 
    }

    int SetParamValue(int index, float value)
    {
        // https://developer.steinberg.help/display/VST/Parameters+and+Automation
        int err = 0;
        if(this->controller)
        {
            #if 0
            realvalue = controller->normalizedParamToPlain(index, value);
            std::cout << "nval:" << value << " rval:" << realvalue << "\n";
            controller->setParamNormalized(index, value);
            #else
            this->processData.prepareParamChange(index, value);
            #endif
        }
        else
            err = -1;
        return err;
    }

    int error;
    VST3App::ProviderPtr provider;
    Steinberg::Vst::IComponent* vstPlug;
    Steinberg::Vst::IEditController* controller;
    Steinberg::Vst::IAudioProcessor* audioEffect;
    Steinberg::Vst::ProcessSetup processSetup;
    //Steinberg::Vst::ProcessData processData; // constructor nulls out fields
    DbVST3ProcessData processData; // constructor nulls out fields
        // numChannels
        // silenceFlags
        // channelBuffers32
};

struct DbVST3ParamInfo
{
    std::string name;
    int stepCount; // 0: float, 1: toggle, 2: discreet
    int id;
    int flags;
    double defaultValue; // normalized, (double is the type of VST::ParamValue)
    std::string units; // XXX: two kinds of units (string and int)

    void Print(char const *indent, int index)
    {
        std::cout << indent << index << ": " << this->name << "\n";
        std::cout << indent << " default: " << this->defaultValue << "\n";
        std::cout << indent << " stepCount: " << this->stepCount << "\n";
        std::cout << indent << " id: " << this->id << "\n";
        std::cout << indent << " flags: " << this->flags << "\n";
        std::cout << indent << " units: '" << this->units << "'\n";
    }
};

// A plugin file (aka Steinberg::Vst::Module), can contain 1 or more
// interfaces (aka modules)
struct DbVST3Module 
{
    std::string name;
    std::string category; // "Audio Module Class" | "Component Controller Class"
    std::string subCategories;
    std::string version;
    std::string sdkVersion;
    std::vector<DbVST3ParamInfo> parameters;
    DbVST3ProcessingCtx processingCtx;

    void Print(char const *indent, int index, bool detailed)
    {
        std::cout << "-- " << index << ": " << this->name << " --\n";
        if(!detailed) return;
        std::cout << indent << "category: " << this->category << "\n";
        std::cout << indent << "subCategories: " << this->subCategories << "\n";
        std::cout << indent << "version: " << this->version << "\n";
        std::cout << indent << "sdkVersion: " << this->sdkVersion << "\n";
        std::cout << indent << "nparams: " << this->parameters.size() << "\n";
        std::string in(indent);
        in.append(indent);
        char const *i2 = in.c_str();
        for(int i=0; i<this->parameters.size();i++)
        {
            this->parameters[i].Print(i2, i);
        }
    }
}; // end struct DbVST3Module
typedef std::shared_ptr<DbVST3Module> DbVST3ModulePtr;


//  DbVST3Ctx is the primary handle that our clients have on a plugin file.
//  Since a plugin can have multiple interfaces/modules, We require
//  a nominal activeModule which can be selected by client.
// 
struct DbVST3Ctx
{
    VST3App::Plugin plugin;
    std::string vendor;
    std::string filepath;
    std::vector<DbVST3ModulePtr> modules;
    DbVST3ModulePtr activeModule;

    bool Ready()
    {
        return this->activeModule.get() != nullptr;
    }

    int ActivateModule(int index, int inCh, int outCh, float sampleRate)
    {
        if(this->modules.size() > index)
        {
            this->activeModule = this->modules[index];
            this->InitProcessing(inCh, outCh, sampleRate);
            return 0;
        }
        else
            return -1; // error
    }

    int GetNumModules()
    {
        return (int) this->modules.size();
    }

    std::string GetModuleName()
    {
        if(this->activeModule)
        {
            return this->activeModule->name;
        }
        else
            return std::string();
    }

    int GetNumParameters()
    {
        if(this->activeModule)
            return (int) this->activeModule->parameters.size();
        else
            return 0;
    }

    int GetParameterName(int index, std::string &nm)
    {
        int err = -1;
        if(this->activeModule.get())
        {
            err = 0;
            nm = this->activeModule->parameters[index].name;
        }
        return err;
    }

    void Reset()
    {
        vendor = "";
        filepath = "";
        modules.clear();
        activeModule.reset();
        plugin.reset(); // must be last
    }

    void Finalize()
    {
        if(this->modules.size() > 0)
        {
            // nominate a main interface, user can select (by name) alternate
            this->activeModule = this->modules[0];
        }
    }
    void Print(bool detailed)
    {
        std::cout << "--- VST3 plugin: " << this->filepath << "\n";
        std::cout << "vendor: " << this->vendor << "\n";
        std::cout << "nmodules: " << this->modules.size() << "\n";
        for(int i=0;i<this->modules.size();i++)
            this->modules[i]->Print("  ", i, detailed);
    }

    DbVST3ProcessingCtx &getProcessingCtx()
    {
        if(this->activeModule.get())
            return this->activeModule->processingCtx;
        else
        {
            static DbVST3ProcessingCtx s_pctx; // empty
            return s_pctx;
        }
    }
    
    int InitProcessing(int inNch, int outNch, float sampleRate)
    {
        DbVST3ProcessingCtx &pctx = this->getProcessingCtx();
        if(!pctx.vstPlug && !pctx.error)
        {
            pctx.initialize(inNch, outNch, sampleRate);
            // ~ProcessingCtx handles teardown
        }
        return pctx.error;
    }

    int SetParamValue(int index, float val)
    {
        int err = -1;
        if(this->activeModule.get())
        {
            // processingCtx's job to add ithe parameter change
            // to the automation setup.
            err = this->getProcessingCtx().SetParamValue(index, val);
        }
        return err;
    }

    void ProcessSamples(float *in, float *out, int nframes)
    {
        DbVST3ProcessingCtx &pctx = this->getProcessingCtx();
        if(pctx.error)
            return;
        
        // midi-events and parameter value changes are applied to
        // processData when they arrive, but don't get processed 
        // 'til here.
        pctx.processData.prepare(in, out, nframes);
        pctx.audioEffect->setProcessing(true);
        VST3App::tresult result = pctx.audioEffect->process(pctx.processData);
        if(result != Steinberg::kResultOk)
        {
            std::cerr << "Problem processing data...\n";
        }
        pctx.audioEffect->setProcessing(false);
    }
}; // end struct DbVST3Ctx

#endif