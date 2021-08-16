
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
CK_DLL_MFUN( abcFile_read );
CK_DLL_MFUN( abcFile_numTracks );
CK_DLL_MFUN( abcFile_rewind );

t_CKINT abcFile_data_offset = 0; // required by chuck

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbAbcFile)
{
    QUERY->setname(QUERY, "DbAbcFile");
    QUERY->begin_class(QUERY, "DbAbcFile", "Object");

    QUERY->add_ctor(QUERY, abcFile_ctor);
    QUERY->add_dtor(QUERY, abcFile_dtor);

    QUERY->add_mfun(QUERY, abcFile_open, "int", "open");
    QUERY->add_arg(QUERY, "string", "path");

    QUERY->add_mfun(QUERY, abcFile_close, "void", "close");
    QUERY->add_arg(QUERY, "string", "path");

    QUERY->add_mfun(QUERY, abcFile_read, "int", "read");
    QUERY->add_arg(QUERY, "int", "track");
    QUERY->add_arg(QUERY, "MidiMsg", "msg");

    QUERY->add_mfun(QUERY, abcFile_numTracks, "int", "numTracks");
    // no params

    QUERY->add_mfun(QUERY, abcFile_rewind, "void", "rewind");
    // no params

    abcFile_data_offset = QUERY->add_mvar(QUERY, "int", "@abcFile_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(abcFile_ctor)
{
    OBJ_MEMBER_INT(SELF, abcFile_data_offset) = 0;
    dbABCFile *c = new dbABCFile();
    OBJ_MEMBER_INT(SELF, abcFile_data_offset) = (t_CKINT) c;
}

CK_DLL_DTOR(abcFile_dtor)
{
    dbABCFile *c = (dbABCFile *) OBJ_MEMBER_INT(SELF, abcFile_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, abcFile_data_offset) = 0;
        c = NULL;
    }
}

CK_DLL_MFUN(abcFile_open)
{
    dbABCFile * c = (dbABCFile *) OBJ_MEMBER_INT(SELF, abcFile_data_offset);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    int err = c->Open(filename);
    RETURN->v_int = err ? 0 : 1; // following midifilein return conventions
}

CK_DLL_MFUN(abcFile_close)
{
    dbABCFile *c = (dbABCFile *) OBJ_MEMBER_INT(SELF, abcFile_data_offset);
    int err = c->Close();
    RETURN->v_int = err ? 0 : 1;
}

CK_DLL_MFUN(abcFile_numTracks)
{
    dbABCFile * c = (dbABCFile *) OBJ_MEMBER_INT(SELF, abcFile_data_offset);
    RETURN->v_int = c->GetNumTracks();
}

CK_DLL_MFUN(abcFile_read)
{
    dbABCFile *c = (dbABCFile *) OBJ_MEMBER_INT(SELF, abcFile_data_offset);
    t_CKINT track = GET_NEXT_INT(ARGS);

    RETURN->v_int = 0; // means error, nothing to read
    if(track >= 0 && track < c->GetNumTracks())
    {
        #if 0
        std::vector<unsigned char> event;
        t_CKDUR dur = c->GetNextMidiEvent(event, track);
        if(event.size())
        {
            OBJ_MEMBER_INT(msg, MidiMsg_offset_data1) = event[0];
            OBJ_MEMBER_INT(msg, MidiMsg_offset_data2) = event.size() >= 2 ? event[1] : 0;
            OBJ_MEMBER_INT(msg, MidiMsg_offset_data3) = event.size() >= 3 ? event[2] : 0;
            OBJ_MEMBER_DUR(msg, MidiMsg_offset_when) = dur;
            RETURN->v_int = 1; 
        }
        #endif
    }
}

CK_DLL_MFUN(abcFile_rewind)
{
    dbABCFile *c = (dbABCFile *) OBJ_MEMBER_INT(SELF, abcFile_data_offset);
    c->Rewind();
    // no return atm
}