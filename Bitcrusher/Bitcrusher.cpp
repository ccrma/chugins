

#include "chugin.h"

#include <stdio.h>
#include <limits.h>


CK_DLL_CTOR(bitcrusher_ctor);
CK_DLL_DTOR(bitcrusher_dtor);

CK_DLL_MFUN(bitcrusher_setBits);
CK_DLL_MFUN(bitcrusher_getBits);
CK_DLL_MFUN(bitcrusher_setDownsampleFactor);
CK_DLL_MFUN(bitcrusher_getDownsampleFactor);

CK_DLL_TICK(bitcrusher_tick);

t_CKINT bitcrusher_data_offset = 0;


struct BitcrusherData
{
    t_CKINT bits;
    t_CKINT downsampleFactor;
    
    int currentSampleCount;
    SAMPLE currentSample;
};


CK_DLL_QUERY(Bitcrusher)
{
    QUERY->setname(QUERY, "Bitcrusher");
    
    QUERY->begin_class(QUERY, "Bitcrusher", "UGen");
    QUERY->doc_class(QUERY, "Applies aliased downsampling and sample-width reduction to create a variety of distortion effects. ");
    QUERY->add_ex(QUERY, "effects/Bitcrusher.ck");
    
    QUERY->add_ctor(QUERY, bitcrusher_ctor);
    QUERY->add_dtor(QUERY, bitcrusher_dtor);
    
    QUERY->add_ugen_func(QUERY, bitcrusher_tick, NULL, 1, 1);
    
    QUERY->add_mfun(QUERY, bitcrusher_setBits, "int", "bits");
    QUERY->add_arg(QUERY, "int", "arg");
    QUERY->doc_func(QUERY, "Number of bits to reduce signal to [1-32].");
    
    QUERY->add_mfun(QUERY, bitcrusher_getBits, "int", "bits");
    QUERY->doc_func(QUERY, "Number of bits to reduce signal to [1-32].");
    
    QUERY->add_mfun(QUERY, bitcrusher_setDownsampleFactor, "int", "downsampleFactor");
    QUERY->add_arg(QUERY, "int", "arg");
    QUERY->doc_func(QUERY, "Factor by which to downsample signal by decimation [&ge;1].");
    
    QUERY->add_mfun(QUERY, bitcrusher_getDownsampleFactor, "int", "downsampleFactor");
    QUERY->doc_func(QUERY, "Factor by which to downsample signal by decimation [&ge;1].");
    
    QUERY->add_mfun(QUERY, bitcrusher_setDownsampleFactor, "int", "downsample");
    QUERY->add_arg(QUERY, "int", "arg");
    QUERY->doc_func(QUERY, "Factor by which to downsample signal by decimation [&ge;1].");
    
    QUERY->add_mfun(QUERY, bitcrusher_getDownsampleFactor, "int", "downsample");
    QUERY->doc_func(QUERY, "Factor by which to downsample signal by decimation [&ge;1].");
    
    bitcrusher_data_offset = QUERY->add_mvar(QUERY, "int", "@bc_data", false);
    
    QUERY->end_class(QUERY);

    return TRUE;
}


CK_DLL_CTOR(bitcrusher_ctor)
{
    OBJ_MEMBER_INT(SELF, bitcrusher_data_offset) = 0;
    
    BitcrusherData * bcdata = new BitcrusherData;
    bcdata->bits = 32;
    bcdata->downsampleFactor = 1;
    bcdata->currentSampleCount = 0;
    bcdata->currentSample = 0;
    
    OBJ_MEMBER_INT(SELF, bitcrusher_data_offset) = (t_CKINT) bcdata;
}

CK_DLL_DTOR(bitcrusher_dtor)
{
    BitcrusherData * bcdata = (BitcrusherData *) OBJ_MEMBER_INT(SELF, bitcrusher_data_offset);
    if(bcdata)
    {
        delete bcdata;
        OBJ_MEMBER_INT(SELF, bitcrusher_data_offset) = 0;
        bcdata = NULL;
    }
}

CK_DLL_TICK(bitcrusher_tick)
{
    BitcrusherData * bcdata = (BitcrusherData *) OBJ_MEMBER_INT(SELF, bitcrusher_data_offset);
    
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
    t_CKINT shift = 32-bcdata->bits;
    // cast to double for avoid floating point precision warning/error | (ge) 2023.4.14
    // "implicit conversion from 'int' to 'float' changes value from 2147483647 to 2147483648"
    // https://stackoverflow.com/questions/23420783/convert-int-max-to-float-and-then-back-to-integer
    // for example...
    //     int i = ((float)(INT_MAX)); // on mac (intel and arm64) seem to produce arbitrary and different values
    //     int i = ((float)(INT_MAX)-64); // on mac appears stable
    //     int i = ((double)(INT_MAX)); // on mac appears stable
    //     int i = ((double)(INT_MAX)+.1); // on mac appears stable
    //     int i = ((double)(INT_MAX)+1); // on mac not stable
    int q32 = ((double)theSample) * INT_MAX;
    q32 = (q32 >> shift) << shift;
    *out = q32 / ((float) INT_MAX);

    return TRUE;
}

CK_DLL_MFUN(bitcrusher_setBits)
{
    BitcrusherData * bcdata = (BitcrusherData *) OBJ_MEMBER_INT(SELF, bitcrusher_data_offset);
    // TODO: sanity check
    bcdata->bits = GET_NEXT_INT(ARGS);
    RETURN->v_int = bcdata->bits;
}

CK_DLL_MFUN(bitcrusher_getBits)
{
    BitcrusherData * bcdata = (BitcrusherData *) OBJ_MEMBER_INT(SELF, bitcrusher_data_offset);
    RETURN->v_int = bcdata->bits;
}

CK_DLL_MFUN(bitcrusher_setDownsampleFactor)
{
    BitcrusherData * bcdata = (BitcrusherData *) OBJ_MEMBER_INT(SELF, bitcrusher_data_offset);
    // TODO: sanity check
    bcdata->downsampleFactor = GET_NEXT_INT(ARGS);
    RETURN->v_int = bcdata->downsampleFactor;
}

CK_DLL_MFUN(bitcrusher_getDownsampleFactor)
{
    BitcrusherData * bcdata = (BitcrusherData *) OBJ_MEMBER_INT(SELF, bitcrusher_data_offset);
    RETURN->v_int = bcdata->downsampleFactor;
}


