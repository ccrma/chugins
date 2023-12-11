//-----------------------------------------------------------------------------
// name: Range.cpp
// desc: Scale an input signal from one number range to another.
//       This is useful for things such as making LFOs that drive
//       parameters that are do not fall under the output range of
//       most oscillators, like vibrato.
//
// example:
// // Use range to make an easy vibrato
// SinOsc lfo = > Range r = > blackhole;
// SinOsc tone = > dac;
//
// // Help reveals all secrets
// Range.help();
//
// 3 = > lfo.freq;
// (0, 1, 440, 10) = > r.radius;
//
// while (10::ms = > now) {
//    r.last() = > tone.freq;
// }
//
// authors: Nick Shaheed (nshaheed@ccrma.stanford.edu)
// date: Winter 2022
//
//-----------------------------------------------------------------------------

// this should align with the correct versions of these ChucK files
#include "chugin.h"

// general includes
#include <stdio.h>
#include <limits.h>

// declaration of chugin constructor
CK_DLL_CTOR(range_ctor);
CK_DLL_CTOR(range_ctor_out);
CK_DLL_CTOR(range_ctor_all);
// declaration of chugin desctructor
CK_DLL_DTOR(range_dtor);

// getters and setters
CK_DLL_MFUN(range_setInRange);
CK_DLL_MFUN(range_setInRadius);
CK_DLL_MFUN(range_setOutRange);
CK_DLL_MFUN(range_setOutRadius);
CK_DLL_MFUN(range_setRange);
CK_DLL_MFUN(range_setRadius);

CK_DLL_MFUN(range_getInMin);
CK_DLL_MFUN(range_getInMax);
CK_DLL_MFUN(range_getOutMin);
CK_DLL_MFUN(range_getOutMax);
CK_DLL_MFUN(range_getInCenter);
CK_DLL_MFUN(range_getInRadius);
CK_DLL_MFUN(range_getOutCenter);
CK_DLL_MFUN(range_getOutRadius);

CK_DLL_MFUN(range_setClip);
CK_DLL_MFUN(range_getClip);

// for Chugins extending UGen, this is mono synthesis function for 1 sample
CK_DLL_TICK(range_tick);

// this is a special offset reserved for Chugin internal data
t_CKINT range_data_offset = 0;

class Range
{
public:
    // constructor
    Range()
    {
        m_param = 0;
        m_in_min = -1;
        m_in_max = 1;
        m_out_min = 0;
        m_out_max = 1;
        m_clip = 0;
    }

    // for Chugins extending UGen
    SAMPLE tick( SAMPLE in )
    {
        SAMPLE out = (in - m_in_min) / (m_in_max - m_in_min);
        out = out * (m_out_max - m_out_min) + m_out_min;

        if (!m_clip) return out;

        // hard clipping - nothing past the limits allowed.
        if (out < m_out_min) out = m_out_min;
        if (out > m_out_max) out = m_out_max;

        // default: this passes whatever input is patched into Chugin
        return out;
    }

    t_CKVOID setInRange( t_CKFLOAT in_min, t_CKFLOAT in_max ) {
        m_in_min = in_min;
        m_in_max = in_max;
    }

    t_CKVOID setInRadius( t_CKFLOAT in_center, t_CKFLOAT in_radius ) {
        m_in_min = in_center - in_radius;
        m_in_max = in_center + in_radius;
    }

    t_CKVOID setOutRange( t_CKFLOAT out_min, t_CKFLOAT out_max ) {
        m_out_min = out_min;
        m_out_max = out_max;
    }

    t_CKVOID setOutRadius( t_CKFLOAT out_center, t_CKFLOAT out_radius ) {
        m_out_min = out_center - out_radius;
        m_out_max = out_center + out_radius;
    }

    t_CKVOID setRange( t_CKFLOAT in_min, t_CKFLOAT in_max, t_CKFLOAT out_min, t_CKFLOAT out_max ) {
        m_in_min = in_min;
        m_in_max = in_max;
        m_out_min = out_min;
        m_out_max = out_max;
    }

