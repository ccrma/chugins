#ifndef App_h
#define App_h

// VST3 hosting glue (extra gooey)
//
// Overviews here:
// https://developer.steinberg.help/display/VST/VST+3+API+Documentation
// https://developer.steinberg.help/display/VST/Audio+Processor+Call+Sequence
// https://developer.steinberg.help/display/VST/Edit+Controller+Call+Sequence

#include "vst3.h"

#include <string>
#include <iostream>
#include <vector>

class App : 
    public Steinberg::FObject,
    public Steinberg::Vst::IHostApplication
    /* more appropriate for (but not needed for) processingCtx.h
    public Steinberg::Vst::IComponentHandler, // react to restart request
    public Steinberg::Vst::IComponentHandler2, // group edit
    public Steinberg::Vst::IUnitHandler // notification when programlist changed
    */
{
private:
    char const *m_name;
	Steinberg::IPtr<Steinberg::Vst::PlugInterfaceSupport> m_plugInterfaceSupport;

public:
    App();
    ~App() override
    {}

	OBJ_METHODS(App, FObject);
	REFCOUNT_METHODS(FObject);

public:
    void 
    PrintAllInstalledPlugins(std::ostream &ostr);

    int
    GetKnownPlugins(std::vector<std::string> &knownPlugins);

    int
    OpenPlugin(std::string const &path, struct PluginCtx &ctx, int verbosity=0);

    char const *
    GetName()
    {
        return m_name;
    }

private: // --------------------------------------------------------------
    Plugin
    loadPlugin(std::string const &path, std::string &error);

    // IHostApplication
    tresult PLUGIN_API
    getName(Steinberg::Vst::String128 name) override;

	tresult PLUGIN_API
    createInstance(Steinberg::TUID cid, Steinberg::TUID iid, void** obj) override;

    tresult PLUGIN_API
    queryInterface(const char* iid, void** obj) override;

    bool endsWith(std::string const &fullpath, std::string const &partpath);
};

#endif
