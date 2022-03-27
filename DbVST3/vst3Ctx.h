#ifndef VST3Ctx_h
#define VST3Ctx_h

#include "host.h"
#include "module.h"
#include <unordered_map>

/* -------------------------------------------------------------------------- */

//  VST3Ctx is the primary handle that our clients have on a plugin file.
//  Since a plugin can have multiple interfaces/modules, we require
//  a nominal activeModule which can be selected by client.
// 
class VST3Ctx
{
private:
    VST3::Hosting::Module::Ptr plugin;
    std::string vendor;
    std::string filepath;
    std::vector<ModulePtr> modules;
    ModulePtr activeModule;
    int verbosity = 0;

public:
    VST3Ctx(VST3::Hosting::Module::Ptr p, std::string const &path)
    {
        this->plugin = p;
        this->filepath = p->getPath();
        if(this->filepath.size() == 0)
        {
            // std::cerr << "plugin doesn't know its path, using ours.\n";
            // happens on linux with plugins installed at ~/.vst3
            this->filepath = path;
        }
        auto factory = this->plugin->getFactory();
        auto finfo = factory.info();
        this->vendor = finfo.vendor();
        for(auto& classInfo : factory.classInfos())
        {
            if(classInfo.category() == kVstAudioEffectClass)
            {
                ModulePtr imod(new Module(classInfo, factory, verbosity));
                this->modules.emplace_back(imod);
            }
            else
            {
                // For now we disgregard anything != kVstAudioEffectClass
                // new Provider throws an error
                #if 0
                    std::cout << "  (skipping params for module of type " 
                        << imod->category << ")\n";
                #endif
            }
        }
        this->Finalize();
    }

    ~VST3Ctx()
    {
        // plugin should be cleaned up by ref-count
    }

    bool Ready()
    {
        return this->activeModule.get() != nullptr;
    }

    int ActivateModule(int index, float sampleRate)
    {
        if(this->modules.size() > index)
        {
            this->activeModule = this->modules[index];
            this->activeModule->SetVerbosity(this->verbosity);
            this->InitProcessing(sampleRate);
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

    void Finalize()
    {
        if(this->modules.size() > 0)
        {
            // nominate a main interface, user can select (by name) alternate
            this->activeModule = this->modules[0];
        }
    }

    void SetVerbosity(int v)
    {
        this->verbosity = v;
        if(this->activeModule)
            this->activeModule->SetVerbosity(this->verbosity);
    }

    void Print(std::ostream &ostr, bool detailed)
    {
        // output to yaml, as two objects:
        //   VST3Plugin:
        //      metainfo
        //   FiddleNodes:
        //      - array of modules with parameters, etc
        ostr << "VST3Plugin:\n";
        ostr << "  filepath: '" << this->filepath << "'\n";
        ostr << "  vendor: '" << this->vendor << "'\n";
        ostr << "  nmodules: " << this->modules.size() << "\n";
        if(!detailed)
            ostr << "Modules:\n";
        else
            ostr << "FiddleNodes:\n";
        for(int i=0;i<this->modules.size();i++)
            this->modules[i]->Print(ostr, "  ", i, detailed);
    }

    PluginInstance &getPluginInstance()
    {
        if(this->activeModule.get())
            return this->activeModule->pluginInstance;
        else
        {
            static PluginInstance s_pctx; // empty
            return s_pctx;
        }
    }
    
    int InitProcessing(float sampleRate, 
        char const *inputBusRouting = nullptr, 
        char const *outputBusRouting = nullptr)
    {
        PluginInstance &pctx = this->getPluginInstance();
        if(!pctx.error)
        {
            pctx.InitProcessing(sampleRate, inputBusRouting, outputBusRouting);
            // ~PluginInstance handles teardown
        }
        return pctx.error;
    }

    // input 'param id' from a user perspective is its index.
    int SetParamValue(int index, float val)
    {
        int err = -1;
        if(this->activeModule.get())
        {
            // pluginInstance's job to add ithe parameter change
            // to the automation setup.
            ParamInfo *info = this->activeModule->GetParamInfo(index);
            if(info)
            {
                this->setParamValue(info, val);
            }
            else
                std::cerr << "invalide parameter index " << index << "\n";
        }
        return err;
    }

    int SetParamValue(std::string const &nm, float val)
    {
        int err = -1;
        if(this->activeModule.get())
        {
            // pluginInstance's job to add the parameter change
            // to the automation setup.
            // std::cerr << "SetParameter parameter " << nm << "\n";
            ParamInfo *info = this->activeModule->GetParamInfo(nm);
            if(!info)
            {
                if(nm != "verbosity")
                    std::cerr << "DbVST3 unknown parameter " << nm << "\n";
                else
                    this->SetVerbosity((int) val);
                return -1;
            }
            else
                err = this->setParamValue(info, val);
        }
        else
        if(nm == "verbosity")
        {
            this->SetVerbosity((int) val);
        }
        else
        {
            std::cerr << "DbVST3 SetParameter " << nm << " called before moduleInit\n";
        }
        return err;
    }

    int MidiEvent(int status, int data1, int data2)
    {
        int err = -1;
        if(this->activeModule.get())
        {
            // pluginInstance's job to add the midi event to its eventslist
            err = this->getPluginInstance().MidiEvent(status, data1, data2);
        }
        return err;
    }

    void ProcessSamples(float *in, int inCh, float *out, int outCh, int nframes)
    {
        PluginInstance &pctx = this->getPluginInstance();
        if(pctx.error)
            return; // error reported earlier
        
        // midi-events and parameter value changes are applied to
        // processData when they arrive, but don't get processed 
        // 'til here.
        pctx.Process(in, inCh, out, outCh, nframes);
    }

private:
    int setParamValue(ParamInfo *info, float val)
    {
        auto id = info->id;
        int err = -1;
        if(info->programChange())
        {
            if(this->verbosity > 1)
                std::cerr << "ProgramChange " << val << "\n";
            err = this->getPluginInstance().SetParamValue(id, val, false);
        }
        else
        if(info->automatable()) // mda-PitchBend etc aren't automatable
        {
            if(this->verbosity > 1)
            {
                std::cerr << "SetParameter " << info->name << " " 
                    << val << "\n";
                    // " (id:" << info->id << ")\n";
            }
            err = this->getPluginInstance().SetParamValue(id, val, true);
        }
        else
        {
            if(this->verbosity > 1)
            {
                std::cerr << "SetParameter " << info->name << " " 
                    << val << " (id:" << info->id << ")\n";
            }
            err = this->getPluginInstance().SetParamValue(id, val, true);
            if(err)
                std::cerr << "parameter " << info->name << " can't be automated.\n";
        }
        return err;
    }

}; // end struct PluginCtx

#endif