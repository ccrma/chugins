#ifndef vst3_h
#define vst3_h

// (lots of headers)

#include <public.sdk/source/vst/hosting/hostclasses.h>
#include <public.sdk/source/vst/hosting/module.h>
#include <public.sdk/source/vst/hosting/pluginterfacesupport.h>
#include <public.sdk/source/vst/hosting/processdata.h>
#include <public.sdk/source/vst/hosting/eventlist.h>
#include <public.sdk/source/vst/hosting/parameterchanges.h>
#include <public.sdk/source/vst/utility/stringconvert.h>
#include <public.sdk/source/vst/vsteditcontroller.h>

#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/base/ftypes.h>
#include <pluginterfaces/base/fplatform.h>

#include <pluginterfaces/vst/ivstcomponent.h>
#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include <pluginterfaces/vst/ivstmidicontrollers.h>
#include <pluginterfaces/vst/ivstevents.h>
#include <pluginterfaces/vst/vsttypes.h>
#include <pluginterfaces/vst/ivsttestplugprovider.h>
#include <base/source/fobject.h>

using Plugin = VST3::Hosting::Module::Ptr;
using tresult = Steinberg::tresult;
using ParamID = Steinberg::Vst::ParamID;
using ParamValue = Steinberg::Vst::ParamValue;
using FObject = Steinberg::FObject;
using int32 = Steinberg::int32;
using uint32 = Steinberg::uint32;
using FUID = Steinberg::FUID;
using TUID = Steinberg::TUID;
using IStringResult = Steinberg::IStringResult;
using IComponent = Steinberg::Vst::IComponent;
using IEditController = Steinberg::Vst::IEditController;

#endif