#ifndef VST3Host_h
#define VST3Host_h

// VST3 hosting glue (extra gooey)
//
// Overviews here:
// https://developer.steinberg.help/display/VST/VST+3+API+Documentation
// https://developer.steinberg.help/display/VST/Audio+Processor+Call+Sequence
// https://developer.steinberg.help/display/VST/Edit+Controller+Call+Sequence
//
//  NB: we are not the IComponentHandler, rather VSTPluginInstance is because
//  we support multiple PluginInstances loaded at the same time.

#include "vst3.h"
#include "concurrentQ.h"
#include <thread>
#include <iostream>

class VST3Host 
{
private:
    char const *m_name;
    std::string m_loadingPath;
    int m_debug;
    static std::unique_ptr<VST3Host> s_managedHost; // as chugin

protected:
    static VST3Host *s_unmanagedHost; // dumpVST3

protected:
    VST3Host(bool createMsgThread=false);

public:
    static VST3Host *Singleton(bool createMsgThread=false);
    ~VST3Host();

public:
    void 
    PrintAllInstalledPlugins(std::ostream &ostr);

    int
    GetKnownPlugins(std::vector<std::string> &knownPlugins);

    void
    OpenPlugin(std::string const &path, 
        std::function<void(class VST3Ctx *)> onCompletion,
        int verbosity=0);

    char const * GetName() { return m_name; }

    void Delegate(std::function<void(void)>); // to worker thread
    bool IsWorkerThread();
    bool IsProcessingThread();

private: // --------------------------------------------------------------
    ModulePtr
    loadPlugin(std::string const &path, std::string &error);

    bool endsWith(std::string const &fullpath, std::string const &partpath);

private: // --------------------------------------------------------------
    ConcurrentQ<std::function<void(void)>> m_queue;
    std::thread::id m_mainThreadId, m_workerThreadId;
    std::thread m_workerThread;
    static void workerThread(VST3Host *h);
    void initHostEnv();

};

#endif
