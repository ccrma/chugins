
/*----------------------------------------------------------------------------
 DbABCFile is a abc notation parser for chuck that produces MidiMsgs
 that represent the file contents.
 Dana Batali Jun 2021 (same license as chuck (gpl2+)).

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

#include "dbABCFile.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

CK_DLL_CTOR( abcFile_ctor );
CK_DLL_DTOR( abcFile_dtor );
CK_DLL_MFUN( abcFile_open );
CK_DLL_MFUN( abcFile_close );
CK_DLL_MFUN( abcFile_numTracks );
CK_DLL_MFUN( abcFile_read );
CK_DLL_MFUN( abcFile_readTrack );
CK_DLL_MFUN( abcFile_rewind );

t_CKINT abcFile_data_offset = 0; // required by chuck

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbAbcFile)
{
    QUERY->setname(QUERY, "DbAbcFile");
    QUERY->begin_class(QUERY, "DbAbcFileIn", "Object");

    QUERY->add_ctor(QUERY, abcFile_ctor);
    QUERY->add_dtor(QUERY, abcFile_dtor);

    QUERY->add_mfun(QUERY, abcFile_open, "int", "open");
    QUERY->add_arg(QUERY, "string", "path");

    QUERY->add_mfun(QUERY, abcFile_close, "void", "close");
    
    QUERY->add_mfun(QUERY, abcFile_read, "int", "read");
    QUERY->add_arg(QUERY, "MidiMsg", "msg");

    QUERY->add_mfun(QUERY, abcFile_readTrack, "int", "read");
    QUERY->add_arg(QUERY, "MidiMsg", "msg");
    QUERY->add_arg(QUERY, "int", "track");

    QUERY->add_mfun(QUERY, abcFile_numTracks, "int", "numTracks");
    QUERY->add_mfun(QUERY, abcFile_rewind, "void", "rewind");

    abcFile_data_offset = QUERY->add_mvar(QUERY, "int", "@abcFile_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(abcFile_ctor)
{
    OBJ_MEMBER_INT(SELF, abcFile_data_offset) = 0;
    dbGrainBuf * c = new dbGrainBuf(API->vm->get_srate(API, SHRED));
    OBJ_MEMBER_INT(SELF, abcFile_data_offset) = (t_CKINT) c;
}

CK_DLL_DTOR(abcFile_dtor)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, abcFile_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, abcFile_data_offset) = 0;
        c = NULL;
    }
}

CK_DLL_MFUN(abcFile_read)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, abcFile_data_offset);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    c->Read(filename);
    // no return atm
}

CK_DLL_MFUN(abcFile_numChan)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, abcFile_data_offset);
    RETURN->v_int = c->FileChan(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(abcFile_ctrl_pos)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, abcFile_data_offset);
    RETURN->v_int = c->SetPos(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(abcFile_cget_pos)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, abcFile_data_offset);
    RETURN->v_int = c->GetPos();
}

CK_DLL_MFUN(abcFile_ctrl_phase)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, abcFile_data_offset);
    RETURN->v_float = c->SetPhase(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(abcFile_cget_phase)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, abcFile_data_offset);
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
    t_CKDUR d = GET_NEXT_DUR(ARGS); // dur is measured in samples
    RETURN->v_dur = c->SetGrainPeriod(d);
}

CK_DLL_MFUN(gbuf_cget_grainPeriod)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_dur = c->GetGrainPeriod();
}

CK_DLL_MFUN(gbuf_ctrl_grainRate)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainRate(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_ctrl_grainPeriodVariance)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPeriodVariance(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_ctrl_grainPeriodVarianceFreq)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPeriodVarianceFreq(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_ctrl_grainPan)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPan(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_ctrl_grainPanRange)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPanRange(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_ctrl_grainPhaseStart)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPhaseStart(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_cget_grainPhaseStart)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->GetGrainPhaseStart();
}

CK_DLL_MFUN(gbuf_ctrl_grainPhaseStop)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPhaseStop(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(gbuf_cget_grainPhaseStop)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->GetGrainPhaseStop();
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

CK_DLL_MFUN(gbuf_ctrl_grainPhaseWobbleFreq)
{
    dbGrainBuf * c = (dbGrainBuf *) OBJ_MEMBER_INT(SELF, gbuf_data_offset);
    RETURN->v_float = c->SetGrainPhaseWobbleFreq(GET_NEXT_FLOAT(ARGS));
}