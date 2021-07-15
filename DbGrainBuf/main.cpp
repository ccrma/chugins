
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

/* our parameters ---
 
    connect trigger as input 
    -or-
    float triggerFreq, default 10
    float triggerRange, default 0 (adds "dust")

    float grainPeriod, default .1 (seconds)
    float grainPeriodVariance: default 0
    float grainRate: default 1

    float grainPhaseStart: 0
    float grainPhaseStop: 1
    float grainPhaseRate: 1
    float grainPhaseWobble: 0
 */

CK_DLL_CTOR(gbuf_ctor);
CK_DLL_DTOR(gbuf_dtor);
CK_DLL_TICK(gbuf_tick);

/* methods to perform standard sndbuf behavior --- */
CK_DLL_MFUN(gbuf_ctrl_debug);
CK_DLL_MFUN(gbuf_ctrl_read);
CK_DLL_MFUN(gbuf_cget_filelen);
CK_DLL_MFUN(gbuf_cget_nchan);
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
CK_DLL_MFUN(gbuf_ctrl_triggerFreq);
CK_DLL_MFUN(gbuf_cget_triggerFreq);
CK_DLL_MFUN(gbuf_ctrl_triggerRange);

CK_DLL_MFUN(gbuf_ctrl_grainWindow);
CK_DLL_MFUN(gbuf_ctrl_grainPeriod);
CK_DLL_MFUN(gbuf_cget_grainPeriod);
CK_DLL_MFUN(gbuf_ctrl_grainPeriodVariance);
CK_DLL_MFUN(gbuf_ctrl_grainRate);

CK_DLL_MFUN(gbuf_ctrl_grainPhaseStart); //start==stop means constant grain pos
CK_DLL_MFUN(gbuf_ctrl_grainPhaseStop);
CK_DLL_MFUN(gbuf_ctrl_grainPhaseStartSec); 
CK_DLL_MFUN(gbuf_ctrl_grainPhaseStopSec);
CK_DLL_MFUN(gbuf_ctrl_grainPhaseRate);
CK_DLL_MFUN(gbuf_ctrl_grainPhaseWobble);

