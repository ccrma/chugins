/* ----------------------------------------------------------------
 DbLiCKLFO is a ugen that implements LiCK's (github/heuermh/lick)
 LFO chugen's natively.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 U.S.A.
 ---------------------------------------------------------------------
*/
#define _USE_MATH_DEFINES

#include "chuck_dl.h"
#include "chuck_def.h"

#include "DbLiCKLFO.h"

CK_DLL_CTOR(dbll_ctor);
CK_DLL_DTOR(dbll_dtor);
CK_DLL_TICK(dbll_tick);
CK_DLL_MFUN(dbll_eval);
CK_DLL_MFUN(dbll_saw);
CK_DLL_MFUN(dbll_sine);
CK_DLL_MFUN(dbll_sqr);
CK_DLL_MFUN(dbll_tri);
CK_DLL_MFUN(dbll_hyper);
CK_DLL_MFUN(dbll_sampleHold);
CK_DLL_MFUN(dbll_smoothSampleHold);
CK_DLL_MFUN(dbll_fnoise);
CK_DLL_MFUN(dbll_modulate);
CK_DLL_MFUN(dbll_setmix);
CK_DLL_MFUN(dbll_setfreq);
CK_DLL_MFUN(dbll_setrange);
CK_DLL_MFUN(dbll_sethold);
CK_DLL_MFUN(dbll_setsmoothhold);
CK_DLL_MFUN(dbll_setnoisehold);
t_CKINT dbll_data_offset = 0;

/* -------------------------------------------------------------------- */
CK_DLL_QUERY(DbLiCKLFO)
{
    QUERY->setname(QUERY, "DbLiCKLFO");
    QUERY->begin_class(QUERY, "DbLiCKLFO", "UGen");
    QUERY->add_ctor(QUERY, dbll_ctor);
    QUERY->add_dtor(QUERY, dbll_dtor);

    QUERY->add_mfun(QUERY, dbll_setfreq, "float", "freq");
    QUERY->add_arg(QUERY, "float", "freq");

    QUERY->add_mfun(QUERY, dbll_setrange, "void", "setrange");
    QUERY->add_arg(QUERY, "float", "min");
    QUERY->add_arg(QUERY, "float", "max");

    QUERY->add_mfun(QUERY, dbll_modulate, "void", "modulate");
    QUERY->add_arg(QUERY, "int", "domod");

    QUERY->add_mfun(QUERY, dbll_saw, "void", "saw");
    QUERY->add_mfun(QUERY, dbll_sine, "void", "sine");
    QUERY->add_mfun(QUERY, dbll_sqr, "void", "sqr");
    QUERY->add_mfun(QUERY, dbll_tri, "void", "tri");
    QUERY->add_mfun(QUERY, dbll_hyper, "void", "hyper");
    QUERY->add_mfun(QUERY, dbll_sampleHold, "void", "sampleHold");
    QUERY->add_mfun(QUERY, dbll_smoothSampleHold, "void", "smoothSampleHold");
    QUERY->add_mfun(QUERY, dbll_fnoise, "void", "fnoise");

    QUERY->add_mfun(QUERY, dbll_setmix, "void", "setmix");
    QUERY->add_arg(QUERY, "float", "sawmix");
    QUERY->add_arg(QUERY, "float", "sinmix");
    QUERY->add_arg(QUERY, "float", "sqrmix");
    QUERY->add_arg(QUERY, "float", "trimix");
    QUERY->add_arg(QUERY, "float", "hypermix");
    QUERY->add_arg(QUERY, "float", "shmix");
    QUERY->add_arg(QUERY, "float", "sshmix");
    QUERY->add_arg(QUERY, "float", "fnoisemix");

    QUERY->add_mfun(QUERY, dbll_sethold, "void", "sethold");
    QUERY->add_arg(QUERY, "dur", "hold");

    QUERY->add_mfun(QUERY, dbll_setsmoothhold, "void", "setsmoothhold");
    QUERY->add_arg(QUERY, "dur", "hold");
    QUERY->add_arg(QUERY, "dur", "smoothhold");

    QUERY->add_mfun(QUERY, dbll_setnoisehold, "void", "setnoisehold");
    QUERY->add_arg(QUERY, "dur", "hold");

    // eval can be used instead of tick when audiorate evaluation
    // isn't required.
    QUERY->add_mfun(QUERY, dbll_eval, "float", "eval");
    QUERY->add_arg(QUERY, "float", "phase");

    QUERY->add_ugen_func(QUERY, dbll_tick, NULL, 1, 1);

    dbll_data_offset = QUERY->add_mvar(QUERY, "int", "@dbLiCKdistort_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(dbll_ctor)
{
    OBJ_MEMBER_INT(SELF, dbll_data_offset) = 0;
    float srate = API->vm->get_srate(API, SHRED);
    DbLiCKLFO *c = new DbLiCKLFO(srate);
    OBJ_MEMBER_INT(SELF, dbll_data_offset) = (t_CKINT)c;
}

CK_DLL_DTOR(dbll_dtor)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    if (c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, dbll_data_offset) = 0;
        c = NULL;
    }
}

