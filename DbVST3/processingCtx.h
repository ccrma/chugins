#ifndef ProcessingCtx_h
#define ProcessingCtx_h

#include "vst3.h"
#include "processingData.h"
#include "plugProvider.h"
#include "param.h"

#include <cassert>

// see comments in processingCtx.cpp
class ProcessingCtx : 
    public Steinberg::Vst::IComponentHandler,
	public Steinberg::Vst::IComponentHandler2,
	public Steinberg::Vst::IUnitHandler // notification of program change
    /*
	public Steinberg::IPlugFrame,
	public Steinberg::IContextInfoProvider3,
    */
{
public: 
    int error;

private:
    std::shared_ptr<dbPlugProvider> provider; // manages loading and cleanup of plugin
    Steinberg::Vst::IComponent* component;
    Steinberg::Vst::IEditController* controller;
    Steinberg::Vst::IAudioProcessor* audioEffect;
    // Steinberg::FUnknownPtr<Steinberg::Vst::EditControllerEx1> controllerEx1;
    Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping> midiMapping;
    std::vector<float> paramValues;
    Steinberg::Vst::ProcessSetup processSetup;
    ProcessingData processData; // our audio buffers are within
    bool activated;
    int debug;
    int verbosity;

public:
    ProcessingCtx();

    ~ProcessingCtx()
    {
        this->deinitProcessing();
    }

    void
    SetVerbosity(int v)
    {
        this->verbosity = v;
        this->processData.SetVerbosity(v);
    }

    /* -------------------------------------------------------------------- */
    void Init(const dbPlugProvider::PluginFactory &factory,
               VST3::Hosting::ClassInfo &classInfo,
               std::vector<ParamInfo> &params);

    void InitProcessing(float sampleRate, 
                char const *inputBusRouting, 
                char const *outputBusRouting);

    int SetParamValue(Steinberg::Vst::ParamID pid, float value, bool asAutomation);
    Steinberg::Vst::ParamID GetMidiMapping(int data1);
    int MidiEvent(int status, int data1, int data2);
    void Process(float *in, int inCh, float *out, int outCh, int nframes);

private:
    void deinitProcessing();
    bool synchronizeStates();
    void initComponent();
    bool activate();
    bool deactivate();
    void readAllParameters(bool andPush);
	tresult PLUGIN_API restartComponent(int32 flags) override;
    // tresult PLUGIN_API setComponentState (IBStream* stream) override

    // optional routing hints
    //  - string with len == num input or output bus.
    //  - in each position '0' || '1' || '2' represent channel allocation
    //    for each bus.  This differs for each component, so there is no
    //    universally correct answer.
    // default behavior:
    // - greedily allocate 2 channels prioritizing Main over Aux buses.
    void 
    initBuses(char const *inputBusRouting, char const *outputBusRouting);
    void setEventBusState(bool enable);

    /* --------------------------------------------------------------------- */
	tresult PLUGIN_API beginEdit(ParamID id) override;
	tresult PLUGIN_API performEdit(ParamID id, ParamValue valueNormalized) override;
	tresult PLUGIN_API endEdit(ParamID id) override;
	tresult PLUGIN_API queryInterface(const Steinberg::TUID _iid, void** obj) override;
	uint32 PLUGIN_API addRef() override { return 1000; }
	uint32 PLUGIN_API release() override { return 1000; }

    /* from ComponentHandler2 ------------------------------------------- */
    tresult PLUGIN_API setDirty(Steinberg::TBool state) override;

    tresult PLUGIN_API requestOpenEditor(Steinberg::FIDString name) override;

    tresult PLUGIN_API startGroupEdit() override;

    tresult PLUGIN_API finishGroupEdit() override;

	tresult PLUGIN_API notifyUnitSelection(Steinberg::Vst::UnitID id) override;

	tresult PLUGIN_API notifyProgramListChange(
        Steinberg::Vst::ProgramListID listId, 
        Steinberg::int32 programIndex) override;

    /* --------------------------------------------------------------------- */
    void
    countChannels(Steinberg::Vst::MediaType media, 
        Steinberg::Vst::BusDirection dir, 
        std::vector<BusUsage::Bus> &chansPerBus,
        int &totalChannels);

    void initParams(std::vector<ParamInfo> &parameters);
};

#endif