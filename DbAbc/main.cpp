/*---------------------------------------------------------------------------- 
 DbAbc is a abc notation parser for chuck that produces MidiMsgs
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

/* ---------------------------------------------------------------------------*/
CK_DLL_CTOR( abcMsg_ctor );
CK_DLL_DTOR( abcMsg_dtor );
static t_CKINT abcMsg_when_offset;
static t_CKINT abcMsg_status_offset; 
static t_CKINT abcMsg_meta_offset; 
static t_CKINT abcMsg_data1_offset; // can't do arrays yet in chuck plugins
static t_CKINT abcMsg_data2_offset;
static t_CKINT abcMsg_data3_offset;
static t_CKINT abcMsg_data4_offset;
static t_CKINT abcMsg_datastr_offset;

/* ---------------------------------------------------------------------------*/
CK_DLL_CTOR( abc_ctor );
CK_DLL_DTOR( abc_dtor );
CK_DLL_MFUN( abc_configure );
CK_DLL_MFUN( abc_open );
CK_DLL_MFUN( abc_close );
CK_DLL_MFUN( abc_read );
CK_DLL_MFUN( abc_numTracks );
CK_DLL_MFUN( abc_setBPM );
CK_DLL_MFUN( abc_getBPM );
CK_DLL_MFUN( abc_rewind );
static t_CKINT abc_data_offset = 0; // offset to instance of DbAbc

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbAbc)
{
    QUERY->setname(QUERY, "DbAbc");

    /* ------------------------------------------------------------- */
    QUERY->begin_class(QUERY, "AbcMsg", "Object");
    QUERY->add_ctor(QUERY, abcMsg_ctor); // unused atm (needed if we get array creation)
    QUERY->add_dtor(QUERY, abcMsg_dtor); // unused atm

    abcMsg_when_offset = QUERY->add_mvar(QUERY, "dur", "when", false);
    abcMsg_status_offset = QUERY->add_mvar(QUERY, "int", "status", false);
    abcMsg_meta_offset = QUERY->add_mvar(QUERY, "int", "meta", false);
    abcMsg_data1_offset = QUERY->add_mvar(QUERY, "int", "data1", false);
    abcMsg_data2_offset = QUERY->add_mvar(QUERY, "int", "data2", false);
    abcMsg_data3_offset = QUERY->add_mvar(QUERY, "int", "data3", false);
    abcMsg_data4_offset = QUERY->add_mvar(QUERY, "int", "data4", false);
    abcMsg_datastr_offset = QUERY->add_mvar(QUERY, "string", "datastr", false);
    QUERY->end_class(QUERY);

    /* ------------------------------------------------------------- */
    QUERY->begin_class(QUERY, "DbAbc", "Object");

    QUERY->add_ctor(QUERY, abc_ctor);
    QUERY->add_dtor(QUERY, abc_dtor);

    QUERY->add_mfun(QUERY, abc_configure, "void", "configure");
    QUERY->add_arg(QUERY, "string[]", "argv");

    QUERY->add_mfun(QUERY, abc_open, "int", "open");
    QUERY->add_arg(QUERY, "string", "path");

    QUERY->add_mfun(QUERY, abc_close, "void", "close");

    QUERY->add_mfun(QUERY, abc_read, "int", "read");
    QUERY->add_arg(QUERY, "AbcMsg", "msg");
    QUERY->add_arg(QUERY, "int", "track");

    QUERY->add_mfun(QUERY, abc_numTracks, "int", "numTracks");

    QUERY->add_mfun(QUERY, abc_setBPM, "void", "setBPM");
    QUERY->add_arg(QUERY, "float", "bpm");

    QUERY->add_mfun(QUERY, abc_getBPM, "float", "getBPM");
    // no params

    QUERY->add_mfun(QUERY, abc_rewind, "void", "rewind");
    // no params

    // this reserves a variable in the ChucK internal class to store
    // referene to the c++ class we defined above
    abc_data_offset = QUERY->add_mvar(QUERY, "int", "@abc_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}


/* ----------------------------------------------------------------------- */

CK_DLL_CTOR(abcMsg_ctor)
{
    // only needed to construct dynamic members
    std::string s; // empty
    OBJ_MEMBER_STRING(SELF, abcMsg_datastr_offset) = (Chuck_String *)
        API->object->create_string(API, SHRED, s);

}

CK_DLL_DTOR(abcMsg_dtor)
{
    // may not need to delete_string? (there's no api)
}

/* ----------------------------------------------------------------------- */
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

CK_DLL_MFUN(abc_configure)
{
    dbAbc * c = (dbAbc *) OBJ_MEMBER_INT(SELF, abc_data_offset);
    Chuck_Object *cfg = GET_NEXT_OBJECT(ARGS);
    Chuck_Array4 *userArray = (Chuck_Array4 *) cfg; // 4 and 8 are the same on 64-bit

    std::vector<std::string> argv;
    for(int i=0;i<userArray->m_vector.size();i++)
    {
        Chuck_String **x = reinterpret_cast<Chuck_String **>(&userArray->m_vector[i]); 
        // printf("%d %s\n", i, (*x)->str().c_str());
        argv.push_back((*x)->str());
    }
    if(argv.size() > 0)
    {
        c->Configure(argv);
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
            size_t sz = mevt.data.size();
            OBJ_MEMBER_DUR(msg, abcMsg_when_offset) = mevt.dur;
            OBJ_MEMBER_INT(msg, abcMsg_status_offset) = mevt.evt;
            OBJ_MEMBER_INT(msg, abcMsg_meta_offset) = mevt.metaType;
            OBJ_MEMBER_INT(msg, abcMsg_data1_offset) = sz > 0 ? mevt.data[0] : 0;
            OBJ_MEMBER_INT(msg, abcMsg_data2_offset) = sz > 1 ? mevt.data[1] : 0;
            OBJ_MEMBER_INT(msg, abcMsg_data3_offset) = sz > 2 ? mevt.data[2] : 0;
            OBJ_MEMBER_INT(msg, abcMsg_data4_offset) = sz > 3 ? mevt.data[3] : 0;
            if(sz > 4)
            {
                Chuck_String *str = OBJ_MEMBER_STRING(msg, abcMsg_datastr_offset);
                str->set(std::string(&mevt.data[0], &mevt.data[0] + mevt.data.size()));
            }
            RETURN->v_int = (int) sz; 
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

CK_DLL_MFUN(abc_setBPM)
{
    dbAbc * c = (dbAbc *) OBJ_MEMBER_INT(SELF, abc_data_offset);
    c->SetBPM(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(abc_getBPM)
{
    dbAbc * c = (dbAbc *) OBJ_MEMBER_INT(SELF, abc_data_offset);
    RETURN->v_float = c->GetBPM();
}