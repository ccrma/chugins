#include "host.h"
#include "vst3Ctx.h"
#include "plugProvider.h"
#include "processingUtil.h"

#include <iostream>

// The VST3 spec requires that IComponent::setupProcessing() is called on the message
// thread. If you call it from a different thread, some plugins may break.

// managed host is auto-contructed and torn down on dll-unload
std::unique_ptr<VST3Host> VST3Host::s_managedHost; 
// unmanagedHost is for subclasses (dumpVST3)
VST3Host * VST3Host::s_unmanagedHost;

VST3Host *
VST3Host::Singleton(bool createMsgThread)
{
    if(s_unmanagedHost) return s_unmanagedHost;
    if(!s_managedHost.get())
        s_managedHost.reset(new VST3Host(createMsgThread));
    return s_managedHost.get();
}

/* ------------------------------------------------------------------- */
void
VST3Host::workerThread(VST3Host *h) // consumer
{
    while(h->m_queue.IsActive())
    {
        auto item = h->m_queue.Pop(); // should block when empty
        if(h->m_debug)
        {
            std::cerr << "workerThread delegateBegin " 
                     << h->m_queue.Size() << "-------------------\n";
        }
        item();  
        if(h->m_debug)
            std::cerr << "workerThread delegateEnd active:" 
                     << h->m_queue.IsActive() << "---------------------\n";
    }
    if(h->m_debug)
        std::cerr << "workerThread exiting\n";
}

#define ASSERT_WORKER_THREAD assert(this->IsWorkerThread())
    // thread not joinable means it hasn't been constructed

#if 0 || defined(WIN32)
// background: loading the juce example plugin (AudioPluginDemo) it triggered 
// massive memory leak report.  Apparently their unloader on windows+DEBUG
// enables the Crt leak detector described here: https://docs.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2015/debugger/finding-memory-leaks-using-the-crt-library?view=vs-2015&redirectedfrom=MSDN
// After some investigation, the conclusion is that it's chuck's fault.
// When enabled and run on dumpVST3 we appear clean.
// 
static _CrtMemState s_mem1, s_mem2;
#define memchk_break() { auto& _ab = _crtBreakAlloc; _ab = 549; __debugbreak();}
#define memcheck1 _CrtMemCheckpoint(&s_mem1);  
#define memcheck2 \
{ \
    _CrtMemCheckpoint( &s_mem2 );  \
    _CrtMemState sdiff; \
    if(_CrtMemDifference( &sdiff, &s_mem1, &s_mem2)) \
        _CrtMemDumpStatistics( &sdiff ); \
}
#else
#define memchk_break
#define memcheck1
#define memcheck2
#endif

/* ------------------------------------------------------------------- */
VST3Host::VST3Host(bool createMsgThread) // private method
{
    memcheck1;
    m_name = "DbVST3";
    m_mainThreadId = std::this_thread::get_id();
    m_debug = 0;
    if(m_debug)
        std::cerr << "VST3Host constructed\n";
    if(createMsgThread)
    {
        m_workerThread = std::thread(workerThread, this);
        m_workerThreadId = m_workerThread.get_id();
        std::function<void(void)> fn = std::bind(&VST3Host::initHostEnv, this);
        this->Delegate(fn); // initialize system in worker (not audio) thread.
    }
    else
    {
        m_workerThreadId = m_mainThreadId;
        this->initHostEnv();
    }
    if(m_debug)
    {
        std::cerr << "VST3Host constructed in thread " <<  m_mainThreadId << "\n";
        std::cerr << "VST3Host workerthread " << m_workerThread.get_id() << "\n";
    }
}

VST3Host::~VST3Host()
{
    // Since we are a singleton, shared across all instances,
    // this destructor runs as part of the shutdown process of
    // the entire dll.  This is under the control of Chuck and
    // is integrally tied into platform-specific teardown.
    // On windows our workerthread appears to be destroyed
    // prior to this point (and we never see "workerThread exiting")
    if(m_workerThread.joinable())
    {
        m_queue.Bail();
        m_workerThread.join();
    }
    if(this->m_debug)
        std::cerr << "VST3Host deleted\n";

    memcheck2;
}

void
VST3Host::initHostEnv()
{
    // placeholder for stuff that needs to execute in the main thread.
}

void
VST3Host::Delegate(std::function<void(void)> delFn)
{
    m_queue.Push(delFn);
}

void 
VST3Host::PrintAllInstalledPlugins(std::ostream &ostr)
{
    ASSERT_WORKER_THREAD;
    std::vector<std::string> knownPlugins;
    this->GetKnownPlugins(knownPlugins);
    if(knownPlugins.size() == 0)
        ostr << "<no VST3 plugins found>\n";
    else
    for(const auto& path : knownPlugins)
        ostr << path << "\n";
}

int
VST3Host::GetKnownPlugins(std::vector<std::string> &knownPlugins)
{
    ASSERT_WORKER_THREAD;
    knownPlugins = VST3::Hosting::Module::getModulePaths();
    return (knownPlugins.size() > 0) ? 0 : -1;
}

bool
VST3Host::IsWorkerThread()
{
    return m_workerThreadId == std::this_thread::get_id();
}

bool
VST3Host::IsProcessingThread()
{
    return m_mainThreadId == std::this_thread::get_id();
}

// OptionPlugin merely opens the plugin file, but doesn't instantiate any
// other objects (ie ModulePtr contains DLL-open context). In each plugin
// is a "factory" function that is used later to construct plugin components.
void
VST3Host::OpenPlugin(std::string const &path, std::function<void(VST3Ctx*)> callback,
                int verbosity)
{
    if(this->IsWorkerThread())
    {
        if(this->m_debug)
            std::cerr << "OpenPlugin " << path << "\n";
        std::string error;
        ModulePtr plugin = this->loadPlugin(path, error); // implemented in parent class
        if(!plugin.get())
        {
            std::string reason = "could not load vstplugin in file:";
            reason += path;
            reason += "\nError: ";
            reason += error;
            std::cerr << this->GetName() << " " << reason;
            callback(nullptr);
        }
        else
        {
            callback(new VST3Ctx(this, plugin, path));
        }
    }
    else
    {
        std::function<void(void)> fn = std::bind(&VST3Host::OpenPlugin, this, path,     
                                                callback, verbosity);
        this->Delegate(fn);
    }
}

ModulePtr
VST3Host::loadPlugin(std::string const &path, std::string &error)
{
    ASSERT_WORKER_THREAD;

    // first assume path is fully q
    this->m_loadingPath = path;
    ModulePtr modPtr = VST3::Hosting::Module::create(path, error); 
    if(!modPtr)
    {
        // see if it matches any installed plugins
        auto paths = VST3::Hosting::Module::getModulePaths();
        for(const auto& plug : paths)
        {
            if(this->endsWith(plug, path))
            {
                modPtr = VST3::Hosting::Module::create(plug, error);
                break;
            }
        }
    }
    return modPtr;
}

bool 
VST3Host::endsWith(std::string const &fullpath, std::string const &partpath)
{
    if(partpath.size() > fullpath.size()) return false;
    int dlen = fullpath.length() - partpath.length(); 
    return (0 == fullpath.compare(dlen, partpath.length(), partpath));
}
