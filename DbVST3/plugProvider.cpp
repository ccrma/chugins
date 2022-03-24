//
// Override for VST3 PlugProvider due to malfunction wrt JUCE plugins.
//

#include "plugprovider.h"
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
    component(nullptr),
    controller(nullptr),
    classInfo(classInfo),
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

		// try to create the controller part from the component
		// (for Plug-ins which did not succeed to separate component from controller)
		if(component->queryInterface(IEditController::iid, (void**)&controller) != Steinberg::kResultTrue)
		{
			TUID controllerCID;

			// ask for the associated controller class ID
			if(component->getControllerClassId (controllerCID) == Steinberg::kResultTrue)
			{
				// create its controller part created from the factory
				controller = factory.createInstance<IEditController>(VST3::UID(controllerCID));
				if(controller)
				{
					// initialize the component with our context
					res = (controller->initialize (hostContext) == Steinberg::kResultOk);
				}
			}
		}
	}
	else if(errorStream)
	{
		*errorStream << "Failed to create instance of " << classInfo.name() << "!\n";
	}

	if(res)
		connectComponents();

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
	}
	else
	{
		if(controllerCP->connect(compICP) != Steinberg::kResultTrue)
		{
			// TODO: Alert or what for non conformant plugin ?
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
