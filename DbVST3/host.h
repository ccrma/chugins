#ifndef VST3Host_h
#define VST3Host_h

// VST3 hosting glue (extra gooey)
//
// Overviews here:
// https://developer.steinberg.help/display/VST/VST+3+API+Documentation
// https://developer.steinberg.help/display/VST/Audio+Processor+Call+Sequence
// https://developer.steinberg.help/display/VST/Edit+Controller+Call+Sequence
//
//  NB: we are not the IComponentHandler, rather VSTPluginInstance.
//  We support multiple PluginInstances loaded at the same time.
//
//  We need at least 2 threads for correct operation:
//   1. the message thread is where plugins are created and instantiated
//   2. the processing thread is where .process calls are made.
//
//  Certain calls trigger actions that require services from another thread.
// 
//   * SynchronizeState triggers a restartComponent message that *may*
//     emanate from the processing thread. Even in dumpVST3 mode.
//     For "multi-threaded plugins", we need a way to communicate the
//     reload-request back to the message thread.
//
// In ChucK-oriented operation (createMsgThread==true), all messages emanate
// from the processing thread. We always create a workerthread and now
// its actively monitored.
//
// In dumpVST operation (createMsgThread), all control messages emanate from 
// the message (main) thread.  Now, after issuing the synchronizeState call
// we must await the restartComponent message that may (a) or may-not (b) 
// emanate from a different thread.  In case (b) readAllParameters is a 
// side-effect of synchronizeState.  In case (a) the restartComponent method is
// Delegated to the message thread.  Problem is, a given plugin may operate in
// mode (a) or (b). When the Delegate method is invoked from another thread
// we need to Pop and execute it into the main thread.

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
    static VST3Host *Singleton(bool inProcessingThread=false);
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

    void Delegate(std::function<void()>); // to worker thread
    bool IsMessageThread();
    bool IsProcessingThread();

protected:
    ConcurrentQ<std::function<void()>> m_msgQueue;

private: // --------------------------------------------------------------
    ModulePtr
    loadPlugin(std::string const &path, std::string &error);

    bool endsWith(std::string const &fullpath, std::string const &partpath);

private: // --------------------------------------------------------------
    std::thread::id m_mainThreadId, m_msgThreadId;
    std::thread m_msgThread;
    static void workerThread(VST3Host *h);
    void initHostEnv();

};

#endif
