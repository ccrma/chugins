
/*----------------------------------------------------------------------------
 DbGrainBuf is a sndbuf-based granular synthesis chugin for Chuck.
 Extends Chuck's SndBuf UGen with SuperCollider's GrainBuf interface.

 Dana Batali Jun 2021 (same license as chuck and libsndbuf (gpl2+)).

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

#include "chuck_dl.h"
#include "chuck_def.h"

#include "dbGrainBuf.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

/* important parameters of SuperCollider GrainBuf

trigger - a kr or ar trigger to start a new grain. If ar, grains after 
  the start of the synth are sample accurate.
    - usually connected to Impulse.ar(10), or Dust.ar(10)

dur	of the grain (in seconds). 
    - CC-ing dur is of some value.
    - when long signal gets loud, so gainreduction is a good idea.

rate - the playback rate of the sampled sound
    - used for pitch-shifting with linear pos playback.

pos - the normalized playback position for the grain.
    - CC-ing the pos can step through the file (used for pitch-shifting)
    - Line with wobble. (Wobble of 0 with pitch-shifting gives 'vocoder'-effect,
      when trigger is impulse).
*/

/* our parameters ---

    // instead of a trigger ugen:
    dur triggerRate, default 10::ms
    float triggerRandomness, default; 0

    dur grainSize: default 10:ms

    float rate: default 1

    float pos: (same as SndBuf phaseOffset)
    int autoPosMode:  WIP

 */

CK_DLL_CTOR(gbuf_ctor);
CK_DLL_DTOR(gbuf_dtor);
CK_DLL_TICK(gbuf_tick);

/* methods to perform standard sndbuf behavior --- */
CK_DLL_MFUN(gbuf_ctrl_read);
CK_DLL_MFUN(gbuf_ctrl_pos);
CK_DLL_MFUN(gbuf_cget_pos);
CK_DLL_MFUN(gbuf_ctrl_phase);
CK_DLL_MFUN(gbuf_cget_phase);
CK_DLL_MFUN(gbuf_ctrl_loop);
CK_DLL_MFUN(gbuf_cget_loop);
CK_DLL_MFUN(gbuf_ctrl_rate);
CK_DLL_MFUN(gbuf_cget_rate);
CK_DLL_MFUN(gbuf_ctrl_maxfilt);
CK_DLL_MFUN(gbuf_cget_maxfilt);
CK_DLL_MFUN(gbuf_ctrl_bypass); // bypass grain logic
CK_DLL_MFUN(gbuf_cget_bypass);

/* extra methods to for grainbuf --- */
CK_DLL_MFUN(gbuf_ctrl_triggerRate);
CK_DLL_MFUN(gbuf_ctrl_triggerRand);
CK_DLL_MFUN(gbuf_ctrl_grainDur);
CK_DLL_MFUN(gbuf_ctrl_grainDurRand);
CK_DLL_MFUN(gbuf_ctrl_phaseMin); // min==max means constant
CK_DLL_MFUN(gbuf_ctrl_phaseMax);
CK_DLL_MFUN(gbuf_ctrl_phaseRate); // 1 means file duration
CK_DLL_MFUN(gbuf_ctrl_phaseNoise); // large values produce nocide
// we'll use rate above


