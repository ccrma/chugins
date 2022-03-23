#include "app.h"
#include "pluginCtx.h"

App::App()
{
    m_name = "DbVST3";
    m_plugInterfaceSupport = owned(new Steinberg::Vst::PlugInterfaceSupport);
    Steinberg::Vst::PluginContextFactory::instance()
        .setPluginContext(this->unknownCast());
}

void 
App::PrintAllInstalledPlugins(std::ostream &ostr)
{
    std::vector<std::string> knownPlugins;
    this->GetKnownPlugins(knownPlugins);
    if(knownPlugins.size() == 0)
        ostr << "<no VST3 plugins found>\n";
    else
    for(const auto& path : knownPlugins)
        ostr << path << "\n";
}

int
App::GetKnownPlugins(std::vector<std::string> &knownPlugins)
{
    knownPlugins = VST3::Hosting::Module::getModulePaths();
    return (knownPlugins.size() > 0) ? 0 : -1;
}

int
App::OpenPlugin(std::string const &path, PluginCtx &ctx, int verbosity)
{
    std::string error;
    ctx.Reset();
    ctx.plugin = this->loadPlugin(path, error); // implemented in parent class
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
    if(ctx.filepath.size() == 0)
    {
        // std::cerr << "plugin doesn't know its path, using ours.\n";
        // happens on linux with plugins installed at ~/.vst3
        ctx.filepath = path;
    }
    auto factory = ctx.plugin->getFactory();
    auto finfo = factory.info();
    ctx.vendor = finfo.vendor();
    for(auto& classInfo : factory.classInfos())
    {
        if(classInfo.category() == kVstAudioEffectClass)
        {
            ModulePtr imod(new Module(classInfo, factory, verbosity));
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

Plugin
App::loadPlugin(std::string const &path, std::string &error)
{
    // first assume path is fully qualified
    Plugin plugin = VST3::Hosting::Module::create(path, error); 
    if(!plugin)
    {
        // see if it matches any installed plugins
        auto paths = VST3::Hosting::Module::getModulePaths();
        for(const auto& plug : paths)
        {
            if(this->endsWith(plug, path))
            {
                plugin = VST3::Hosting::Module::create(plug, error);
                break;
            }
        }
    }
    return plugin;
}

tresult PLUGIN_API
App::getName(Steinberg::Vst::String128 name)
{
    return VST3::StringConvert::convert(m_name, name) ? 
        Steinberg::kResultTrue : Steinberg::kInternalError;
}

tresult PLUGIN_API
App::createInstance(Steinberg::TUID cid, Steinberg::TUID iid, void** obj)
{
    // std::cout << "createInstance for cid:" << cid << " iid:" << iid << "\n";
    Steinberg::FUID classID = Steinberg::FUID::fromTUID(cid);
    Steinberg::FUID interfaceID = Steinberg::FUID::fromTUID(iid);
    if(classID == Steinberg::Vst::IMessage::iid && 
        interfaceID == Steinberg::Vst::IMessage::iid)
    {
        *obj = new Steinberg::Vst::HostMessage;
        return Steinberg::kResultTrue;
    }
    else 
    if(classID == Steinberg::Vst::IAttributeList::iid && 
        interfaceID == Steinberg::Vst::IAttributeList::iid)
    {
        if(auto al = Steinberg::Vst::HostAttributeList::make())
        {
            *obj = al.take();
            return Steinberg::kResultTrue;
        }
        return Steinberg::kOutOfMemory;
    }
    std::cerr << "VST3App::createInstance FAILED!!\n";
    *obj = nullptr;
    return Steinberg::kResultFalse;
}

tresult PLUGIN_API
App::queryInterface(const char* iid, void** obj)
{
    // std::cerr << "query call for " << iid << "\n";
    // default implemantion as of 3.7.3 (source/vst/hosting/pluginterfacesupport.cpp)
    /*
    ////---VST 3.0.0--------------------------------
    addPlugInterfaceSupported (IComponent::iid);
    addPlugInterfaceSupported (IAudioProcessor::iid);
    addPlugInterfaceSupported (IEditController::iid);
    addPlugInterfaceSupported (IConnectionPoint::iid);

    addPlugInterfaceSupported (IUnitInfo::iid);
    addPlugInterfaceSupported (IUnitData::iid);
    addPlugInterfaceSupported (IProgramListData::iid);

    //---VST 3.0.1--------------------------------
    addPlugInterfaceSupported (IMidiMapping::iid);

    //---VST 3.1----------------------------------
    addPlugInterfaceSupported (IEditController2::iid);
    */

    QUERY_INTERFACE(iid, obj, Steinberg::FUnknown::iid, 
                    Steinberg::Vst::IHostApplication);

    QUERY_INTERFACE(iid, obj, Steinberg::Vst::IHostApplication::iid, 
                    Steinberg::Vst::IHostApplication);

    if (m_plugInterfaceSupport && 
        m_plugInterfaceSupport->queryInterface(iid, obj) == Steinberg::kResultTrue)
    {
        return ::Steinberg::kResultOk;
    }
    else
    {
        std::cerr << "AppBase: queryInterface went unanswered for " << iid << "\n";
        *obj = nullptr;
        return Steinberg::kResultFalse;
    }
}

bool 
App::endsWith(std::string const &fullpath, std::string const &partpath)
{
    if(partpath.size() > fullpath.size()) return false;
    int dlen = fullpath.length() - partpath.length(); 
    return (0 == fullpath.compare(dlen, partpath.length(), partpath));
}