

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>


CK_DLL_CTOR(saturator_ctor);
CK_DLL_DTOR(saturator_dtor);

CK_DLL_MFUN(saturator_setBits);
CK_DLL_MFUN(saturator_getBits);
CK_DLL_MFUN(saturator_setDownsampleFactor);
CK_DLL_MFUN(saturator_getDownsampleFactor);

CK_DLL_TICK(saturator_tick);

t_CKINT saturator_data_offset = 0;


struct SaturatorData
{
    int bits;
    int downsampleFactor;
    
    int currentSampleCount;
    SAMPLE currentSample;
};


CK_DLL_QUERY(Saturator)
{
    QUERY->setname(QUERY, "Saturator");
    
    QUERY->begin_class(QUERY, "Saturator", "UGen");
    
    QUERY->add_ctor(QUERY, saturator_ctor);
    QUERY->add_dtor(QUERY, saturator_dtor);
    
    QUERY->add_ugen_func(QUERY, saturator_tick, NULL, 1, 1);
    
    QUERY->add_mfun(QUERY, saturator_setBits, "int", "bits");
    QUERY->add_arg(QUERY, "int", "arg");
    
    QUERY->add_mfun(QUERY, saturator_getBits, "int", "bits");
    
    QUERY->add_mfun(QUERY, saturator_setDownsampleFactor, "int", "downsampleFactor");
    QUERY->add_arg(QUERY, "int", "arg");
    
    QUERY->add_mfun(QUERY, saturator_getDownsampleFactor, "int", "downsampleFactor");
    
    saturator_data_offset = QUERY->add_mvar(QUERY, "int", "@bc_data", false);
    
    QUERY->end_class(QUERY);

    return TRUE;
}


CK_DLL_CTOR(saturator_ctor)
{
    OBJ_MEMBER_INT(SELF, saturator_data_offset) = 0;
    
    SaturatorData * bcdata = new SaturatorData;
    bcdata->bits = 32;
    bcdata->downsampleFactor = 1;
    bcdata->currentSampleCount = 0;
    bcdata->currentSample = 0;
    
    OBJ_MEMBER_INT(SELF, saturator_data_offset) = (t_CKINT) bcdata;
}

CK_DLL_DTOR(saturator_dtor)
{
    SaturatorData * bcdata = (SaturatorData *) OBJ_MEMBER_INT(SELF, saturator_data_offset);
    if(bcdata)
    {
        delete bcdata;
        OBJ_MEMBER_INT(SELF, saturator_data_offset) = 0;
        bcdata = NULL;
    }
}

CK_DLL_TICK(saturator_tick)
{
    SaturatorData * bcdata = (SaturatorData *) OBJ_MEMBER_INT(SELF, saturator_data_offset);
    
    SAMPLE theSample;

    if((bcdata->currentSampleCount % bcdata->downsampleFactor) == 0)
    {
        // clamp to [-1,1]
        if(in > 1) in = 1;
        else if(in < -1) in = -1;
        
        // sample and hold
        bcdata->currentSample = theSample = in;
    }
    else
    {
        // decimate!
        theSample = bcdata->currentSample;
    }
    
    bcdata->currentSampleCount = (bcdata->currentSampleCount+1) % bcdata->downsampleFactor;
    
    // convert to 32-bit int
    int shift = 32-bcdata->bits;
    int q32 = theSample * INT_MAX;
    q32 = (q32 >> shift) << shift;
    
    *out = q32 / ((float) INT_MAX);

    return TRUE;
}

CK_DLL_MFUN(saturator_setBits)
{
    SaturatorData * bcdata = (SaturatorData *) OBJ_MEMBER_INT(SELF, saturator_data_offset);
    // TODO: sanity check
    bcdata->bits = GET_NEXT_INT(ARGS);
    RETURN->v_int = bcdata->bits;
}

CK_DLL_MFUN(saturator_getBits)
{
    SaturatorData * bcdata = (SaturatorData *) OBJ_MEMBER_INT(SELF, saturator_data_offset);
    RETURN->v_int = bcdata->bits;
}

CK_DLL_MFUN(saturator_setDownsampleFactor)
{
    SaturatorData * bcdata = (SaturatorData *) OBJ_MEMBER_INT(SELF, saturator_data_offset);
    // TODO: sanity check
    bcdata->downsampleFactor = GET_NEXT_INT(ARGS);
    RETURN->v_int = bcdata->downsampleFactor;
}

CK_DLL_MFUN(saturator_getDownsampleFactor)
{
    SaturatorData * bcdata = (SaturatorData *) OBJ_MEMBER_INT(SELF, saturator_data_offset);
    RETURN->v_int = bcdata->downsampleFactor;
}


