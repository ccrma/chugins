#ifndef DbVST3Ctx_h
#define DbVST3Ctx_h

#include "VST3App.h"

/* -------------------------------------------------------------------------- */
class DbVST3ProcessData : public Steinberg::Vst::ProcessData
{
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
        if(this->vstPlug && this->controller)
            this->provider->releasePlugIn(this->vstPlug, this->controller);
        if(this->audioEffect)
        {
            this->audioEffect->release();
            // buffers are freed in processData destructor
            // std::cout << "Unwinding!!!\n";
        }
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
}; // end struct DBVST3Module

//  DbVST3Ctx is the primary handle that our clients have on a plugin file.
//  Since a plugin can have multiple interfaces/modules, We require
//  a nominal activeModule which can be selected by client.
// 
struct DbVST3Ctx
{
    VST3App::Plugin plugin;
    std::string vendor;
    std::string filepath;
    std::vector<DbVST3Module> modules;
    DbVST3Module *activeModule;

    bool Ready()
    {
        return this->activeModule != nullptr;
    }

    int ActivateModule(int index)
    {
        if(this->modules.size() > index)
        {
            this->activeModule = &this->modules[index];
            return 0;
        }
        else
            return -1; // error
    }

    int GetNumModules()
    {
        return (int) this->modules.size();
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
        if(this->activeModule)
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
        activeModule = nullptr;
        plugin.reset(); // must be last
    }

    void Finalize()
    {
        if(this->modules.size() > 0)
        {
            // nominate a main interface, user can select (by name) alternate
            this->activeModule = &this->modules[0];
        }
    }
    void Print(bool detailed)
    {
        std::cout << "--- VST3 plugin: " << this->filepath << "\n";
        std::cout << "vendor: " << this->vendor << "\n";
        std::cout << "nmodules: " << this->modules.size() << "\n";
        for(int i=0;i<this->modules.size();i++)
            this->modules[i].Print("  ", i, detailed);
    }

    DbVST3ProcessingCtx &getProcessingCtx()
    {
        if(this->activeModule)
            return this->activeModule->processingCtx;
        else
        {
            static DbVST3ProcessingCtx s_pctx; // empty
            return s_pctx;
        }
    }

    void ProcessSamples(float *in, float *out, 
        int inNch, int outNch, int nframes, float sampleRate)
    {
        DbVST3ProcessingCtx &pctx = this->getProcessingCtx();
        if(!pctx.vstPlug && !pctx.error)
        {
            pctx.controller = pctx.provider->getController();
            pctx.vstPlug = pctx.provider->getComponent();
		    if(Steinberg::kResultTrue != pctx.vstPlug->queryInterface(
                                            Steinberg::Vst::IAudioProcessor::iid, 
                                            (void**)&pctx.audioEffect))
            {
                std::cerr << "oops: no audioeffect here\n";
                pctx.error = 1;;
            }
            else
            {
                memset (&pctx.processSetup, 0, sizeof(Steinberg::Vst::ProcessSetup));
                pctx.processSetup.processMode = Steinberg::Vst::kRealtime;
                    // from: kRealtime, kPrefetch, kOffline
                pctx.processSetup.symbolicSampleSize = Steinberg::Vst::kSample32;
                    // from: kSample32, kSample64
                pctx.processSetup.sampleRate = sampleRate; // a double
                pctx.processSetup.maxSamplesPerBlock = 256; // usually 1

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
                if(pctx.audioEffect->setupProcessing(pctx.processSetup) 
                    == Steinberg::kResultTrue)
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

                    int nin = pctx.vstPlug->getBusCount(Steinberg::Vst::kAudio,
                                                        Steinberg::Vst::kInput);
                    int nout = pctx.vstPlug->getBusCount(Steinberg::Vst::kAudio,
                                                        Steinberg::Vst::kOutput);

                    auto inSpArrs = nin ? new SA[nin] : nullptr; // can be 0
                    for(int i=0;i<nin;i++)
                        inSpArrs[i] = wantin;

                    auto outSpArrs = nout ? new SA[nout] : nullptr; // can be 0
                    for(int i=0;i<nout;i++)
                        outSpArrs[i] = wantout;
                    
                    if(pctx.audioEffect->setBusArrangements(inSpArrs, nin, 
                        outSpArrs, nout) != Steinberg::kResultTrue)
                    {
                        std::cerr << "Problem configuring bus arrangement.\n";
                        pctx.error = 1;
                    }
                    else
                    {
                        //std::cout << "Configuring vst3 plugs busses, in: " 
                        // << wantin << " out: " << wantout << "\n";
                        //  in: 3, out: 3 (means stereo i/o)
                        pctx.processData.initialize(pctx.processSetup, 
                                            nin, nout, inNch, outNch);
                        for(int i=0; i<nin;i++)
                        {
                            pctx.vstPlug->activateBus(Steinberg::Vst::kAudio,
                                            Steinberg::Vst::kInput, i, true);
                        }
                        for(int i=0; i<nout;i++)
                        {
                            pctx.vstPlug->activateBus(Steinberg::Vst::kAudio,
                                            Steinberg::Vst::kOutput, i, true);
                        }
                    }
                    if(inSpArrs)
                        delete [] inSpArrs;
                    if(outSpArrs)
                        delete [] outSpArrs;
                }
                else
                {
                    std::cerr << "Problem setting up audioEffect\n";
                }
            }
            // ~ProcessingCtx handles teardown
        }
        if(pctx.error)
            return;
        
        pctx.processData.prepare(in, out, nframes);
        // pctx.vstPlug->setActive(true); // if our config is constant we can do this once upon initialization 
        pctx.audioEffect->setProcessing(true);
        VST3App::tresult result = pctx.audioEffect->process(pctx.processData);
        if(result != Steinberg::kResultOk)
        {
            std::cerr << "Problem processing data...";
        }
        pctx.audioEffect->setProcessing(false);
        // pctx.vstPlug->setActive(false); // see above
    }
}; // end struct DbVST3Ctx

#endif