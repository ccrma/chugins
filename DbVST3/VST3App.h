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
#include <public.sdk/source/vst/vsteditcontroller.h>
#include <public.sdk/samples/vst-hosting/audiohost/source/media/miditovst.h>

#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include <pluginterfaces/vst/ivstmidicontrollers.h>
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

    void 
    PrintAllInstalledPlugins(std::ostream &ostr)
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
    GetKnownPlugins(std::vector<std::string> &knownPlugins)
    {
        knownPlugins = VST3::Hosting::Module::getModulePaths();
        return (knownPlugins.size() > 0) ? 0 : -1;
    }

    Plugin
    LoadPlugin(std::string const &path, std::string &error)
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
    queryInterface(const char* iid, void** obj) override
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

private:
    bool endsWith(std::string const &fullpath, std::string const &partpath)
    {
        if(partpath.size() > fullpath.size()) return false;
        return (0 == fullpath.compare(fullpath.length() - partpath.length(), 
                                        partpath.length(), partpath));
    }

private:
    char const *m_name;
	Steinberg::IPtr<Steinberg::Vst::PlugInterfaceSupport> m_plugInterfaceSupport;
};

class VST3ComponentHandler : public Steinberg::Vst::IComponentHandler
{
public:
    using tresult = Steinberg::tresult;
    using ParamID = Steinberg::Vst::ParamID;
    using ParamValue = Steinberg::Vst::ParamValue;
    using int32 = Steinberg::int32;
    using uint32 = Steinberg::uint32;
    using TUID = Steinberg::TUID;

	tresult PLUGIN_API beginEdit(ParamID id) override
	{
		std::cout << "beginEdit called " << id << "\n";
		return Steinberg::kNotImplemented;
	}
	tresult PLUGIN_API performEdit(ParamID id, ParamValue valueNormalized) override
	{
		std::cout << "performEdit called " << id << " " << valueNormalized << "\n";
		return Steinberg::kNotImplemented;
	}
	tresult PLUGIN_API endEdit(ParamID id) override
	{
		std::cout << "endEdit called " << id << "\n";
		return Steinberg::kNotImplemented;
	}
	tresult PLUGIN_API restartComponent(int32 flags) override
	{
		std::cout << "restartComponent called " << flags << "\n";
		return Steinberg::kNotImplemented;
	}

private:
	tresult PLUGIN_API queryInterface(const TUID /*_iid*/, void** /*obj*/) override
	{
		return Steinberg::kNoInterface;
	}
	uint32 PLUGIN_API addRef() override { return 1000; }
	uint32 PLUGIN_API release() override { return 1000; }
};

#endif