    t_CKVOID setRadius(t_CKFLOAT in_center, t_CKFLOAT in_radius, t_CKFLOAT out_center, t_CKFLOAT out_radius) {
        m_in_min = in_center - in_radius;
        m_in_max = in_center + in_radius;
        m_out_min = out_center - out_radius;
        m_out_max = out_center + out_radius;
    }

    t_CKINT setClip(t_CKINT val)
    {
        m_clip = val;
        return val;
    }

    t_CKINT getClip()
    {
        return m_clip;
    }

    // get parameter example
    t_CKFLOAT getInMin() { return m_in_min; }
    t_CKFLOAT getInMax() { return m_in_max; }
    t_CKFLOAT getOutMin() { return m_out_min; }
    t_CKFLOAT getOutMax() { return m_out_max; }

    t_CKFLOAT getInCenter() { return (m_in_min + m_in_max) / 2; }
    t_CKFLOAT getInRadius() { return (m_in_max - m_in_min) / 2; }
    t_CKFLOAT getOutCenter() { return (m_out_min + m_out_max) / 2; }
    t_CKFLOAT getOutRadius() { return (m_out_max - m_out_min) / 2; }


private:
    // instance data
    t_CKFLOAT m_param;
    t_CKFLOAT m_in_min, m_in_max, m_out_min, m_out_max;
    t_CKINT m_clip;
};


