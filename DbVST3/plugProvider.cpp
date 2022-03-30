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
dbPlugProvider::dbPlugProvider(
    Steinberg::Vst::IHostApplication *hostCtx,
    const PluginFactory& factory, ClassInfo classInfo, bool plugIsGlobal) : 
    factory(factory), 
    classInfo(classInfo),
    component(nullptr),
    controller(nullptr),
    plugIsGlobal(plugIsGlobal)
{
    this->hostContext = hostCtx;
	if(plugIsGlobal)
	{
		setupPlugin(this->hostContext);
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
	if(!this->component)
		this->setupPlugin(this->hostContext);

	if(this->component)
		this->component->addRef();

	return this->component;
}

//------------------------------------------------------------------------
IEditController* PLUGIN_API 
dbPlugProvider::getController()
{
	if(this->controller)
		this->controller->addRef();

	// 'iController == 0' is allowed! In this case the plug has no controller
	return this->controller;
}

//------------------------------------------------------------------------
Steinberg::IPluginFactory* PLUGIN_API 
dbPlugProvider::getPluginFactory()
{
	if(auto f = this->factory.get())
		return f.get();
	return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API 
dbPlugProvider::getComponentUID(FUID& uid) const
{
	uid = FUID::fromTUID(this->classInfo.ID().data());
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
	this->component = factory.createInstance<IComponent>(this->classInfo.ID());
	if(this->component)
	{
		// initialize the component with our context
		res = (this->component->initialize(hostContext) == Steinberg::kResultOk);
        if(!res)
        {
            std::cerr << "Problem initializing plugin component "
                      << this->classInfo.name() << "\n";
            return false;
        }

        // check if component is controller
        this->componentIsController = true;
        this->controller = Steinberg::FUnknownPtr<IEditController>(component).getInterface();
        if(!this->controller)
        {
            this->componentIsController = false;
            TUID controllerCID;
			if(this->component->getControllerClassId(controllerCID) == Steinberg::kResultTrue)
			{
				// create its controller part created from the factory
				this->controller = this->factory.createInstance<IEditController>(VST3::UID(controllerCID));
				if(!this->controller)
                {
                    std::cerr << "Problem initializing plugin controller(0) "
                            << this->classInfo.name() << "\n";
                    return false;
                }
			}
            else
            {
                std::cerr << "Problem initializing plugin controller(1) "
                            << this->classInfo.name() << "\n";
                return false;
            }
        }
        if(!this->componentIsController)
            this->controller->initialize(hostContext);
		this->connectComponents();
	}
	else 
    if(errorStream)
	{
		*errorStream << "Failed to create instance of " 
                     << this->classInfo.name() << "!\n";
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

	this->componentCP = NEW Steinberg::Vst::ConnectionProxy(compICP);
	this->controllerCP = NEW Steinberg::Vst::ConnectionProxy(contrICP);

	if(this->componentCP->connect(contrICP) != Steinberg::kResultTrue)
	{
		// TODO: Alert or what for non conformant plugin ?
        std::cerr << "Plugin is nonconformant " << classInfo.name() << "\n";
	}
	else
	{
		if(this->controllerCP->connect(compICP) != Steinberg::kResultTrue)
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
	if(!this->componentCP || !this->controllerCP)
		return false;

	bool res = this->componentCP->disconnect();
	res &= this->controllerCP->disconnect();

	this->componentCP = nullptr;
	this->controllerCP = nullptr;

	return res;
}

//------------------------------------------------------------------------
void dbPlugProvider::terminatePlugin()
{
	disconnectComponents();

	if(this->component)
		this->component->terminate();

	if(this->controller && this->componentIsController == false)
		this->controller->terminate();

	component = nullptr;
	controller = nullptr;
}
