//
// Override for VST3 PlugProvider due to malfunction wrt JUCE plugins.
//

#include "plugProvider.h"
#include "vst3.h"
#include <public.sdk/source/vst/hosting/connectionproxy.h>
#include <cstdio>
#include <iostream>

static std::ostream* s_errorStream = &std::cerr;

//------------------------------------------------------------------------
// dbPlugProvider
//------------------------------------------------------------------------
dbPlugProvider::dbPlugProvider(
    Steinberg::Vst::IHostApplication *hostCtx,
    const PluginFactory& factory, ClassInfo classInfo, bool plugIsGlobal) : 
    factory(factory), 
    classInfo(classInfo),
    component(nullptr),
    editController(nullptr),
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
	if(this->editController)
		this->editController->addRef();

	// 'iController == 0' is allowed! In this case the plug has no controller
	return this->editController;
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
        this->editController = Steinberg::FUnknownPtr<IEditController>(component).getInterface();
        if(!this->editController)
        {
            this->componentIsController = false;
            TUID controllerCID;
			if(this->component->getControllerClassId(controllerCID) == Steinberg::kResultTrue)
			{
				// create its controller part created from the factory
                // std::cerr << "editController cid " << controllerCID << "\n";
				this->editController = this->factory.createInstance<IEditController>(VST3::UID(controllerCID));
				if(!this->editController)
                {
                    std::cerr << "Problem initializing plugin controller(0) "
                            << this->classInfo.name() << "\n";
                    return false;
                }
                else
                    this->editController->initialize(hostContext);
			}
            else
            {
                std::cerr << "Problem initializing plugin controller(1) "
                            << this->classInfo.name() << "\n";
                return false;
            }
        }
        else
        {
            // ardour claims that we may need to double-initialize in
            // the case where the component can act as editcontroller
            // tried this to to avail with LABS.
            // this->editController->initialize(hostContext);
        }
        this->setupExtra();
		this->connectComponents();
	}
	else 
    if(s_errorStream)
	{
		*s_errorStream << "PlugProvider failed to create instance of " 
                       << this->classInfo.name() << "!\n";
	}

	return res;
}

//------------------------------------------------------------------------
bool dbPlugProvider::setupExtra()
{
	if(!this->component || !this->editController)
		return false;
    
    // FUnknownPtr magic follows
	this->editController2 = this->component; 
	this->midiMapping = this->component;
	this->audioProcessor = this->component;
	this->componentHandler = this->component;
	this->componentHandler2 = this->component;
	this->unitInfo = this->component;
	this->unitData = this->component;
    this->programListData = this->component;
    if(!this->componentIsController)
    {
        if(!this->editController2) this->editController2 = this->editController; 
        if(!this->midiMapping) this->midiMapping = this->editController;
        if(!this->audioProcessor) this->audioProcessor = this->editController;
        if(!this->componentHandler) this->componentHandler = this->editController;
        if(!this->componentHandler2) this->componentHandler2 = this->editController;
        if(!this->unitInfo) this->unitInfo = this->editController;
        if(!this->unitData) this->unitData = this->editController;
        if(!this->programListData) this->programListData = this->editController;
    }
    return true;
}

//------------------------------------------------------------------------
bool dbPlugProvider::connectComponents()
{
	if(!this->component || !this->editController)
		return false;

	Steinberg::FUnknownPtr<Steinberg::Vst::IConnectionPoint> compICP(this->component);
	Steinberg::FUnknownPtr<Steinberg::Vst::IConnectionPoint> contrICP(this->editController);
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
    this->editController->setComponentHandler(nullptr); // defensive

	disconnectComponents();

    // release hold on all these..
    this->editController2 = nullptr;
    this->midiMapping = nullptr;
    this->audioProcessor = nullptr;
    this->componentHandler = nullptr;
    this->componentHandler2 = nullptr;
    this->unitInfo = nullptr;
    this->unitData = nullptr;
    this->programListData = nullptr;

	if(this->component)
		this->component->terminate();

	if(this->editController && !this->componentIsController)
		this->editController->terminate();

	this->component = nullptr;
	this->editController = nullptr;
}
