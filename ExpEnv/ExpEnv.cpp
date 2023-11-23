/*----------------------------------------------------------------------------
 ChucK Simple Decaying Exponential Envelope Unit Generator

 Simple single time constant exponential decay, applied to any
 signal passed through it.  Obeys:  value, T60, radius, and keyOn
 Especially useful for Modal synthesis.  One of these for each mode:
 SinOsc s => ExpEnv e => dac;

 Also useful for noise excitation pulse, using it like:
 Noise n => ExpEnv e => dac;

 Copyright (c) 2015 Perry R. Cook.  All rights reserved.
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 U.S.A.
 -----------------------------------------------------------------------------*/

#include "chugin.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>


CK_DLL_CTOR(expenv_ctor);
CK_DLL_DTOR(expenv_dtor);

CK_DLL_TICK(expenv_tick);

CK_DLL_MFUN(expenv_setValue);
CK_DLL_MFUN(expenv_getValue);

CK_DLL_MFUN(expenv_setRadius);
CK_DLL_MFUN(expenv_getRadius);

CK_DLL_MFUN(expenv_setT60);
CK_DLL_MFUN(expenv_getT60);

CK_DLL_MFUN(expenv_keyOn);

t_CKINT expenv_data_offset = 0;

class ExpEnv	{
public:
    
    ExpEnv(float sr) {
        srate = sr;
	radius = 0.999;
    }

    SAMPLE tick()			{
	value *= radius;
        return value;
    }
    
    t_CKFLOAT setValue(t_CKFLOAT f)     {
        value = f;
        return value;
    }
    
    t_CKFLOAT getValue() { return value; }
    
    t_CKFLOAT setRadius(t_CKFLOAT f)     {
        radius = f;
	if (radius < 1.0) {
	    T60 = (t_CKDUR) -3.0/log10(radius)/srate; // check this someday
	}
	else T60 = 0.0;
        return radius;
    }
    
    t_CKFLOAT getRadius() { return radius; }
    
    t_CKDUR setT60(t_CKDUR samps)     {
        T60 = samps;
        if (samps > 0) {
            radius = pow(10.0,-3.0/samps);
	}
	else radius = 1.0;
        return T60;			
    }

    t_CKDUR getT60() { return T60; } 

    t_CKINT keyOn()  { 
	value = 1.0;
	return 1;
    }
    
private:
    
    t_CKFLOAT srate;
    t_CKFLOAT value;
    t_CKFLOAT radius;
    t_CKDUR T60;
};

CK_DLL_QUERY(ExpEnv)
{
    QUERY->setname(QUERY, "ExpEnv");
    
    QUERY->begin_class(QUERY, "ExpEnv", "UGen");
    
    QUERY->add_ctor(QUERY, expenv_ctor);
    QUERY->add_dtor(QUERY, expenv_dtor);

    QUERY->doc_class(QUERY,  "ChucK Simple Decaying Exponential Envelope UGen\n\n"
                     "Simple single time constant exponential decay, applied to any "
                     "signal passed through it.  Obeys:  value, T60, radius, and keyOn "
                     "Especially useful for Modal synthesis.  One of these for each mode:\n"
                     "SinOsc s => ExpEnv e => dac;\n\n"

                     "Also useful for noise excitation pulse, using it like:\n"
                     "Noise n => ExpEnv e => dac;");
    QUERY->add_ex(QUERY, "effects/ExpEnv.ck");
    QUERY->add_ex(QUERY, "effects/ExpEnv-modes.ck");
    
    QUERY->add_ugen_func(QUERY, expenv_tick, NULL, 1, 1);
    
    QUERY->add_mfun(QUERY, expenv_setValue, "float", "value");
    QUERY->add_arg(QUERY, "float", "arg");
    
    QUERY->add_mfun(QUERY, expenv_getValue, "float", "value");
    
    QUERY->add_mfun(QUERY, expenv_setRadius, "float", "radius");
    QUERY->add_arg(QUERY, "float", "arg");
    
    QUERY->add_mfun(QUERY, expenv_getRadius, "float", "radius");
    
    QUERY->add_mfun(QUERY, expenv_setT60, "dur", "T60");
    QUERY->add_arg(QUERY, "dur", "arg");
    QUERY->doc_func(QUERY, "Set T60 delay time (time for sounds to decay by 60dB");

    QUERY->add_mfun(QUERY, expenv_getT60, "dur", "T60");
    QUERY->doc_func(QUERY, "Get T60 delay time (time for sounds to decay by 60dB");
    
    QUERY->add_mfun(QUERY, expenv_keyOn, "int", "keyOn");
    QUERY->add_arg(QUERY, "int", "arg");
    
    expenv_data_offset = QUERY->add_mvar(QUERY, "int", "@expenv_data", false);
    
    QUERY->end_class(QUERY);

    return TRUE;
}


CK_DLL_CTOR(expenv_ctor)
{
    OBJ_MEMBER_INT(SELF, expenv_data_offset) = 0;
    
    ExpEnv * c = new ExpEnv(API->vm->srate(VM));
    
    OBJ_MEMBER_INT(SELF, expenv_data_offset) = (t_CKINT) c;
}

CK_DLL_DTOR(expenv_dtor)
{
    ExpEnv * c = (ExpEnv *) OBJ_MEMBER_INT(SELF, expenv_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, expenv_data_offset) = 0;
        c = NULL;
    }
}

CK_DLL_TICK(expenv_tick)
{
    ExpEnv * c = (ExpEnv *) OBJ_MEMBER_INT(SELF, expenv_data_offset);
    
    //  if(c)    // if it ain’t, why are we even here?

    *out = in * c->tick();

    return TRUE;
}

CK_DLL_MFUN(expenv_setValue)
{
    ExpEnv * c = (ExpEnv *) OBJ_MEMBER_INT(SELF, expenv_data_offset);
    // TODO: sanity check
    RETURN->v_float = c->setValue(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(expenv_getValue)
{
    ExpEnv * c = (ExpEnv *) OBJ_MEMBER_INT(SELF, expenv_data_offset);
    RETURN->v_float = c->getValue();
}

CK_DLL_MFUN(expenv_setRadius)
{
    ExpEnv * c = (ExpEnv *) OBJ_MEMBER_INT(SELF, expenv_data_offset);
    // TODO: sanity check
    RETURN->v_float = c->setRadius(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(expenv_getRadius)
{
    ExpEnv * c = (ExpEnv *) OBJ_MEMBER_INT(SELF, expenv_data_offset);
    RETURN->v_float = c->getRadius();
}

CK_DLL_MFUN(expenv_setT60)
{
    ExpEnv * c = (ExpEnv *) OBJ_MEMBER_INT(SELF, expenv_data_offset);
    // TODO: sanity check
    RETURN->v_float = c->setT60(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(expenv_getT60)
{
    ExpEnv * c = (ExpEnv *) OBJ_MEMBER_INT(SELF, expenv_data_offset);
    RETURN->v_float = c->getT60();
}

CK_DLL_MFUN(expenv_keyOn)
{
    ExpEnv * c = (ExpEnv *) OBJ_MEMBER_INT(SELF, expenv_data_offset);
    RETURN->v_int = c->keyOn();
}
