// A modified version of the vst3 file:
//  public.sdk/source/vst/hosting/plugprovider.h
// A plugprovider is associated with each module/pluginInstance
#ifndef plugprovider_h
#define plugprovider_h

#include "vst3.h"
#include <public.sdk/source/vst/hosting/connectionproxy.h>
#include <iostream>

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
	dbPlugProvider(Steinberg::Vst::IHostApplication *hostCtx,
                   const PluginFactory& factory, ClassInfo info, 
                   bool plugIsGlobal=true);
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
private:
	bool setupPlugin(Steinberg::FUnknown* hostContext);
    bool setupExtra();
	bool connectComponents();
	bool disconnectComponents();
	void terminatePlugin();

	PluginFactory factory;
    Steinberg::Vst::IHostApplication *hostContext;
	Steinberg::IPtr<IComponent> component;
	Steinberg::IPtr<IEditController> editController;
	ClassInfo classInfo;

	Steinberg::OPtr<Steinberg::Vst::ConnectionProxy> componentCP;
	Steinberg::OPtr<Steinberg::Vst::ConnectionProxy> controllerCP;

    // misc handles to grab hold of during construction to ensure
    // consistency (in the worker thread) (defensive).
    // These are all the potential interfaces we can expect our
    // audioplugin to provide (either via component or controller).
    Steinberg::FUnknownPtr<Steinberg::Vst::IEditController2> editController2;
    Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping> midiMapping;
    Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor> audioProcessor;
    Steinberg::FUnknownPtr<Steinberg::Vst::IComponentHandler> componentHandler;
    Steinberg::FUnknownPtr<Steinberg::Vst::IComponentHandler2> componentHandler2;
    Steinberg::FUnknownPtr<Steinberg::Vst::IUnitInfo> unitInfo;
    Steinberg::FUnknownPtr<Steinberg::Vst::IUnitData> unitData;
    Steinberg::FUnknownPtr<Steinberg::Vst::IProgramListData> programListData;
    // Steinberg::IPtr<Steinberg::Vst::ChannelContext::IInfoListener> trackInfoListener;

	bool plugIsGlobal;
    bool componentIsController;
};

#endif