// query function: chuck calls this when loading the Chugin
// NOTE: developer will need to modify this function to
// add additional functions to this Chugin
CK_DLL_QUERY( Range )
{
    QUERY->setname(QUERY, "Range");

    // ------------------------------------------------------------------------
    // Chugin metadata
    // ------------------------------------------------------------------------
    QUERY->setinfo( QUERY, CHUGIN_INFO_AUTHORS, "Nick Shaheed" );
    // the version string of this chugin, e.g., "v1.2"
    QUERY->setinfo( QUERY, CHUGIN_INFO_CHUGIN_VERSION, "1.0" );
    // text description of this chugin; what is it? what does it do? who is it for?
    QUERY->setinfo( QUERY, CHUGIN_INFO_DESCRIPTION, "Linearly scale an input signal to a different range of numbers." );
    // (optional) URL of the homepage for this chugin
    QUERY->setinfo( QUERY, CHUGIN_INFO_URL, "https://github.com/ccrma/chugins/tree/main/Range" );
    // (optional) contact email
    QUERY->setinfo( QUERY, CHUGIN_INFO_EMAIL, "nshaheed@ccrma.stanford.edu" );

    // begin the class definition
    // can change the second argument to extend a different ChucK class
    QUERY->begin_class(QUERY, "Range", "UGen");

    // register the constructor (probably no need to change)
    QUERY->add_ctor(QUERY, range_ctor);

    QUERY->add_ctor(QUERY, range_ctor_out);
    QUERY->add_arg(QUERY, "float", "outMin" );
    QUERY->add_arg(QUERY, "float", "outMax" );
    QUERY->doc_func(QUERY, "Constructor that expects an input "
                    "range of [-1, 1] and an output range of [outMin, outMax]");

    QUERY->add_ctor(QUERY, range_ctor_all);
    QUERY->add_arg(QUERY, "float", "inMin" );
    QUERY->add_arg(QUERY, "float", "inMax" );
    QUERY->add_arg(QUERY, "float", "outMin" );
    QUERY->add_arg(QUERY, "float", "outMax" );
    QUERY->doc_func(QUERY, "Constructor that expects an input "
                    "range of [inMin, inMax] and an output range of [outMin, outMax]");

    // register the destructor (probably no need to change)
    QUERY->add_dtor(QUERY, range_dtor);

    // for UGen's only: add tick function
    QUERY->add_ugen_func(QUERY, range_tick, NULL, 1, 1);

    // NOTE: if this is to be a UGen with more than 1 channel,
    // e.g., a multichannel UGen -- will need to use add_ugen_funcf()
    // and declare a tickf function using CK_DLL_TICKF

    QUERY->add_mfun(QUERY, range_setInRange, "void", "inRange");
    QUERY->add_arg(QUERY, "float", "min");
    QUERY->add_arg(QUERY, "float", "max");
    QUERY->doc_func(QUERY, "Set the expected input range.");

    QUERY->add_mfun(QUERY, range_setInRadius, "void", "inRadius");
    QUERY->add_arg(QUERY, "float", "center");
    QUERY->add_arg(QUERY, "float", "radius");
    QUERY->doc_func(QUERY, "Set the expected input radius.");

    QUERY->add_mfun(QUERY, range_setOutRange, "void", "outRange");
    QUERY->add_arg(QUERY, "float", "min");
    QUERY->add_arg(QUERY, "float", "max");
    QUERY->doc_func(QUERY, "Set the output range.");

    QUERY->add_mfun(QUERY, range_setOutRadius, "void", "outRadius");
    QUERY->add_arg(QUERY, "float", "center");
    QUERY->add_arg(QUERY, "float", "radius");
    QUERY->doc_func(QUERY, "Set the output radius.");

    QUERY->add_mfun(QUERY, range_setRange, "void", "range");
    QUERY->add_arg(QUERY, "float", "inMin");
    QUERY->add_arg(QUERY, "float", "inMax");
    QUERY->add_arg(QUERY, "float", "outMin");
    QUERY->add_arg(QUERY, "float", "outMax");
    QUERY->doc_func(QUERY, "Set the expected input range and desired output range.");

    QUERY->add_mfun(QUERY, range_setRadius, "void", "radius");
    QUERY->add_arg(QUERY, "float", "inCenter");
    QUERY->add_arg(QUERY, "float", "inRadius");
    QUERY->add_arg(QUERY, "float", "outCenter");
    QUERY->add_arg(QUERY, "float", "outRadius");
    QUERY->doc_func(QUERY, "Set the expected input range and desired output range in terms of a center with a radius.");

    QUERY->add_mfun(QUERY, range_getInMin, "float", "inMin");
    QUERY->doc_func(QUERY, "Get input min.");

    QUERY->add_mfun(QUERY, range_getInMax, "float", "inMax");
    QUERY->doc_func(QUERY, "Get input max.");

    QUERY->add_mfun(QUERY, range_getOutMin, "float", "outMin");
    QUERY->doc_func(QUERY, "Get output min.");

    QUERY->add_mfun(QUERY, range_getOutMax, "float", "outMax");
    QUERY->doc_func(QUERY, "Get output max.");

    QUERY->add_mfun(QUERY, range_getInCenter, "float", "inCenter");
    QUERY->doc_func(QUERY, "Get input center.");

    QUERY->add_mfun(QUERY, range_getInRadius, "float", "inRad");
    QUERY->doc_func(QUERY, "Get input radius.");

    QUERY->add_mfun(QUERY, range_getOutCenter, "float", "outCenter");
    QUERY->doc_func(QUERY, "Get output center.");

    QUERY->add_mfun(QUERY, range_getOutRadius, "float", "outRad");
    QUERY->doc_func(QUERY, "Get output radius.");

    QUERY->add_mfun(QUERY, range_setClip, "int", "clip");
    QUERY->add_arg(QUERY, "int", "val");
    QUERY->doc_func(QUERY, "Set Range's clipping mode. Accepted values are: 0 (no clipping), 1 (hard clipping). Defaults to 0");

    QUERY->add_mfun(QUERY, range_getClip, "int", "clip");
    QUERY->doc_func(QUERY, "Get clip state.");


    // this reserves a variable in the ChucK internal class to store
    // referene to the c++ class we defined above
    range_data_offset = QUERY->add_mvar(QUERY, "int", "@s_data", false);

    // end the class definition
    // IMPORTANT: this MUST be called!
    QUERY->end_class(QUERY);

    // wasn't that a breeze?
    return TRUE;
}


// implementation for the constructor
CK_DLL_CTOR(range_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, range_data_offset) = 0;

    // instantiate our internal c++ class representation
    Range * s_obj = new Range();

    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, range_data_offset) = (t_CKINT) s_obj;
}


// implementation for the constructor
CK_DLL_CTOR(range_ctor_out)
{
    range_ctor(SELF, ARGS, VM, SHRED, API);

    Range * r_obj = (Range *) OBJ_MEMBER_INT(SELF, range_data_offset);

    t_CKFLOAT outMin = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT outMax = GET_NEXT_FLOAT(ARGS);

    r_obj->setOutRange(outMin, outMax);
}


