#ifndef DbVST3Module_h
#define DbVST3Module_h

#include "DbVST3Param.h"
#include "DbVST3Processing.h"


#include <string>
#include <vector>
#include <unordered_map>

// A plugin file (aka Steinberg::Vst::Module), can contain 1 or more
// interfaces (aka modules)
class DbVST3Module 
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
    std::vector<DbVST3ParamInfo> parameters;
    DbVST3ProcessingCtx processingCtx;
    std::unordered_map<std::string, int> nameToIndex;
    std::unordered_map<Steinberg::Vst::ParamID, int> idToIndex;

    DbVST3Module(VST3::Hosting::ClassInfo &classInfo, 
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
        auto provider = this->processingCtx.initProvider(factory, classInfo);
        if(provider.get())
        {
            this->getProviderParams(provider, this->parameters);
            this->processingCtx.synchronizeStates();
        }
        this->programChangeIndex = -1;
        this->verbosity = 0 ;
    }

    void
    SetVerbosity(int v)
    {
        this->verbosity = v;
        this->processingCtx.SetVerbosity(v);
    }

    DbVST3ParamInfo *
    GetParamInfo(int index)
    {
        if(index < this->parameters.size())
            return &this->parameters[index];
        else
            return nullptr;
    }

    /* this entrypoint supports midi name-remapping
     */
    DbVST3ParamInfo *
    GetParamInfo(std::string const &nm, float *val=nullptr)
    {
        DbVST3ParamInfo *info = nullptr;
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
    void
    getProviderParams(VST3App::ProviderPtr provider, 
        std::vector<DbVST3ParamInfo> &parameters)
    {
        this->programChangeIndex = -1;
        Steinberg::Vst::IComponent* vstPlug = provider->getComponent();
        if(!vstPlug)
            return;
        Steinberg::Vst::IEditController* controller = provider->getController();
        if(!controller)
            return;

        Steinberg::FUnknownPtr<Steinberg::Vst::IUnitInfo> unitInfo(controller);

        std::unordered_map<int, std::shared_ptr<std::vector<std::string>>> programMap;
        if(unitInfo)
        {
            Steinberg::int32 programListCount = unitInfo->getProgramListCount();
            if(programListCount > 0)
            {
                for(int i=0;i<programListCount;i++)
                {
                    Steinberg::Vst::ProgramListInfo plInfo;
                    if(unitInfo->getProgramListInfo(i, plInfo) == Steinberg::kResultOk)
                    {
                        int plId = plInfo.id;
                        auto plName = VST3::StringConvert::convert(plInfo.name);

                        auto np = std::shared_ptr<std::vector<std::string>>(new std::vector<std::string>);
                        programMap[plId] = np;
                        // std::cerr << "programlist " << plName << " for " << plId << "-------------------------\n";
                        for(int j=0;j<plInfo.programCount; j++)
                        {
                            Steinberg::Vst::TChar pnm[256];
                            if(unitInfo->getProgramName(plId, j, pnm) == Steinberg::kResultOk)
                            {
                                if(pnm[0] == 0)
                                    continue;
                                auto pnmStr = VST3::StringConvert::convert(pnm);
                                // std::cerr << "   " << pnmStr << "\n";
                                np->push_back(pnmStr);
                            }
                        }
                    }
                }
            }
        }
        for(int i=0;i<controller->getParameterCount();i++)
        {
            Steinberg::Vst::ParameterInfo pinfo = {};
            Steinberg::tresult ret = controller->getParameterInfo(i, pinfo);
            if(ret != Steinberg::kResultOk)
            {
                std::cerr << "Parameter " << i << "has no info\n";
                continue;
            }
            if(pinfo.id < 0)
            {
                std::cerr << "Parameter " << i 
                    << "has invalid id: " << pinfo.id << "\n";
                continue;
            }
		    if(VST3::StringConvert::convert(pinfo.title).find("MIDI CC ")
                != std::string::npos)
                continue;

            DbVST3ParamInfo paramInfo(pinfo);
            if(pinfo.flags & Steinberg::Vst::ParameterInfo::kIsProgramChange)
            {
                if(programMap.count(pinfo.id))
                {
                    paramInfo.menuItems = programMap[pinfo.id];
                    // std::cerr << paramInfo.name << " has " <<
                    //   paramInfo.menuItems->size() << " menuitems\n";
                }
                else
                if(this->programChangeIndex == -1)
                {
                    this->programChangeIndex = i;
                    if(this->verbosity)
                        std::cerr << "Program-change index: " << i << "\n";
                }
                else
                {
                    // this appears to be allowed if a module supports 
                    // programlists (above).
                    std::cerr << "Multiple program-changes? " << i << "\n";
                }
            }
            parameters.push_back(paramInfo); // issues copy-constructor
        }
        // this->activateMainIOBusses(vstPlug, false);
        provider->releasePlugIn(vstPlug, controller);
    }

};

typedef std::shared_ptr<DbVST3Module> DbVST3ModulePtr;

#endif