t_CKINT gbuf_data_offset = 0; // required by chuck

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbGrainBuf)
{
    QUERY->setname(QUERY, "DbGrainBuf");
    QUERY->begin_class(QUERY, "DbGrainBuf", "UGen");

    QUERY->add_ctor(QUERY, gbuf_ctor);
    QUERY->add_dtor(QUERY, gbuf_dtor);
    QUERY->add_ugen_func(QUERY, gbuf_tick, NULL, 1, 1);

    QUERY->add_mfun(QUERY, gbuf_ctrl_debug, "int", "debug");
    QUERY->add_arg(QUERY, "int", "debug");

    QUERY->add_mfun(QUERY, gbuf_ctrl_read, "string", "read");
    QUERY->add_arg(QUERY, "string", "read");

    QUERY->add_mfun(QUERY, gbuf_ctrl_grainWindow, "string", "grainWindow");
    QUERY->add_arg(QUERY, "string", "grainWindow");

    // query of loaded file duration returns seconds
    QUERY->add_mfun(QUERY, gbuf_cget_filelen, "float", "filelen");
    QUERY->add_mfun(QUERY, gbuf_cget_nchan, "int", "nchan");

    // bypassMode pos
    QUERY->add_mfun(QUERY, gbuf_ctrl_pos, "int", "pos");
    QUERY->add_arg(QUERY, "int", "pos"); 
    QUERY->add_mfun(QUERY, gbuf_cget_pos, "int", "pos");

    // bypassMode phase
    QUERY->add_mfun(QUERY, gbuf_ctrl_phase, "float", "phase");
    QUERY->add_arg(QUERY, "float", "phase"); 
    QUERY->add_mfun(QUERY, gbuf_cget_phase, "float", "phase");

    // bypassMode loop
    QUERY->add_mfun(QUERY, gbuf_ctrl_loop, "int", "loop");
    QUERY->add_arg(QUERY,  "int", "loop" );
    QUERY->add_mfun(QUERY, gbuf_cget_loop, "int", "loop");

    // bypassMode rate
    QUERY->add_mfun(QUERY, gbuf_ctrl_rate, "float", "rate");
    QUERY->add_arg(QUERY, "float", "rate" );
    QUERY->add_mfun(QUERY, gbuf_cget_rate, "float", "rate");

    QUERY->add_mfun(QUERY, gbuf_ctrl_maxfilt, "int", "maxfilt");
    QUERY->add_arg(QUERY,  "int", "maxfilt" );
    QUERY->add_mfun(QUERY, gbuf_cget_maxfilt, "int", "maxfilt");

    QUERY->add_mfun(QUERY, gbuf_ctrl_bypass, "int", "bypass");
    QUERY->add_arg(QUERY,  "int", "bypass" );
    QUERY->add_mfun(QUERY, gbuf_cget_bypass, "int", "bypass");

    /* - trigger ------------------------------------------------------ */
    QUERY->add_mfun(QUERY, gbuf_ctrl_triggerFreq, "float", "triggerFreq");
    QUERY->add_arg(QUERY, "float", "triggerFreq" );
    QUERY->add_mfun(QUERY, gbuf_cget_triggerFreq, "float", "triggerFreq");

    QUERY->add_mfun(QUERY, gbuf_ctrl_triggerRange, "float", "triggerRange");
    QUERY->add_arg(QUERY, "float", "triggerRange" );

    /* grain ------------------------------------------------------------ */
    QUERY->add_mfun(QUERY, gbuf_ctrl_grainPeriod, "float", "grainPeriod"); // seconds
    QUERY->add_arg(QUERY, "float", "grainPeriod" );
    QUERY->add_mfun(QUERY, gbuf_cget_grainPeriod, "float", "grainPeriod");

    QUERY->add_mfun(QUERY, gbuf_ctrl_grainPeriodVariance, "float", "grainRand");
    QUERY->add_arg(QUERY, "float", "grainRand" );

    QUERY->add_mfun(QUERY, gbuf_ctrl_grainRate, "float", "grainRate");
    QUERY->add_arg(QUERY, "float", "grainRate" );

    /* phasor ------------------------------------------------------------ */
    QUERY->add_mfun(QUERY, gbuf_ctrl_grainPhaseStart, "float", "phasorStart");
    QUERY->add_arg(QUERY, "float", "phasorStart" );

    QUERY->add_mfun(QUERY, gbuf_ctrl_grainPhaseStartSec, "float", "phasorStartSec");
    QUERY->add_arg(QUERY, "float", "phasorStartSec" );

    QUERY->add_mfun(QUERY, gbuf_ctrl_grainPhaseStop, "float", "phasorStop");
    QUERY->add_arg(QUERY, "float", "phasorStop" );

    QUERY->add_mfun(QUERY, gbuf_ctrl_grainPhaseStopSec, "float", "phasorStopSec");
    QUERY->add_arg(QUERY, "float", "phasorStopSec" );

    QUERY->add_mfun(QUERY, gbuf_ctrl_grainPhaseRate, "float", "phasorRate");
    QUERY->add_arg(QUERY, "float", "phasorRate" );

    QUERY->add_mfun(QUERY, gbuf_ctrl_grainPhaseWobble, "float", "phasorWobble");
    QUERY->add_arg(QUERY, "float", "phasorWobble" );

    // phasorWobbleFreq

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

CK_DLL_MFUN(gbuf_ctrl_debug)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_int = c->Debug(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(gbuf_ctrl_read)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    c->Read(filename);
    // no return atm
}

CK_DLL_MFUN(gbuf_ctrl_grainWindow)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    std::string winfilter = GET_NEXT_STRING_SAFE(ARGS);
    c->GrainWindow(winfilter);
    // no return atm
}

CK_DLL_MFUN(gbuf_cget_filelen)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->GetFileDur();
}

CK_DLL_MFUN(gbuf_cget_nchan)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_int = c->GetNChan();
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

CK_DLL_MFUN(gbuf_ctrl_triggerFreq)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetTriggerFreq(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_cget_triggerFreq)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->GetTriggerFreq();
}

CK_DLL_MFUN(gbuf_ctrl_triggerRange)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetTriggerRange(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_ctrl_grainPeriod)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPeriod(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_cget_grainPeriod)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->GetGrainPeriod();
}

CK_DLL_MFUN(gbuf_ctrl_grainPeriodVariance)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPeriodVariance(GET_NEXT_FLOAT(ARGS));
}
CK_DLL_MFUN(gbuf_ctrl_grainRate)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainRate(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_ctrl_grainPhaseStart)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPhaseStart(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_ctrl_grainPhaseStop)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPhaseStop(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_ctrl_grainPhaseStartSec)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPhaseStartSec(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_ctrl_grainPhaseStopSec)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPhaseStopSec(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_ctrl_grainPhaseRate)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPhaseRate(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_ctrl_grainPhaseWobble)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPhaseWobble(GET_NEXT_FLOAT(ARGS));
}