t_CKINT gbuf_data_offset = 0; // required by chuck

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbGrainBuf)
{
    QUERY->setname(QUERY, "DbGrainBuf");
    QUERY->begin_class(QUERY, "DbGrainBuf", "UGen");

    QUERY->add_ctor(QUERY, gbuf_ctor);
    QUERY->add_dtor(QUERY, gbuf_dtor);
    QUERY->add_ugen_func(QUERY, gbuf_tick, NULL, 1, 1);

    QUERY->add_mfun(QUERY, gbuf_ctrl_read, "string", "read");
    QUERY->add_arg(QUERY, "string", "read");
    // no get function

    // set/get play position by framenum
    QUERY->add_mfun(QUERY, gbuf_ctrl_pos, "int", "pos");
    QUERY->add_arg(QUERY, "int", "pos"); 
    QUERY->add_mfun(QUERY, gbuf_cget_pos, "int", "pos");
    // getpos has no args

    // set/get play position by phase (pct of filesize)
    QUERY->add_mfun(QUERY, gbuf_ctrl_phase, "float", "phase");
    QUERY->add_arg(QUERY, "float", "phase"); 
    QUERY->add_mfun(QUERY, gbuf_cget_phase, "float", "phase");

    // set/get loop
    QUERY->add_mfun(QUERY, gbuf_ctrl_loop, "int", "loop");
    QUERY->add_arg(QUERY,  "int", "loop" );
    QUERY->add_mfun(QUERY, gbuf_cget_loop, "int", "loop");
    // getloop has no args

    // set/get rate
    QUERY->add_mfun(QUERY, gbuf_ctrl_rate, "float", "rate");
    QUERY->add_arg(QUERY, "float", "rate" );
    QUERY->add_mfun(QUERY, gbuf_cget_rate, "float", "rate");
    // getrate has no args

    QUERY->add_mfun(QUERY, gbuf_ctrl_maxfilt, "int", "maxfilt");
    QUERY->add_arg(QUERY,  "int", "maxfilt" );
    QUERY->add_mfun(QUERY, gbuf_cget_maxfilt, "int", "maxfilt");

    QUERY->add_mfun(QUERY, gbuf_ctrl_bypass, "int", "bypass");
    QUERY->add_arg(QUERY,  "int", "bypass" );
    QUERY->add_mfun(QUERY, gbuf_cget_bypass, "int", "bypass");

    gbuf_data_offset = QUERY->add_mvar(QUERY, "int", "@gbuf_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(gbuf_ctor)
{
    OBJ_MEMBER_INT(SELF, gbuf_data_offset) = 0;
    dbGrainBuf * c = new dbGrainBuf(API->vm->get_srate(API, SHRED));
    OBJ_MEMBER_INT(SELF, gbuf_data_offset) = (t_CKINT) c;
}

CK_DLL_DTOR(gbuf_dtor)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, gbuf_data_offset) = 0;
        c = NULL;
    }
}

CK_DLL_TICK(gbuf_tick)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    *out = c->Tick(in);
    return TRUE;
}

CK_DLL_MFUN(gbuf_ctrl_read)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    c->Read(filename);
    // no return atm
}

CK_DLL_MFUN(gbuf_ctrl_pos)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_int = c->SetPos(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(gbuf_cget_pos)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_int = c->GetPos();
}

CK_DLL_MFUN(gbuf_ctrl_phase)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetPhase(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_cget_phase)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->GetPhase();
}

CK_DLL_MFUN(gbuf_ctrl_loop)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_int = c->SetLoop(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(gbuf_cget_loop)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_int = c->GetLoop();
}

CK_DLL_MFUN(gbuf_ctrl_rate)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetRate(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_cget_rate)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->GetRate();
}

CK_DLL_MFUN(gbuf_ctrl_maxfilt)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_int = c->SetMaxFilt(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(gbuf_cget_maxfilt)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_int = c->GetMaxFilt();
}

CK_DLL_MFUN(gbuf_ctrl_bypass)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_int = c->SetBypass(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(gbuf_cget_bypass)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_int = c->GetBypass();
}

CK_DLL_MFUN(gbuf_ctrl_triggerRate)
{

}

CK_DLL_MFUN(gbuf_ctrl_triggerRand)
{

}

CK_DLL_MFUN(gbuf_ctrl_grainDur)
{

}

CK_DLL_MFUN(gbuf_ctrl_grainDurRand)
{

}

// phase is the position within the file to initialize new grains.
// it is expressed in pct of the sndbuf.  We only automate phase
// generation if we detect that explicit phase isn't provided.
CK_DLL_MFUN(gbuf_ctrl_phaseMin) // min==max means constant
{

}

CK_DLL_MFUN(gbuf_ctrl_phaseMax)
{
}

CK_DLL_MFUN(gbuf_ctrl_phaseRate) // 1 means file duration
{

}

CK_DLL_MFUN(gbuf_ctrl_phaseNoise) // large values produce random position
{
}