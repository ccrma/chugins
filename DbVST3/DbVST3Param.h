#ifndef DbVST3Param_h
#define DbVST3Param_h

#include <string>
#include <pluginterfaces/vst/ivsteditcontroller.h> // ParameterInfo

class DbVST3ParamInfo : public Steinberg::Vst::ParameterInfo
{
public:
    std::string name;

    DbVST3ParamInfo(Steinberg::Vst::ParameterInfo const &pinfo) :
        Steinberg::Vst::ParameterInfo(pinfo)
    {
        this->name = VST3::StringConvert::convert(pinfo.title);
    }

    bool hidden() const
    {
        return this->flags & Steinberg::Vst::ParameterInfo::kIsHidden;
    }

    bool automatable() const
    {
        return this->flags & Steinberg::Vst::ParameterInfo::kCanAutomate;
    }

    bool readOnly() const
    {
        return this->flags & Steinberg::Vst::ParameterInfo::kIsReadOnly;
    }

    bool programChange() const
    {
        return this->flags & Steinberg::Vst::ParameterInfo::kIsProgramChange;
    }

/*
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
*/

    void Print(std::ostream &ostr, char const *indent, int index)
    {
        if(this->flags & Steinberg::Vst::ParameterInfo::kIsHidden)
            return;

        ostr << indent << "- name: \"" << this->name << "\"\n";
        ostr << indent << "  type: float\n"; // all vst3 params are floats
        ostr << indent << "  id: " << this->id << "\n";
        ostr << indent << "  default: " << this->defaultNormalizedValue << "\n";
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
        
        if(this->flags & Steinberg::Vst::ParameterInfo::kCanAutomate)
            ostr << indent << "  auto: " << 1 << "\n";
        std::string ustr = VST3::StringConvert::convert(this->units);
        if(ustr.size())
            ostr << indent << "  units: '" << ustr << "'\n";
        if(this->flags & Steinberg::Vst::ParameterInfo::kIsReadOnly)
            ostr << indent << "  ro:  1\n";
        if(this->flags & Steinberg::Vst::ParameterInfo::kIsList)
            ostr << indent << "  enum:  1\n";
        if(this->flags & Steinberg::Vst::ParameterInfo::kIsHidden)
            ostr << indent << "  hidden:  1\n";
        if(this->flags & Steinberg::Vst::ParameterInfo::kIsProgramChange)
            ostr << indent << "  programchange:  1\n";
    }
};

#endif