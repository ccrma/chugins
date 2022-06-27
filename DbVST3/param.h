#ifndef Param_h
#define Param_h

#include <pluginterfaces/vst/ivsteditcontroller.h> // ParameterInfo
#include <string>
#include <iostream>

class ParamInfo : public Steinberg::Vst::ParameterInfo
{
public:
    std::string name;
    float currentValue;
    std::shared_ptr<std::vector<std::string>> menuItems;

    ParamInfo(Steinberg::Vst::ParameterInfo const &pinfo) :
        Steinberg::Vst::ParameterInfo(pinfo)
    {
        this->name = VST3::StringConvert::convert(pinfo.title);
        this->currentValue = pinfo.defaultNormalizedValue;
    }

#if 0
    ParamInfo(ParamInfo const &rhs) :
        Steinberg::Vst::ParameterInfo(rhs)
    {
        this->name = rhs.name;
        this->currentValue = rhs.currentValue;
        this->menuItems = rhs.menuItems;
    }
#endif

    ~ParamInfo()
    {
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
        ostr << indent << "- name: \"" << this->name << "\"\n";
        ostr << indent << "  id: " << this->id << "\n";

        bool emitEditHints = true;
        if(this->programChange())
        {
            // We establish a non-valid index as the default. 
            // NB: these are normalized and -1 signifies invalid.
            ostr << indent << "  default: -1\n";
            ostr << indent << "  description: >-\n";
            ostr << indent << "    When not set to (none), a preset may override\n";
            ostr << indent << "    all or some of the other parameter values.\n";
            ostr << indent << "  programchange:  1\n";
            if(this->menuItems && this->menuItems->size())
            {
                emitEditHints = false;
                ostr << indent << "  type: float\n"; // all vst3 params are floats
                ostr << indent << "  editType: optionIndexNormalized\n";
                ostr << indent << "  range:\n"; // an array
                for(int i=0;i<this->menuItems->size();i++)
                    ostr << indent << "    - '" << this->menuItems->at(i) << "'\n";
            }
            else
            {
                std::cerr << this->name << " (" <<
                    this->id << ") has no menuitems ########################\n";
            }
        }
        else
        {
            // Vexed only has zeros in the defaultValue. Seems
            // like the actualValue might be better anyways.
            ostr << indent << "  default: " << this->currentValue << "\n";
        }
        if(emitEditHints)
        {
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

    }
};

#endif