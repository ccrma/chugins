// A modified version of the vst3 file:
//  public.sdk/source/vst/hosting/plugprovider.h
#ifndef plugprovider_h
#define plugprovider_h

#include "vst3.h"
#include <public.sdk/source/vst/hosting/connectionproxy.h>

//------------------------------------------------------------------------
/** Helper for creating and initializing component.
 */

//------------------------------------------------------------------------
class dbPlugProvider : public FObject, Steinberg::Vst::ITestPlugProvider2
{
public:
	using ClassInfo = VST3::Hosting::ClassInfo;
	using PluginFactory = VST3::Hosting::PluginFactory;

	//--- ---------------------------------------------------------------------
	dbPlugProvider(const PluginFactory& factory, ClassInfo info, 
                   bool plugIsGlobal = true);
	~dbPlugProvider() override;

	//--- from ITestPlugProvider ------------------
	IComponent* PLUGIN_API getComponent() override;
	IEditController* PLUGIN_API getController() override;
	tresult PLUGIN_API releasePlugIn(IComponent* component, 
                                    IEditController* controller) override;
	tresult PLUGIN_API getSubCategories(IStringResult& result) const override
	{
		result.setText(classInfo.subCategoriesString().data());
		return Steinberg::kResultTrue;
	}
	tresult PLUGIN_API getComponentUID(FUID& uid) const override;

	//--- from ITestPlugProvider2 ------------------
	Steinberg::IPluginFactory* PLUGIN_API getPluginFactory() override;

	//--- ---------------------------------------------------------------------
	OBJ_METHODS(dbPlugProvider, FObject)
	REFCOUNT_METHODS(FObject)
    DEF_INTERFACES_2(ITestPlugProvider, ITestPlugProvider2, FObject)

    std::string const &GetName() { return this->classInfo.name(); }
	
//------------------------------------------------------------------------
protected:
	bool setupPlugin(Steinberg::FUnknown* hostContext);
	bool connectComponents();
	bool disconnectComponents();
	void terminatePlugin();

	PluginFactory factory;
	ClassInfo classInfo;
	Steinberg::IPtr<IComponent> component;
	Steinberg::IPtr<IEditController> controller;

	Steinberg::OPtr<Steinberg::Vst::ConnectionProxy> componentCP;
	Steinberg::OPtr<Steinberg::Vst::ConnectionProxy> controllerCP;

	bool plugIsGlobal;
};

class dbPluginContextFactory
{
public:
	static dbPluginContextFactory& instance()
	{
		static dbPluginContextFactory factory;
		return factory;
	}
	
	void setPluginContext(Steinberg::FUnknown* obj) { context = obj; }
	Steinberg::FUnknown* getPluginContext() const { return context; }

private:
	Steinberg::FUnknown* context;
};

#endif