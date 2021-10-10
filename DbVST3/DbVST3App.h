#ifndef DbVST3App_h
#define DbVST3App_h

#include "VST3App.h"
#include "DbVST3Ctx.h"

/* -------------------------------------------------------------------------- */
class DbVST3App : public VST3App
{
public:
    DbVST3App() :
        VST3App("DbVST3")
    {
    }

    ~DbVST3App()
    {}

    int
    OpenPlugin(std::string const &path, DbVST3Ctx &ctx)
    {
        std::string error;
        ctx.Reset();
        ctx.plugin = this->LoadPlugin(path, error); // implemented in parent class
        if(!ctx.plugin.get())
        {
            std::string reason = "could not load vstplugin in file:";
            reason += path;
            reason += "\nError: ";
            reason += error;
            std::cerr << this->GetName() << " " << reason;
            return -1;
        }
        ctx.filepath = ctx.plugin->getPath();
        auto factory = ctx.plugin->getFactory();
		auto finfo = factory.info();
        ctx.vendor = finfo.vendor();
		for(auto& classInfo : factory.classInfos())
		{
            DbVST3Module imod;
            imod.name = classInfo.name();
            imod.category = classInfo.category();
            imod.subCategories = classInfo.subCategoriesString();
            imod.version = classInfo.version();
            imod.sdkVersion = classInfo.sdkVersion();
            if(classInfo.category() == kVstAudioEffectClass)
            {
                imod.processingCtx.provider = Steinberg::owned(new Provider(
                                      factory, classInfo, 
                                      true/*useGlobalInstance*/));
                if(imod.processingCtx.provider.get())
                {
                    this->getProviderParams(imod.processingCtx.provider, 
                                            imod.parameters);
                }
                ctx.modules.push_back(imod);
            }
            else
            {
                // For now we disgregard anything != kVstAudioEffectClass
                // new Provider throws an error
                #if 0
                    std::cout << "  (skipping params for module of type " 
                        << imod.category << ")\n";
                #endif
            }
		}
        ctx.Finalize();
        return 0;
    }


    /* ------------------------------------------------------------------ */
    void
    getProviderParams(ProviderPtr provider, 
        std::vector<DbVST3ParamInfo> &parameters)
    {
        Steinberg::Vst::IComponent* vstPlug = provider->getComponent();
        if(!vstPlug)
            return;
        Steinberg::Vst::IEditController* controller = provider->getController();
        if(!controller)
            return;
        // this->activateMainIOBusses(vstPlug, true);
        for(int i=0;i<controller->getParameterCount();i++)
        {
            ParameterInfo pinfo = {};
            tresult ret = controller->getParameterInfo(i, pinfo);
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

            DbVST3ParamInfo paramInfo;
            paramInfo.name = VST3::StringConvert::convert(pinfo.title);
            paramInfo.id = pinfo.id;
            paramInfo.defaultValue = pinfo.defaultNormalizedValue;
            paramInfo.stepCount = pinfo.stepCount;
            paramInfo.units = VST3::StringConvert::convert(pinfo.units);
            paramInfo.flags = pinfo.flags;
            parameters.push_back(paramInfo);
        }
        // this->activateMainIOBusses(vstPlug, false);
        provider->releasePlugIn(vstPlug, controller);
    }
};

#endif