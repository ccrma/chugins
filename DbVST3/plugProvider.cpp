//
// Override for VST3 PlugProvider due to malfunction wrt JUCE plugins.
//

#include "plugProvider.h"
#include "vst3.h"
#include <public.sdk/source/vst/hosting/connectionproxy.h>
#include <cstdio>
#include <iostream>

static std::ostream* errorStream = &std::cout;

//------------------------------------------------------------------------
// dbPlugProvider
//------------------------------------------------------------------------
dbPlugProvider::dbPlugProvider(const PluginFactory& factory, 
                               ClassInfo classInfo, bool plugIsGlobal) : 
    factory(factory), 
    classInfo(classInfo),
    component(nullptr),
    controller(nullptr),
    plugIsGlobal(plugIsGlobal)
{
	if(plugIsGlobal)
	{
		setupPlugin(dbPluginContextFactory::instance().getPluginContext());
	}
}

//------------------------------------------------------------------------
dbPlugProvider::~dbPlugProvider()
{
	terminatePlugin();
}

//------------------------------------------------------------------------
IComponent* PLUGIN_API 
dbPlugProvider::getComponent()
{
	if(!component)
		setupPlugin(dbPluginContextFactory::instance().getPluginContext());

	if(component)
		component->addRef();

	return component;
}

//------------------------------------------------------------------------
IEditController* PLUGIN_API 
dbPlugProvider::getController()
{
	if(controller)
		controller->addRef();

	// 'iController == 0' is allowed! In this case the plug has no controller
	return controller;
}

//------------------------------------------------------------------------
Steinberg::IPluginFactory* PLUGIN_API 
dbPlugProvider::getPluginFactory()
{
	if(auto f = factory.get())
		return f.get();
	return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API 
dbPlugProvider::getComponentUID(FUID& uid) const
{
	uid = FUID::fromTUID(classInfo.ID().data());
	return Steinberg::kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API 
dbPlugProvider::releasePlugIn(IComponent* iComponent, IEditController* iController)
{
	if(iComponent)
		iComponent->release();

	if(iController)
		iController->release();

	if(!plugIsGlobal)
	{
		terminatePlugin();
	}

	return Steinberg::kResultOk;
}

//------------------------------------------------------------------------
bool dbPlugProvider::setupPlugin(FUnknown* hostContext)
{
	bool res = false;

	//---create Plug-in here!--------------
	// create its component part
	component = factory.createInstance<IComponent>(classInfo.ID());
	if(component)
	{
		// initialize the component with our context
		res = (component->initialize(hostContext) == Steinberg::kResultOk);
        if(!res)
        {
            std::cerr << "Problem initializing plugin component "
                      << classInfo.name() << "\n";
            return false;
        }

        // check if component is controller
        bool controllerIsComponent = true;
        controller = Steinberg::FUnknownPtr<IEditController>(component).getInterface();
        if(!controller)
        {
            controllerIsComponent = false;
            TUID controllerCID;
			if(component->getControllerClassId(controllerCID) == Steinberg::kResultTrue)
			{
				// create its controller part created from the factory
				controller = factory.createInstance<IEditController>(VST3::UID(controllerCID));
				if(!controller)
                {
                    std::cerr << "Problem initializing plugin controller(0) "
                            << classInfo.name() << "\n";
                    return false;
                }
			}
            else
            {
                std::cerr << "Problem initializing plugin controller(1) "
                            << classInfo.name() << "\n";
                return false;
            }
        }
        controller->initialize(hostContext); // may be a double-init, but okay
		connectComponents();
	}
	else 
    if(errorStream)
	{
		*errorStream << "Failed to create instance of " << classInfo.name() << "!\n";
	}

	return res;
}

//------------------------------------------------------------------------
bool dbPlugProvider::connectComponents()
{
	if(!component || !controller)
		return false;

	Steinberg::FUnknownPtr<Steinberg::Vst::IConnectionPoint> compICP(component);
	Steinberg::FUnknownPtr<Steinberg::Vst::IConnectionPoint> contrICP(controller);
	if(!compICP || !contrICP)
		return false;

	bool res = false;

	componentCP = NEW Steinberg::Vst::ConnectionProxy(compICP);
	controllerCP = NEW Steinberg::Vst::ConnectionProxy(contrICP);

	if(componentCP->connect(contrICP) != Steinberg::kResultTrue)
	{
		// TODO: Alert or what for non conformant plugin ?
        std::cerr << "Plugin is nonconformant " << classInfo.name() << "\n";
	}
	else
	{
		if(controllerCP->connect(compICP) != Steinberg::kResultTrue)
		{
			// TODO: Alert or what for non conformant plugin ?
            std::cerr << "Plugin is nonconformant " << classInfo.name() << " (1)\n";
		}
		else
			res = true;
	}
	return res;
}

//------------------------------------------------------------------------
bool dbPlugProvider::disconnectComponents()
{
	if(!componentCP || !controllerCP)
		return false;

	bool res = componentCP->disconnect();
	res &= controllerCP->disconnect();

	componentCP = nullptr;
	controllerCP = nullptr;

	return res;
}

//------------------------------------------------------------------------
void dbPlugProvider::terminatePlugin()
{
	disconnectComponents();

	bool controllerIsComponent = false;
	if(component)
	{
		controllerIsComponent = Steinberg::FUnknownPtr<IEditController>(component).getInterface() != nullptr;
		component->terminate();
	}

	if(controller && controllerIsComponent == false)
		controller->terminate();

	component = nullptr;
	controller = nullptr;
}
