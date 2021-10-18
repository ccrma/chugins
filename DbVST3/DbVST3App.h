#ifndef DbVST3App_h
#define DbVST3App_h

#include "VST3App.h"
#include "DbVST3Ctx.h"

/* DbVST3App is used as a factory to construct a DbVST3Ctx from a plugin.
 * A DbVST3Ctx holds one or more DbVST3Modules, each of which  represents 
 * an independent processing unit to users. Many  plugin files contain a 
 * single module, some (like mda-vst3) contain 10s of modules.
 */

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
            if(classInfo.category() == kVstAudioEffectClass)
            {
                DbVST3ModulePtr imod(new DbVST3Module(classInfo, factory));
                ctx.modules.emplace_back(imod);
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
        ctx.Finalize();
        return 0;
    }

};

#endif