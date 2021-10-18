#ifndef DbVST3Param_h
#define DbVST3Param_h

#include <string>
#include <pluginterfaces/vst/ivsteditcontroller.h> // ParameterInfo

struct DbVST3ParamInfo
{
    std::string name;
    int stepCount; // 0: float, 1: toggle, 2: discreet
    Steinberg::Vst::ParamID id;
    double defaultValue; // normalized, (double is the type of VST::ParamValue)
    float currentValue;
    std::string units; // XXX: two kinds of units (string and int)
    bool readOnly;
    bool automatable;
    bool hidden;
    bool islist;
    bool isProgramChange;

    DbVST3ParamInfo(Steinberg::Vst::ParameterInfo &pinfo)
    {
        this->name = VST3::StringConvert::convert(pinfo.title);
        this->id = pinfo.id;
        this->defaultValue = pinfo.defaultNormalizedValue;
        this->stepCount = pinfo.stepCount;
        this->units = VST3::StringConvert::convert(pinfo.units);
        this->readOnly = pinfo.flags & Steinberg::Vst::ParameterInfo::kIsReadOnly;
        this->automatable = pinfo.flags & Steinberg::Vst::ParameterInfo::kCanAutomate;
        this->isProgramChange = pinfo.flags & Steinberg::Vst::ParameterInfo::kIsProgramChange;
        this->hidden = // (pinfo.flags == 0) || 
                        (pinfo.flags & Steinberg::Vst::ParameterInfo::kIsHidden);
            // flags == 0 case is for Midi CC;
            // some plugins add 16 * 128 automatable MIDI CC parameters
        this->islist = pinfo.flags & Steinberg::Vst::ParameterInfo::kIsList;
    }

    void Print(std::ostream &ostr, char const *indent, int index)
    {
        if(this->hidden) return;

        ostr << indent << "- name: \"" << this->name << "\"\n";
        ostr << indent << "  type: float\n"; // all vst3 params are floats
        ostr << indent << "  id: " << this->id << "\n";
        ostr << indent << "  default: " << this->defaultValue << "\n";
        if(this->stepCount == 1)
            ostr << indent << "  editType: checkbox\n";
        else
        {
            float delta;
            if(this->stepCount == 0) //
                delta = .01; /* .001 was too small.. */
            else
                delta = 1.0f / this->stepCount;
            ostr << indent << "  range: [0, 1, " << delta << "]\n";
        }

        ostr << indent << "  auto: " << this->automatable << "\n";
        if(this->units.size())
            ostr << indent << "  units: '" << this->units << "'\n";
        if(this->readOnly)
            ostr << indent << "  ro:  1\n";
        if(this->islist)
            ostr << indent << "  enum:  1\n";
        if(this->hidden)
            ostr << indent << "  hidden:  1\n";
        if(this->isProgramChange)
            ostr << indent << "  programchange:  1\n";
    }
};

#endif