#ifndef vst3_h
#define vst3_h

// (lots of headers)

#include <public.sdk/source/vst/hosting/hostclasses.h>
#include <public.sdk/source/vst/hosting/module.h>
#include <public.sdk/source/vst/hosting/pluginterfacesupport.h>
#include <public.sdk/source/vst/hosting/plugprovider.h>
#include <public.sdk/source/vst/hosting/processdata.h>
#include <public.sdk/source/vst/hosting/eventlist.h>
#include <public.sdk/source/vst/hosting/parameterchanges.h>
#include <public.sdk/source/vst/utility/stringconvert.h>
#include <public.sdk/source/vst/vsteditcontroller.h>

#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include <pluginterfaces/vst/ivstmidicontrollers.h>
#include <pluginterfaces/vst/ivstevents.h>
#include <pluginterfaces/vst/vsttypes.h>
#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/base/ftypes.h>
#include <pluginterfaces/base/fplatform.h>
#include <base/source/fobject.h>

using Plugin = VST3::Hosting::Module::Ptr;
using VSTProvider = Steinberg::Vst::PlugProvider;
using VSTProviderPtr = Steinberg::IPtr<VSTProvider>;
using tresult = Steinberg::tresult;
using ParamID = Steinberg::Vst::ParamID;
using ParamValue = Steinberg::Vst::ParamValue;
using int32 = Steinberg::int32;
using uint32 = Steinberg::uint32;
using TUID = Steinberg::TUID;

#endif