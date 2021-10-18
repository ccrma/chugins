#ifndef DbVST3Ctx_h
#define DbVST3Ctx_h

#include "VST3App.h"
#include "DbVST3Module.h"
#include <unordered_map>

/* -------------------------------------------------------------------------- */

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
    int verbosity = 0;

    bool Ready()
    {
        return this->activeModule.get() != nullptr;
    }

    int ActivateModule(int index, float sampleRate)
    {
        if(this->modules.size() > index)
        {
            this->activeModule = this->modules[index];
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

    void SetVerbosity(int v)
    {
        this->verbosity = v;
    }

    void Print(std::ostream &ostr, bool detailed)
    {
        // output to yaml, as two objects:
        //   VST3Plugin:
        //      metainfo
        //   FiddleNodes:
        //      - array of modules with parameters, etc
        ostr << "VST3Plugin:\n";
        ostr << "  filepath: " << this->filepath << "\n";
        ostr << "  vendor: " << this->vendor << "\n";
        ostr << "  nmodules: " << this->modules.size() << "\n";
        if(!detailed)
            ostr << "Modules:\n";
        else
            ostr << "FiddleNodes:\n";
        for(int i=0;i<this->modules.size();i++)
            this->modules[i]->Print(ostr, "  ", i, detailed);
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
    
    int InitProcessing(float sampleRate)
    {
        DbVST3ProcessingCtx &pctx = this->getProcessingCtx();
        if(!pctx.error)
        {
            pctx.beginProcessing(sampleRate);
            // ~ProcessingCtx handles teardown
        }
        return pctx.error;
    }

    // input 'param id' from a user perspective is its index.
    int SetParamValue(int index, float val)
    {
        int err = -1;
        if(this->activeModule.get())
        {
            // processingCtx's job to add ithe parameter change
            // to the automation setup.
            DbVST3ParamInfo const *info = this->activeModule->GetParamInfo(index);
            if(info)
            {
                auto id = info->id;
                if(info->automatable)
                    err = this->getProcessingCtx().SetParamValue(id, val, true);
                else
                if(!info->readOnly) // program change
                {
                    err = this->getProcessingCtx().SetParamValue(id, val, false);
                }
                else
                    std::cerr << "parameter " << index << " can't be automated.\n";
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
            // processingCtx's job to add the parameter change
            // to the automation setup.
            DbVST3ParamInfo const *info = this->activeModule->GetParamInfo(nm);
            if(!info)
            {
                std::cerr << "Unknown parameter " << nm << "\n";
                return -1;
            }
            if(this->verbosity)
                std::cerr << "Parameter " << nm << " found as id: " << info->id << "\n";

            if(info->automatable)
                err = this->getProcessingCtx().SetParamValue(info->id, val, true);
            else
            if(!info->readOnly) // a program change
                err = this->getProcessingCtx().SetParamValue(info->id, val, false);
            else
                std::cerr << "parameter " << nm << " can't be automated.\n";
        }
        return err;
    }

    int MidiEvent(int status, int data1, int data2)
    {
        int err = -1;
        if(this->activeModule.get())
        {
            // processingCtx's job to add the midi event to its eventslist
            err = this->getProcessingCtx().MidiEvent(status, data1, data2);
        }
        return err;
    }

    void ProcessSamples(float *in, int inCh, float *out, int outCh, int nframes)
    {
        DbVST3ProcessingCtx &pctx = this->getProcessingCtx();
        if(pctx.error)
            return; // error reported earlier
        
        // midi-events and parameter value changes are applied to
        // processData when they arrive, but don't get processed 
        // 'til here.
        pctx.Process(in, inCh, out, outCh, nframes);
    }
}; // end struct DbVST3Ctx

#endif