#ifndef vst3Chugin_h
#define vst3Chugin_h

#include "chuck_type.h"
#include "host.h"
#include "vst3Ctx.h"

class VST3Chugin
{
private:
    static std::unique_ptr<Host> s_hostPtr; // shared across multiple instances

public:
    // constructor
    VST3Chugin(t_CKFLOAT srate);
    ~VST3Chugin();

    bool loadPlugin(const std::string& filename);
    bool ready();

    void printModules();
    void setVerbosity(int);
    int getNumModules();
    int selectModule(int index); // returns 0 on success
    std::string getModuleName(); // of current module
    void onPluginLoaded(VST3Ctx *ctx);

    int getNumParameters();
    int getParameterName(int index, std::string &pnm);
    float getParameter(int index);
    bool setParameter(int index, float v);
    bool setParameter(std::string const &nm, float v);

    bool noteOn(int note, float velocity);
    bool noteOff(int note, float velocity);
    bool midiEvent(int data1, int data2, int data3, float when);

    void multitick(SAMPLE* in, SAMPLE* out, int nframes);

    // experimental
    void setInputRouting(std::string const &r);
    void setOutputRouting(std::string const &r);

private:
    int m_verbosity;
    t_CKFLOAT m_sampleRate;
    std::string m_pluginPath;
    VST3Ctx *m_vst3Ctx;
    int m_midiEvents;
    int m_activeModule;

    // i/o routing:
    // usually empty, otherwise: nch for _bus_
    // eg:  "11" for TAL-Vocoder input to use the sidechain.
    //      "20" is the default behavior which "should" work for L+R mode (but doesn't?)
    std::string m_inputBusRouting;
    std::string m_outputBusRouting;
    // VST3 plugins have:
    //  0 or more input Audio busses, mono or multichan.
    //  1 or more output Audio busses, each with either mono 
    //    or multichannel support - aka Speaker arrangement.
    //   
    // Currently we register ourselves as requiring two inputs 
    // and two outputs.  Details of mono-vs-stereo managed by
    // InitProcessing, ProcessSamples
};

#endif