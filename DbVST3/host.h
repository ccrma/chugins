#ifndef Host_h
#define Host_h

// VST3 hosting glue (extra gooey)
//
// Overviews here:
// https://developer.steinberg.help/display/VST/VST+3+API+Documentation
// https://developer.steinberg.help/display/VST/Audio+Processor+Call+Sequence
// https://developer.steinberg.help/display/VST/Edit+Controller+Call+Sequence

#include "vst3.h"
#include "concurrentQ.h"
#include <thread>
#include <future>  // packaged_task

class Host : 
    public Steinberg::FObject,
    public Steinberg::Vst::IHostApplication
    /* more appropriate for (but not needed for) pluginInstance.h
    public Steinberg::Vst::IComponentHandler, // react to restart request
    public Steinberg::Vst::IComponentHandler2, // group edit
    public Steinberg::Vst::IUnitHandler // notification when programlist changed

    // JUCE plugin host:
    struct VST3HostContext  : public Vst::IComponentHandler,  // From VST V3.0.0
                          public Vst::IComponentHandler2, // From VST V3.1.0 (a very well named class, of course!)
                          public Vst::IComponentHandler3, // From VST V3.5.0 (also very well named!)
                          public Vst::IContextMenuTarget,
                          public Vst::IHostApplication,
                          public Vst::IUnitHandler,
                          private ComponentRestarter::Listener
    */
{
private:
    char const *m_name;
	Steinberg::IPtr<Steinberg::Vst::PlugInterfaceSupport> m_plugInterfaceSupport;
    std::string m_loadingPath;
    int m_debug;

public:
    Host(bool createMsgThread=false);
    ~Host() override;

	OBJ_METHODS(Host, FObject);
	REFCOUNT_METHODS(FObject);

public:
    void 
    PrintAllInstalledPlugins(std::ostream &ostr);

    int
    GetKnownPlugins(std::vector<std::string> &knownPlugins);

    void
    OpenPlugin(std::string const &path, 
        std::function<void(class VST3Ctx *)> onCompletion,
        int verbosity=0);

    char const *
    GetName() { return m_name; }

    void Delegate(std::function<void(void)>); // to worker thread
    bool IsWorkerThread();

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

private: // --------------------------------------------------------------
    ConcurrentQ<std::function<void(void)>> m_queue;
    std::thread::id m_mainThreadId, m_workerThreadId;
    std::thread m_workerThread;
    void initHostEnv();

    static void workerThread(Host *h);

};

#endif
