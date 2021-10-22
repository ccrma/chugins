#ifndef DbVST3Param_h
#define DbVST3Param_h

#include <string>
#include <pluginterfaces/vst/ivsteditcontroller.h> // ParameterInfo

class DbVST3ParamInfo : public Steinberg::Vst::ParameterInfo
{
public:
    std::string name;
    float currentValue;

    DbVST3ParamInfo(Steinberg::Vst::ParameterInfo const &pinfo) :
        Steinberg::Vst::ParameterInfo(pinfo)
    {
        this->name = VST3::StringConvert::convert(pinfo.title);
        this->currentValue = pinfo.defaultNormalizedValue;
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

    bool isList() const
    {
        return this->flags & Steinberg::Vst::ParameterInfo::kIsList;
    }

    void Print(std::ostream &ostr, char const *indent, int index)
    {
        if(this->flags & Steinberg::Vst::ParameterInfo::kIsHidden)
            return;

        ostr << indent << "- name: \"" << this->name << "\"\n";
        ostr << indent << "  id: " << this->id << "\n";
        ostr << indent << "  default: " << this->defaultNormalizedValue << "\n";
        if(this->stepCount == 1)
        {
            // checkboxes only work for int (even though VST3 is float)
            ostr << indent << "  type: int\n";
            ostr << indent << "  editType: checkbox\n";
        }
        else
        {
            float delta;
            if(this->stepCount == 0) //
                delta = .01; /* .001 was too small.. */
            else
                delta = 1.0f / this->stepCount;
            ostr << indent << "  type: float\n"; // all vst3 params are floats
            ostr << indent << "  range: [0, 1, " << delta << "]\n";
        }
        
        if(this->automatable())
            ostr << indent << "  auto: " << 1 << "\n";
        std::string ustr = VST3::StringConvert::convert(this->units);
        if(ustr.size())
            ostr << indent << "  units: '" << ustr << "'\n";
        if(this->readOnly())
            ostr << indent << "  ro:  1\n";
        if(this->isList())
            ostr << indent << "  enum:  1\n";
        if(this->hidden())
            ostr << indent << "  hidden:  1\n";
        if(this->programChange())
            ostr << indent << "  programchange:  1\n";
    }
};

#endif