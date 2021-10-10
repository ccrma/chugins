#ifndef VST3App_h
#define VST3App_h

// VST3 hosting glue (extra gooey)
//
// Overviews here:
// https://developer.steinberg.help/display/VST/VST+3+API+Documentation
// https://developer.steinberg.help/display/VST/Audio+Processor+Call+Sequence
// https://developer.steinberg.help/display/VST/Edit+Controller+Call+Sequence

#include <string>
#include <iostream>
#include <vector>

#include <public.sdk/source/vst/hosting/hostclasses.h>
#include <public.sdk/source/vst/hosting/module.h>
#include <public.sdk/source/vst/hosting/pluginterfacesupport.h>
#include <public.sdk/source/vst/hosting/plugprovider.h>
#include <public.sdk/source/vst/hosting/processdata.h>
#include <public.sdk/source/vst/hosting/eventlist.h>
#include <public.sdk/source/vst/hosting/parameterchanges.h>
#include <public.sdk/source/vst/utility/stringconvert.h>

#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include <pluginterfaces/vst/ivstevents.h>
#include <pluginterfaces/vst/vsttypes.h>
#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/base/ftypes.h>
#include <pluginterfaces/base/fplatform.h>
#include <base/source/fobject.h>

/* -------------------------------------------------------------------------- */
// we pattern ourself after validator.cpp which bypasses HostApplication
class VST3App : public Steinberg::FObject, public Steinberg::Vst::IHostApplication 
{
public:
    using Provider = Steinberg::Vst::PlugProvider;
    using ProviderPtr = Steinberg::IPtr<Provider>;
    using tresult = Steinberg::tresult;

    VST3App(char const *nm) :
        m_name(nm)
    {
        m_plugInterfaceSupport = owned(new Steinberg::Vst::PlugInterfaceSupport);
        Steinberg::Vst::PluginContextFactory::instance()
            .setPluginContext(this->unknownCast());
    }

    ~VST3App() override
    {}

	OBJ_METHODS(VST3App, FObject);
	REFCOUNT_METHODS(FObject);

public:
    using Plugin = VST3::Hosting::Module::Ptr;

    Plugin
    LoadPlugin(std::string const &path, std::string &error)
    {
        return VST3::Hosting::Module::create(path, error);
    }

    char const *
    GetName()
    {
        return m_name;
    }

protected: // --------------------------------------------------------------
    using ProviderList = std::vector<ProviderPtr>;
    using ParameterInfo = Steinberg::Vst::ParameterInfo;

    // IHostApplication
    tresult PLUGIN_API
    getName(Steinberg::Vst::String128 name) override
    {
        return VST3::StringConvert::convert(m_name, name) ? 
            Steinberg::kResultTrue : Steinberg::kInternalError;
    }

	tresult PLUGIN_API
    createInstance(Steinberg::TUID cid, Steinberg::TUID iid, void** obj) override
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
            *obj = new Steinberg::Vst::HostAttributeList;
            return Steinberg::kResultTrue;
        }
        std::cerr << "VST3App::createInstance FAILED!!\n";
        *obj = nullptr;
        return Steinberg::kResultFalse;
    }

    tresult PLUGIN_API
    queryInterface(const char* iid, void** obj)
    {
        // std::cerr << "query call for " << _iid << "\n";
        QUERY_INTERFACE(iid, obj, Steinberg::Vst::IHostApplication::iid, 
                        Steinberg::Vst::IHostApplication);
        if (m_plugInterfaceSupport && 
            m_plugInterfaceSupport->queryInterface(iid, obj) == Steinberg::kResultTrue)
            return ::Steinberg::kResultOk;
        std::cerr << "query went unanswered!!!\n";
        return Steinberg::kResultFalse;
    }

    void 
    printAllInstalledPlugins()
    {
        std::cout << "Searching installed Plug-ins...\n";
        std::cout.flush();
        auto paths = VST3::Hosting::Module::getModulePaths();
        if(paths.empty())
        {
            std::cout << "No Plug-ins found.\n";
            return;
        }
        for(const auto& path : paths)
        {
            std::cout << path << "\n";
        }
    }

private:
    char const *m_name;
	Steinberg::IPtr<Steinberg::Vst::PlugInterfaceSupport> m_plugInterfaceSupport;
};

#endif