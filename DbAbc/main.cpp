/*---------------------------------------------------------------------------- DbAbc is a abc notation parser for chuck that produces MidiMsgs
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

#include "dbAbc.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>


CK_DLL_CTOR( abc_ctor );
CK_DLL_DTOR( abc_dtor );
CK_DLL_MFUN( abc_open );
CK_DLL_MFUN( abc_close );
CK_DLL_MFUN( abc_read );
CK_DLL_MFUN( abc_numTracks );
CK_DLL_MFUN( abc_rewind );

// required by chuck
t_CKINT abc_data_offset = 0;

// hack alert:
// we'd like to use standard MidiMsg for communicating with
// outside world.  To do that we need access to MidiMsg field
// offsets.  But the chugin interface doesn't expose these and
// we'd need to link against libchuck.  But we don't want to link
// the static library, rather we'd need to link the .dll and the
// .dll would need to export these symbols.  Our hack is to 
// determine these offset values by nefarious means and hard-code
// them here.  NB: these values are likely to be valid for
// all 64 bit builds. Unclear/unlikely valid for 32-bit builds.
static t_CKINT midiMsg_offset_data1 = 8;
static t_CKINT midiMsg_offset_data2 = 16;
static t_CKINT midiMsg_offset_data3 = 24;
static t_CKINT midiMsg_offset_when = 32;

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbAbc)
{
    QUERY->setname(QUERY, "DbAbc");
    QUERY->begin_class(QUERY, "DbAbc", "Object");

    QUERY->add_ctor(QUERY, abc_ctor);
    QUERY->add_dtor(QUERY, abc_dtor);

    QUERY->add_mfun(QUERY, abc_open, "int", "open");
    QUERY->add_arg(QUERY, "string", "path");

    QUERY->add_mfun(QUERY, abc_close, "void", "close");

    QUERY->add_mfun(QUERY, abc_read, "int", "read");
    QUERY->add_arg(QUERY, "MidiMsg", "msg");
    QUERY->add_arg(QUERY, "int", "track");

    QUERY->add_mfun(QUERY, abc_numTracks, "int", "numTracks");
    // no params

    QUERY->add_mfun(QUERY, abc_rewind, "void", "rewind");
    // no params

    abc_data_offset = QUERY->add_mvar(QUERY, "int", "@abc_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(abc_ctor)
{
    OBJ_MEMBER_INT(SELF, abc_data_offset) = 0;
    dbAbc *c = new dbAbc(API->vm->get_srate(API, SHRED));
    OBJ_MEMBER_INT(SELF, abc_data_offset) = (t_CKINT) c;
}

CK_DLL_DTOR(abc_dtor)
{
    dbAbc *c = (dbAbc *) OBJ_MEMBER_INT(SELF, abc_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, abc_data_offset) = 0;
        c = NULL;
    }
}

CK_DLL_MFUN(abc_open)
{
    dbAbc * c = (dbAbc *) OBJ_MEMBER_INT(SELF, abc_data_offset);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    int err = c->Open(filename);
    RETURN->v_int = err ? 0 : 1; // following midifilein return conventions
}

CK_DLL_MFUN(abc_close)
{
    dbAbc *c = (dbAbc *) OBJ_MEMBER_INT(SELF, abc_data_offset);
    int err = c->Close();
    RETURN->v_int = err ? 0 : 1;
}

CK_DLL_MFUN(abc_numTracks)
{
    dbAbc * c = (dbAbc *) OBJ_MEMBER_INT(SELF, abc_data_offset);
    RETURN->v_int = c->GetNumTracks();
}

CK_DLL_MFUN(abc_read)
{
    dbAbc *c = (dbAbc *) OBJ_MEMBER_INT(SELF, abc_data_offset);

    // MidiIn::readTrack(MidiMsg m, int track)
    Chuck_Object *msg = GET_NEXT_OBJECT(ARGS);
    t_CKINT track = GET_NEXT_INT(ARGS);
    RETURN->v_int = 0; // means error or nothing left to read
    if(track >= 0 && track < c->GetNumTracks())
    {
        MidiEvent mevt;
        int active = c->Read(track, &mevt);
        if(active)
        {
            OBJ_MEMBER_DUR(msg, midiMsg_offset_when) = mevt.dur;
            if(mevt.size <= 2)
            {
                OBJ_MEMBER_INT(msg, midiMsg_offset_data1) = mevt.evt;
                OBJ_MEMBER_INT(msg, midiMsg_offset_data2) = mevt.size >= 1 ? mevt.data.d[0] : 0;
                OBJ_MEMBER_INT(msg, midiMsg_offset_data3) = mevt.size >= 2 ? mevt.data.d[1] : 0;
            }
            else
            {
                // won't fit in MidiMsg (usually a label/annotation)
                OBJ_MEMBER_INT(msg, midiMsg_offset_data1) = mevt.evt;
                OBJ_MEMBER_INT(msg, midiMsg_offset_data2) = 0;
                OBJ_MEMBER_INT(msg, midiMsg_offset_data3) =  0;
            }
            RETURN->v_int = 1; 
        }
        // else nothing left to do
    }
}

CK_DLL_MFUN(abc_rewind)
{
    dbAbc *c = (dbAbc *) OBJ_MEMBER_INT(SELF, abc_data_offset);
    c->Rewind();
    // no return atm
}
