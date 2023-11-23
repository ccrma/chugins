#include "chugin.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>


CK_DLL_CTOR(foldback_ctor);
CK_DLL_DTOR(foldback_dtor);

CK_DLL_MFUN(foldback_setMakeupGain);
CK_DLL_MFUN(foldback_getMakeupGain);

CK_DLL_MFUN(foldback_setThreshold);
CK_DLL_MFUN(foldback_getThreshold);

CK_DLL_MFUN(foldback_setIndex);
CK_DLL_MFUN(foldback_getIndex);

CK_DLL_TICK(foldback_tick);

t_CKINT foldback_data_offset = 0;

struct FoldbackData
{
    float makeupGain;
    float threshold;
    float index;
};


CK_DLL_QUERY(FoldbackSaturator)
{
    QUERY->setname(QUERY, "FoldbackSaturator");
    
    QUERY->begin_class(QUERY, "FoldbackSaturator", "UGen");
    QUERY->doc_class(QUERY, "Foldback saturator that inverts a signal and multiplies it by an index as it passes a threshold.");
    QUERY->add_ex(QUERY, "effects/FoldbackSaturator-index.ck");
    QUERY->add_ex(QUERY, "effects/FoldbackSaturator-threshold.ck");
    
    QUERY->add_ctor(QUERY, foldback_ctor);
    QUERY->add_dtor(QUERY, foldback_dtor);
    
    QUERY->add_ugen_func(QUERY, foldback_tick, NULL, 1, 1);

    QUERY->add_mfun(QUERY, foldback_setMakeupGain, "float", "makeupGain");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "The amount of makeup gain applied to the signal after processing, multiplied against the reciprocal of the threshold. Defaults to 1.0");

    QUERY->add_mfun(QUERY, foldback_getMakeupGain, "float", "makeupGain");
    QUERY->doc_func(QUERY, "The amount of makeup gain applied to the signal after processing, multiplied against the reciprocal of the threshold. Defaults to 1.0");

    QUERY->add_mfun(QUERY, foldback_setThreshold, "float", "threshold");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "The threshold (positive and negative) that the signal is inverted against as it is passed. Defaults to 0.6");

    QUERY->add_mfun(QUERY, foldback_getThreshold, "float", "threshold");
    QUERY->doc_func(QUERY, "The threshold (positive and negative) that the signal is inverted against as it is passed. Defaults to 0.6");

    QUERY->add_mfun(QUERY, foldback_setIndex, "float", "index");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "The index that the signal is multiplied by after it is inverted against the threshold. Defaults to 2.0");

    QUERY->add_mfun(QUERY, foldback_getIndex, "float", "index");
    QUERY->doc_func(QUERY, "The index that the signal is multiplied by after it is inverted against the threshold. Defaults to 2.0");

    foldback_data_offset = QUERY->add_mvar(QUERY, "int", "@fb_data", false);
    
    QUERY->end_class(QUERY);

    return TRUE;
}


CK_DLL_CTOR(foldback_ctor)
{
    OBJ_MEMBER_INT(SELF, foldback_data_offset) = 0;

    FoldbackData * fbdata = new FoldbackData;
    fbdata->makeupGain = 1.0;
    fbdata->threshold = 0.6f;
    fbdata->index = 2.0;
    
    OBJ_MEMBER_INT(SELF, foldback_data_offset) = (t_CKINT) fbdata;
}

CK_DLL_DTOR(foldback_dtor)
{
    FoldbackData * fbdata = (FoldbackData *) OBJ_MEMBER_INT(SELF, foldback_data_offset);
    if(fbdata)
    {
        delete fbdata;
        OBJ_MEMBER_INT(SELF, foldback_data_offset) = 0;
        fbdata = NULL;
    }
}

CK_DLL_TICK(foldback_tick)
{
    FoldbackData * fbdata = (FoldbackData *) OBJ_MEMBER_INT(SELF, foldback_data_offset);
    
    SAMPLE theSample = in;

    if((theSample > fbdata->threshold) || (theSample < fbdata->threshold * -1.0)){
        theSample = fabs(fabs(fmod((float)(theSample - fbdata->threshold), (float)(fbdata->threshold * 4.0))) - fbdata->threshold * 2.0) - fbdata->threshold;
    }

    *out = theSample * (1.0/fbdata->threshold) * fbdata->makeupGain;

    return TRUE;
}

CK_DLL_MFUN(foldback_setMakeupGain)
{
    FoldbackData * fbdata = (FoldbackData *) OBJ_MEMBER_INT(SELF, foldback_data_offset);
    // TODO: sanity check
    fbdata->makeupGain = GET_NEXT_FLOAT(ARGS);
    RETURN->v_float = fbdata->makeupGain;
}

CK_DLL_MFUN(foldback_getMakeupGain)
{
    FoldbackData * fbdata = (FoldbackData *) OBJ_MEMBER_INT(SELF, foldback_data_offset);
    RETURN->v_float = fbdata->makeupGain;
}

CK_DLL_MFUN(foldback_setThreshold)
{
    FoldbackData * fbdata = (FoldbackData *) OBJ_MEMBER_INT(SELF, foldback_data_offset);
    float arg = GET_NEXT_FLOAT(ARGS);
    if(arg >= 0){
        fbdata->threshold = arg;
    }
    RETURN->v_float = fbdata->threshold;
}

CK_DLL_MFUN(foldback_getThreshold)
{
    FoldbackData * fbdata = (FoldbackData *) OBJ_MEMBER_INT(SELF, foldback_data_offset);
    RETURN->v_float = fbdata->threshold;
}

CK_DLL_MFUN(foldback_setIndex)
{
    FoldbackData * fbdata = (FoldbackData *) OBJ_MEMBER_INT(SELF, foldback_data_offset);
    // TODO: sanity check
    fbdata->index = GET_NEXT_FLOAT(ARGS);
    RETURN->v_float = fbdata->index;
}

CK_DLL_MFUN(foldback_getIndex)
{
    FoldbackData * fbdata = (FoldbackData *) OBJ_MEMBER_INT(SELF, foldback_data_offset);
    RETURN->v_float = fbdata->index;
}