// implementation for the constructor
CK_DLL_CTOR(range_ctor_all)
{
    range_ctor(SELF, ARGS, VM, SHRED, API);

    Range * r_obj = (Range *) OBJ_MEMBER_INT(SELF, range_data_offset);

    t_CKFLOAT inMin = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT inMax = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT outMin = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT outMax = GET_NEXT_FLOAT(ARGS);

    r_obj->setRange(inMin, inMax, outMin, outMax);
}


// implementation for the destructor
CK_DLL_DTOR(range_dtor)
{
    // get our c++ class pointer
    Range * r_obj = (Range *) OBJ_MEMBER_INT(SELF, range_data_offset);
    // check it
    if( r_obj )
    {
        // clean up
        delete r_obj;
        OBJ_MEMBER_INT(SELF, range_data_offset) = 0;
        r_obj = NULL;
    }
}


// implementation for tick function
CK_DLL_TICK(range_tick)
{
    // get our c++ class pointer
    Range * r_obj = (Range *) OBJ_MEMBER_INT(SELF, range_data_offset);

    // invoke our tick function; store in the magical out variable
    if(r_obj) *out = r_obj->tick(in);

    // yes
    return TRUE;
}

CK_DLL_MFUN(range_setInRange)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);

    t_CKFLOAT inMin = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT inMax = GET_NEXT_FLOAT(ARGS);

    // set the return value
    r_obj->setInRange(inMin, inMax);
}

CK_DLL_MFUN(range_setInRadius)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);

    t_CKFLOAT inCenter = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT inRadius = GET_NEXT_FLOAT(ARGS);

    // set the return value
    r_obj->setInRadius(inCenter, inRadius);
}

CK_DLL_MFUN(range_setOutRange)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);

    t_CKFLOAT outMin = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT outMax = GET_NEXT_FLOAT(ARGS);

    // set the return value
    r_obj->setOutRange(outMin, outMax);
}

CK_DLL_MFUN(range_setOutRadius)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);

    t_CKFLOAT outCenter = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT outRadius = GET_NEXT_FLOAT(ARGS);

    // set the return value
    r_obj->setOutRadius(outCenter, outRadius);
}

CK_DLL_MFUN(range_setRange)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);

    t_CKFLOAT inMin = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT inMax = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT outMin = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT outMax = GET_NEXT_FLOAT(ARGS);

    // set the return value
    r_obj->setRange(inMin, inMax, outMin, outMax);
}

CK_DLL_MFUN(range_setRadius)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);

    t_CKFLOAT inCenter = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT inRadius = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT outCenter = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT outRadius = GET_NEXT_FLOAT(ARGS);

    // set the return value
    r_obj->setRadius(inCenter, inRadius, outCenter, outRadius);
}

CK_DLL_MFUN(range_getInMin)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);
    // set the return value
    RETURN->v_float = r_obj->getInMin();
}

CK_DLL_MFUN(range_getInMax)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);
    // set the return value
    RETURN->v_float = r_obj->getInMax();
}

CK_DLL_MFUN(range_getOutMin)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);
    // set the return value
    RETURN->v_float = r_obj->getOutMin();
}

CK_DLL_MFUN(range_getOutMax)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);
    // set the return value
    RETURN->v_float = r_obj->getOutMax();
}

CK_DLL_MFUN(range_getInCenter)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);
    // set the return value
    RETURN->v_float = r_obj->getInCenter();
}

CK_DLL_MFUN(range_getInRadius)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);
    // set the return value
    RETURN->v_float = r_obj->getInRadius();
}

CK_DLL_MFUN(range_getOutCenter)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);
    // set the return value
    RETURN->v_float = r_obj->getOutCenter();
}

CK_DLL_MFUN(range_getOutRadius)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);
    // set the return value
    RETURN->v_float = r_obj->getOutRadius();
}

CK_DLL_MFUN(range_setClip)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);
    // set the return value
    RETURN->v_int = r_obj->setClip(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(range_getClip)
{
    // get our c++ class pointer
    Range* r_obj = (Range*)OBJ_MEMBER_INT(SELF, range_data_offset);
    // set the return value
    RETURN->v_int = r_obj->getClip();
}
