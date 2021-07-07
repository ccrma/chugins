/*----------------------------------------------------------------------------
 ChucK Toylike RemapOsc UGen. Maps input linear values
 between -1 and 1 onto output exponential values: 
 0, max rounding to the nearest multiple of round. 

 Useful for generating frequencies for frequency modulation.

 Typical use case:

 SubNoise a => FreqRange b => SinOsc c => dac;
 2 => c.sync; 
 // now random frequency offsets for c produce 'bubbles' around
 // c's primary frequency.

 Dana Batali Jun 2021 (MIT licensed)

 -----------------------------------------------------------------------------*/

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

CK_DLL_CTOR(remap_ctor);
CK_DLL_DTOR(remap_dtor);
CK_DLL_TICK(remap_tick);
CK_DLL_MFUN(remap_getMin);
CK_DLL_MFUN(remap_setMin);
CK_DLL_MFUN(remap_getMax);
CK_DLL_MFUN(remap_setMax);
CK_DLL_MFUN(remap_getRound);
CK_DLL_MFUN(remap_setRound);

t_CKINT remap_data_offset = 0; // required by chuck

class RemapOsc
{
private:
    t_CKFLOAT srate;
    t_CKFLOAT outMin;
    t_CKFLOAT outMax;
    t_CKFLOAT outRange;
    t_CKDUR round;
    SAMPLE lastIn;
    SAMPLE lastOut;

public:
    RemapOsc(float sr) 
    {
        this->srate = sr;
        this->lastIn = -100;
        this->round = 0;
    }

    SAMPLE tick(SAMPLE in)
    {
        if(in == this->lastIn)
            return this->lastOut;
        else
            this->lastIn = in;

        // assume src range = [-1, 1] (so we can input SubNoise)
        if(in < -1.)
            in = -1.;
        else
        if(in > 1.)
            in = 1.;

        t_CKFLOAT pct = (in + 1) / 2;
        this->lastOut = this->outMin + pct * this->outRange;
        if(this->round != 0)
        {
            float mult = this->lastOut / this->round;
            int imult = (int) mult; // truncate
            if(mult - imult > .5) 
                this->lastOut = (imult+1) * round;
            else
                this->lastOut = imult * round;
        }
        return this->lastOut;
    }
    
    t_CKFLOAT setMin(t_CKFLOAT f)
    {
        this->outMin = f;
        this->updateRange();
        return f;
    }
    t_CKFLOAT getMin() { return this->outMin; }

    t_CKFLOAT setMax(t_CKFLOAT f)
    {
        this->outMax = f;
        this->updateRange();
        return f;
    }
    t_CKFLOAT getMax() { return this->outMax; }
    
    t_CKFLOAT setRound(t_CKFLOAT f)
    {
        this->round = f;
        return f;
    }
    t_CKFLOAT getRound() { return this->round; }

    void updateRange()
    {
        this->outRange = this->outMax - this->outMin;
    }
    
};

/* ----------------------------------------------------------- */
CK_DLL_QUERY(RemapOsc)
{
    QUERY->setname(QUERY, "dbRemapOsc");
    QUERY->begin_class(QUERY, "dbRemapOsc", "UGen");
    QUERY->add_ctor(QUERY, remap_ctor);
    QUERY->add_dtor(QUERY, remap_dtor);
    QUERY->add_ugen_func(QUERY, remap_tick, NULL, 1, 1);
    /* for now we're single-channel otherwise we 
     * add_ugen_funcf and CK_DLL_TICKF
     */

    QUERY->add_mfun(QUERY, remap_setMin, "float", "min");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, remap_getMin, "float", "min");

    QUERY->add_mfun(QUERY, remap_setMax, "float", "max");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, remap_getMax, "float", "max");

    QUERY->add_mfun(QUERY, remap_setRound, "float", "round");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, remap_getRound, "float", "round");
    
    remap_data_offset = QUERY->add_mvar(QUERY, "int", "@remap_data", false);
    
    QUERY->end_class(QUERY);

    return TRUE;
}


CK_DLL_CTOR(remap_ctor)
{
    OBJ_MEMBER_INT(SELF, remap_data_offset) = 0;
    
    RemapOsc * c = new RemapOsc(API->vm->get_srate(API, SHRED));
    
    OBJ_MEMBER_INT(SELF, remap_data_offset) = (t_CKINT) c;
}

CK_DLL_DTOR(remap_dtor)
{
    RemapOsc * c = (RemapOsc *) OBJ_MEMBER_INT(SELF, remap_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, remap_data_offset) = 0;
        c = NULL;
    }
}

CK_DLL_TICK(remap_tick)
{
    RemapOsc * c = (RemapOsc *) OBJ_MEMBER_INT(SELF, remap_data_offset);
    *out = c->tick(in);
    return TRUE;
}

CK_DLL_MFUN(remap_setMin)
{
    RemapOsc * c = (RemapOsc *) OBJ_MEMBER_INT(SELF, remap_data_offset);
    // TODO: sanity check
    RETURN->v_float = c->setMin(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(remap_getMin)
{
    RemapOsc * c = (RemapOsc *) OBJ_MEMBER_INT(SELF, remap_data_offset);
    RETURN->v_float = c->getMin();
}

CK_DLL_MFUN(remap_setMax)
{
    RemapOsc * c = (RemapOsc *) OBJ_MEMBER_INT(SELF, remap_data_offset);
    // TODO: sanity check
    RETURN->v_float = c->setMax(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(remap_getMax)
{
    RemapOsc * c = (RemapOsc *) OBJ_MEMBER_INT(SELF, remap_data_offset);
    RETURN->v_float = c->getMax();
}

CK_DLL_MFUN(remap_setRound)
{
    RemapOsc * c = (RemapOsc *) OBJ_MEMBER_INT(SELF, remap_data_offset);
    // TODO: sanity check
    RETURN->v_float = c->setRound(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(remap_getRound)
{
    RemapOsc * c = (RemapOsc *) OBJ_MEMBER_INT(SELF, remap_data_offset);
    RETURN->v_float = c->getRound();
}
