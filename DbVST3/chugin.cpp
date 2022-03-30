#include "chugin.h"
#include <functional>

static void deferredDelete(VST3Ctx *ctx)
{
    delete ctx;
}

VST3Chugin::VST3Chugin(t_CKFLOAT srate)
{
    m_sampleRate = srate;
    m_verbosity = 0;
    m_debug = 0;
    m_midiEvents = 0;
    m_vst3Ctx = nullptr;
    m_activeModule = -1;
}

VST3Chugin::~VST3Chugin()
{
#if 1 
    // verified to be required for LABS (hangs in the other variant)
    // implies some use of thread-local-store?
    std::function<void(void)> fn = std::bind(&deferredDelete, m_vst3Ctx);
    VST3Host::Singleton(true)->Delegate(fn);
    if(this->m_debug)
        std::cerr << "VST3Chugin deleted (defer VST3Ctx)\n";
#else
    delete m_vst3Ctx;
    if(this->m_debug)
        std::cerr << "VST3Chugin deleted (including VST3Ctx)\n";
#endif
}

bool VST3Chugin::loadPlugin(const std::string& filepath)
{
    if(m_vst3Ctx)
    {
        delete m_vst3Ctx;
        m_vst3Ctx = nullptr;
    }
    auto fn = std::bind(&VST3Chugin::onPluginLoaded, this, std::placeholders::_1);
    VST3Host::Singleton(true)->OpenPlugin(filepath, fn, this->m_verbosity);
    return true; // XXX async open has no response
}

bool VST3Chugin::ready()
{
    return this->m_vst3Ctx != nullptr && this->m_vst3Ctx->Ready();
}

void
VST3Chugin::onPluginLoaded(VST3Ctx *ctx)
{
    // NB: this runs in the "workerthread", not the chugin/audio thread.
    // so as to not require activateModule, we activateModule(0)
    // since it's the 95% case.
    m_vst3Ctx = ctx;
    if(m_vst3Ctx)
        this->selectModule(0);
}

void
VST3Chugin::setVerbosity(int v)
{
    if(!m_vst3Ctx) return;
    m_verbosity = v;
    m_vst3Ctx->SetVerbosity(v);
}

void 
VST3Chugin::printModules()
{
    if(!m_vst3Ctx) return;
    m_vst3Ctx->Print(std::cout, false/*detailed*/);
}

int 
VST3Chugin::getNumModules()
{
    if(!m_vst3Ctx) return 0;
    return m_vst3Ctx->GetNumModules();
}

int 
VST3Chugin::selectModule(int m)
{
    if(!m_vst3Ctx) return 0;
    if(m != m_activeModule)
    {
        int err = m_vst3Ctx->ActivateModule(m, m_sampleRate); 
        if(!err)
            m_activeModule = m;
        return err;
    }
    else
        return 0; // already active
}

std::string 
VST3Chugin::getModuleName()
{
    if(!m_vst3Ctx) return std::string("<no module>");
    return m_vst3Ctx->GetModuleName();
}

int 
VST3Chugin::getNumParameters() 
{
    if(!m_vst3Ctx) return 0;
    return m_vst3Ctx->GetNumParameters();
}

int 
VST3Chugin::getParameterName(int index, std::string &nm) 
{
    if(!m_vst3Ctx) return -1;
    return m_vst3Ctx->GetParameterName(index, nm);
}

float 
VST3Chugin::getParameter(int index) 
{
    return 0.f;
}

bool 
VST3Chugin::setParameter(int index, float v) 
{
    if(!m_vst3Ctx) return false;
    return m_vst3Ctx->SetParamValue(index, v);
}

bool 
VST3Chugin::setParameter(std::string const &nm, float v) 
{
    if(!m_vst3Ctx) return false;
    return m_vst3Ctx->SetParamValue(nm, v);
}

void
VST3Chugin::setInputRouting(std::string const &r)
{
    if(!m_vst3Ctx) return;
    this->m_inputBusRouting = r;
}

void
VST3Chugin::setOutputRouting(std::string const &r)
{
    if(!m_vst3Ctx) return;
    this->m_outputBusRouting = r;
}

/* --------------------------------------------------------------------- */
bool 
VST3Chugin::noteOn(int noteNumber, float velocity) 
{
    // 144 is channel 0
    this->midiEvent(144, noteNumber,  int(velocity * 127), 0.);
    return true;
}

bool 
VST3Chugin::noteOff(int noteNumber, float velocity) 
{
    // 128 is channel 0
    this->midiEvent(128, noteNumber,  int(velocity * 127), 0.);
    return true;
}

bool
VST3Chugin::midiEvent(int status, int data1, int data2, float dur)
{
    // this->m_midiEvents++;
    // std::cout << "VST3Chugin::midiEvent " /*  */<< status << " " << data1 
     // << " " << data2 << " " << dur << " " << this->m_midiEvents << "\n"; 
    // dur is zero unless playing from file
    if(!m_vst3Ctx) return false;
    return m_vst3Ctx->MidiEvent(status, data1, data2);
}

/* -------------------------------------------------------------------- */
void 
VST3Chugin::multitick(SAMPLE* in, SAMPLE* out, int nframes)
{
    if(!m_vst3Ctx || !m_vst3Ctx->Ready())
    {
        // fprintf(stderr, "dbVST3Ctx isn't ready\n");
        if(nframes == 1) // actual case
        {
            out[0] = 0.f;
            out[1] = 0.f;
        }
        else
        {
            for(int i = 0; i < nframes; i++)
            {
                out[i * 2] = 0.f;
                out[i * 2 + 1] = 0.f;
            }
        }
        return;
    }
    else
    {
        // fprintf(stderr, "dbVST3Ctx process\n");
        m_vst3Ctx->ProcessSamples(in, 2, out, 2, nframes);
    }
}
