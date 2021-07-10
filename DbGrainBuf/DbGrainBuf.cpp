
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

#include "dbGrainBufImpl.h"

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

CK_DLL_MFUN(gbuf_ctrl_read);
CK_DLL_MFUN(gbuf_ctrl_pos);
CK_DLL_MFUN(gbuf_cget_pos);
CK_DLL_MFUN(gbuf_ctrl_loop);
CK_DLL_MFUN(gbuf_cget_loop);
CK_DLL_MFUN(gbuf_ctrl_rate);
CK_DLL_MFUN(gbuf_cget_rate);

t_CKINT gbuf_data_offset = 0; // required by chuck

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbGrainBuf)
{
    QUERY->setname(QUERY, "DbGrainBuf");
    QUERY->begin_class(QUERY, "DbGrainBuf", "UGen");

    QUERY->add_ctor(QUERY, gbuf_ctor);
    QUERY->add_dtor(QUERY, gbuf_dtor);
    QUERY->add_ugen_func(QUERY, gbuf_tick, NULL, 1, 1);

    gbuf_data_offset = QUERY->add_mvar(QUERY, "int", "@gbuf_data", false);

    QUERY->add_mfun(QUERY, gbuf_ctrl_read, "string", "read");
    QUERY->add_arg(QUERY, "string", "read");
    // no get function

    // set/get play position by framenum
    QUERY->add_mfun(QUERY, gbuf_ctrl_pos, "int", "pos");
    QUERY->add_arg(QUERY, "int", "pos"); 
    QUERY->add_mfun(QUERY, gbuf_cget_pos, "int", "pos");
    // getpos has no args

    // set/get loop
    QUERY->add_mfun(QUERY, gbuf_ctrl_loop, "int", "loop");
    QUERY->add_arg(QUERY,  "int", "loop" );
    QUERY->add_mfun(QUERY, gbuf_cget_loop, "int", "pos");
    // getloop has no args

    // set/get rate
    QUERY->add_mfun(QUERY, gbuf_ctrl_rate, "float", "rate");
    QUERY->add_arg(QUERY, "float", "rate" );
    QUERY->add_mfun(QUERY, gbuf_cget_rate, "int", "pos");
    // getrate has no args

    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(gbuf_ctor)
{
    OBJ_MEMBER_INT(SELF, gbuf_data_offset) = 0;
    dbGrainBufImpl * c = new dbGrainBufImpl(API->vm->get_srate(API, SHRED));
    OBJ_MEMBER_INT(SELF, gbuf_data_offset) = (t_CKINT) c;
}

CK_DLL_DTOR(gbuf_dtor)
{
    dbGrainBufImpl * c = (dbGrainBufImpl *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, gbuf_data_offset) = 0;
        c = NULL;
    }
}

CK_DLL_TICK(gbuf_tick)
{
    dbGrainBufImpl * c = (dbGrainBufImpl *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    *out = c->Tick(in);
    return TRUE;
}

CK_DLL_MFUN(gbuf_ctrl_read)
{
    dbGrainBufImpl * c = (dbGrainBufImpl *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    c->Read(GET_NEXT_STRING_SAFE(ARGS));
    // no return atm
}

CK_DLL_MFUN(gbuf_ctrl_loop)
{
    dbGrainBufImpl * c = (dbGrainBufImpl *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_int = c->SetLoop(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(gbuf_cget_loop)
{
    dbGrainBufImpl * c = (dbGrainBufImpl *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_int = c->GetLoop();
}