CK_DLL_TICK(dbll_tick)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    *out = c->Tick(in);
    return TRUE;
}

CK_DLL_MFUN(dbll_eval)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    float phase = GET_NEXT_FLOAT(ARGS); //
    RETURN->v_float = c->Eval(phase);
}

CK_DLL_MFUN(dbll_saw)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    c->Saw();
}

CK_DLL_MFUN(dbll_sine)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    c->Sine();
}
CK_DLL_MFUN(dbll_sqr)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    c->Sqr();
}
CK_DLL_MFUN(dbll_tri)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    c->Tri();
}
CK_DLL_MFUN(dbll_hyper)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    c->Hyper();
}
CK_DLL_MFUN(dbll_sampleHold)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    c->SampleHold();
}
CK_DLL_MFUN(dbll_smoothSampleHold)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    c->SmoothSampleHold();
}
CK_DLL_MFUN(dbll_fnoise)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    c->FilteredNoise();
}
CK_DLL_MFUN(dbll_modulate)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    c->Modulate(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(dbll_setmix)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    float sawMix = GET_NEXT_FLOAT(ARGS);
    float sinMix = GET_NEXT_FLOAT(ARGS);
    float sqrMix = GET_NEXT_FLOAT(ARGS);
    float triMix = GET_NEXT_FLOAT(ARGS);
    float hypMix = GET_NEXT_FLOAT(ARGS);
    float shMix = GET_NEXT_FLOAT(ARGS);
    float sshMix = GET_NEXT_FLOAT(ARGS);
    float modNoiseMix = GET_NEXT_FLOAT(ARGS);
    c->Mix(sawMix, sinMix, sqrMix, triMix, hypMix, shMix, sshMix, modNoiseMix);
}

CK_DLL_MFUN(dbll_setfreq)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    float freq = GET_NEXT_FLOAT(ARGS); //
    c->SetFreq(freq);
    RETURN->v_float = freq;
}

CK_DLL_MFUN(dbll_setrange)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    float min = GET_NEXT_FLOAT(ARGS); //
    float max = GET_NEXT_FLOAT(ARGS); //
    c->SetOutputRange(min, max);
}

CK_DLL_MFUN(dbll_sethold)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    float hold = GET_NEXT_DUR(ARGS); //
    c->SetHold(hold);
}

CK_DLL_MFUN(dbll_setnoisehold)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    float hold = GET_NEXT_DUR(ARGS); //
    c->SetNoiseHold(hold);
}

CK_DLL_MFUN(dbll_setsmoothhold)
{
    DbLiCKLFO *c = (DbLiCKLFO *)OBJ_MEMBER_INT(SELF, dbll_data_offset);
    float hold = GET_NEXT_DUR(ARGS);       //
    float interphold = GET_NEXT_DUR(ARGS); //
    c->SetSmoothHold(hold, interphold);
}
