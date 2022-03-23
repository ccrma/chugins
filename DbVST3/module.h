#ifndef Module_h
#define Module_h

#include "param.h"
#include "processingCtx.h"

#include <string>
#include <vector>
#include <unordered_map>

// A plugin file (which is accessed as a VST::Module) can contain 1 or more
// interfaces (AudioEffects, Controllers, ...).  This class represents one 
// of these. IE: it maps to the user's definition of module.
//
class Module 
{
private:
    int programChangeIndex; // some modules support 0, some 1, some > 1
    int verbosity;

public:
    std::string name;
    std::string category; // "Audio Module Class" | "Component Controller Class"
    std::string subCategories;
    std::string version;
    std::string sdkVersion;
    std::vector<ParamInfo> parameters;
    ProcessingCtx processingCtx;
    std::unordered_map<std::string, int> nameToIndex;
    std::unordered_map<Steinberg::Vst::ParamID, int> idToIndex;

public:
    // constructed by pluginCtx in building up its list of kVstAudioEffectClass
    // (modules).  Plugin's factory is passed in. Comp
    // of our processingCtx until is is needed via its nomination as the 
    // activemodule.
    Module(VST3::Hosting::ClassInfo &classInfo, 
        const Steinberg::Vst::PlugProvider::PluginFactory &factory,
        int verboseness=0)
    {
        this->programChangeIndex = -1;
        this->verbosity = 0;
        if(verboseness != 0)
            this->SetVerbosity(verboseness);
        this->name = classInfo.name();
        this->category = classInfo.category();
        this->subCategories = classInfo.subCategoriesString();
        this->version = classInfo.version();
        this->sdkVersion = classInfo.sdkVersion();
        this->processingCtx.Init(factory, classInfo, this->parameters);
        this->programChangeIndex = -1;
        this->verbosity = 0 ;
    }

    void
    SetVerbosity(int v)
    {
        this->verbosity = v;
        this->processingCtx.SetVerbosity(v);
    }

    ParamInfo *
    GetParamInfo(int index)
    {
        if(index < this->parameters.size())
            return &this->parameters[index];
        else
            return nullptr;
    }

    /* this entrypoint supports midi name-remapping
     */
    ParamInfo *
    GetParamInfo(std::string const &nm, float *val=nullptr)
    {
        ParamInfo *info = nullptr;
        if(this->nameToIndex.size() == 0 && this->parameters.size() > 0)
        {
            // build a little cache
            for(int i=0;i<this->parameters.size();i++)
            {
                this->nameToIndex[this->parameters[i].name] = i;
                this->idToIndex[this->parameters[i].id] = i;
            }
        }
        if(this->nameToIndex.count(nm))
        {
            int index = this->nameToIndex[nm];
            info = &this->parameters[index];
        }
        else
        {
            Steinberg::Vst::ParamID id = Steinberg::Vst::kNoParamId;
            // it may be a standard Fiddle Midi Control
            if(nm == "PitchWheel" || nm == "PitchBend")
            {
                // -1,1 => 0,1
                if(val)
                {
                    float newval = .5f * (*val + 1.f);
                    // std::cerr << *val <<" -> "<<newval<<"\n";
                    *val = newval;
                }
                id = this->processingCtx.GetMidiMapping(Steinberg::Vst::kPitchBend);
            }
            else
            if(nm == "AfterTouch")
            {
                id = this->processingCtx.GetMidiMapping(Steinberg::Vst::kAfterTouch);
            }
            // "AfterTouch.poly" is an event in VST3 (tbd)
            else
            if(nm.rfind("CC", 0) == 0) // eg. CC74
            {
                int midiCC = std::stoi(nm.substr(2));
                id = this->processingCtx.GetMidiMapping(midiCC);
            }
            if(id != Steinberg::Vst::kNoParamId)
            {
                int index = this->idToIndex[id];
                if(index < this->parameters.size())
                    info = &this->parameters[index];
            }
        }
        return info;
    }

    void Print(std::ostream &ostr, char const *indent, int index, bool detailed)
    {
        if(!detailed)
            ostr << indent << "- " << this->name << "\n";
        else
            ostr << indent << "- RegistryName: '" << this->name << "'\n";
        if(!detailed) return;
        // category is always "Audio Module Class"
        ostr << indent << "  Categories:\n";

        // parse the subcategory on |
        std::string sc = this->subCategories;
        ostr << indent << "    - vst3\n";
        while(1)
        {
            size_t pos = sc.find("|");
            ostr << indent << "    - " << sc.substr(0, pos) << "\n";
            if(pos != std::string::npos)
                sc.erase(0, pos + 1);
            else
                break;
        } 
        
        ostr << indent << "  Version: " << this->version << "\n";
        ostr << indent << "  SdkVersion: '" << this->sdkVersion << "'\n";
        ostr << indent << "  NumInputs: " << this->parameters.size() << "\n";
        ostr << indent << "  Inputs:\n";
        std::string in(indent);
        in.append("  ");
        char const *i2 = in.c_str();
        for(int i=0; i<this->parameters.size();i++)
        {
            if(this->parameters[i].hidden())
                continue;
            this->parameters[i].Print(ostr, i2, i);
        }
    }

    /* ------------------------------------------------------------------ */
private:

};

typedef std::shared_ptr<Module> ModulePtr;

#endif