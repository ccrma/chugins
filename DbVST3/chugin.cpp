#include "chugin.h"

std::shared_ptr<Host> VST3Chugin::s_hostPtr; // shared across multiple instances

bool VST3Chugin::loadPlugin(const std::string& filepath)
{
    if(m_pluginCtx)
    {
        delete m_pluginCtx;
        m_pluginCtx = nullptr;
    }
    m_pluginCtx = s_hostPtr->OpenPlugin(filepath, this->m_verbosity);
    // so as to not require activateModule, we activateModule(0)
    // since it's the 95% case.
    if(m_pluginCtx)
        this->selectModule(0);
    return m_pluginCtx != nullptr;
}

void
VST3Chugin::setVerbosity(int v)
{
    m_verbosity = v;
    m_pluginCtx->SetVerbosity(v);
}

void 
VST3Chugin::printModules()
{
    m_pluginCtx->Print(std::cout, false/*detailed*/);
}

int 
VST3Chugin::getNumModules()
{
    return m_pluginCtx->GetNumModules();
}

int 
VST3Chugin::selectModule(int m)
{
    if(m != m_activeModule)
    {
        int err = m_pluginCtx->ActivateModule(m, m_sampleRate); 
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
    return m_pluginCtx->GetModuleName();
}

int 
VST3Chugin::getNumParameters() 
{
    return m_pluginCtx->GetNumParameters();
}

int 
VST3Chugin::getParameterName(int index, std::string &nm) 
{
    return m_pluginCtx->GetParameterName(index, nm);
}

float 
VST3Chugin::getParameter(int index) 
{
    return 0.f;
}

bool 
VST3Chugin::setParameter(int index, float v) 
{
    return m_pluginCtx->SetParamValue(index, v);
}

bool 
VST3Chugin::setParameter(std::string const &nm, float v) 
{
    return m_pluginCtx->SetParamValue(nm, v);
}

void
VST3Chugin::setInputRouting(std::string const &r)
{
    this->m_inputBusRouting = r;
}

void
VST3Chugin::setOutputRouting(std::string const &r)
{
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
    return m_pluginCtx->MidiEvent(status, data1, data2);
}

/* -------------------------------------------------------------------- */
void 
VST3Chugin::multitick(SAMPLE* in, SAMPLE* out, int nframes)
{
    if(!m_pluginCtx->Ready())
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
        m_pluginCtx->ProcessSamples(in, 2, out, 2, nframes);
    }
}
