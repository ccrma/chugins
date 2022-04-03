#ifndef mdaParam_h
#define mdaParam_h

// Here's our minimalist notion on parameters:
// two kinds: ParamDef and ParamValue
// ParamDefs are covariant with the class def
// ParamValues are for instances and interpretted relative
// to its ParamDef.
// Both live in arrays of the same size and the id is the 
// array index. Direct access to def is currently builtin 
// to ParamValue.

struct ParamDef
{
    enum Units
    {
        kPct,
        kRatio,
        kUInt,
        kInt,
        kCents,
        kHz
    };

    enum Flags
    {
        kAutomatable = 1 << 0,
        kIsProgramChange = 1 << 1,
        kIsBypass = 1 << 2,
    };

    struct Range
    {
        Range()
        {
            min = 0.f;
            max = 1.f;
            step = .01f;
        }
        Range(const Range &rhs)
        {
            min = rhs.min;
            max = rhs.max;
            step = rhs.step;
        }
        Range(float mn, float mx, float s)
        {
            min = mn;
            max = mx;
            step = s;
        }
        float min, max, step;
    };

    ParamDef(char const *n, char const *mr, float def, 
        Range r, Units u, Flags f,
        unsigned i)
    {
        name = n;
        midiRole = mr;
        default = def;
        range = r;
        units = u;
        flags = f;
        id = i;
    };
    char const *name;
    char const *midiRole; // eg: "PitchBend" "ModWheel", "CCXX"
    float default; // could be a union
    Range range;
    Units units;
    Flags flags;
    unsigned id;
};

struct ParamValue
{
    ParamValue()
    {
        def = nullptr;
    }
    ParamValue(ParamDef const *d, float val)
    {
        def = d;
        value = val;
    }
    ParamDef const *def;
    float value;
};

#endif