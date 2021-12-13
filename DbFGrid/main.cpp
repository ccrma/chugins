/*---------------------------------------------------------------------------- 
 DbFMatrix is a note-grid (.fgrd) parser for chuck that produces Midi-like Msgs
 that represent the file contents.
 Dana Batali Dec 2021 (same license as chuck (gpl2+)).

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

#include "dbFGrid.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

/* ---------------------------------------------------------------------------*/
// XXX: we don't use midimsg cuz we wish to convey more data.  We
// can/should share code with DbAbc, but for now...
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
CK_DLL_CTOR( fgrd_ctor );
CK_DLL_DTOR( fgrd_dtor );
CK_DLL_MFUN( fgrd_open );
CK_DLL_MFUN( fgrd_close );
CK_DLL_MFUN( fgrd_rewind );
CK_DLL_MFUN( fgrd_read );
CK_DLL_MFUN( fgrd_numLayers );

static t_CKINT fgrd_data_offset = 0; // offset to instance of DbAbc

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbFGrid)
{
    QUERY->setname(QUERY, "DbFGrid");

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
    QUERY->begin_class(QUERY, "DbFGrid", "Object");

    QUERY->add_ctor(QUERY, fgrd_ctor);
    QUERY->add_dtor(QUERY, fgrd_dtor);

    // open(path)
    QUERY->add_mfun(QUERY, fgrd_open, "int", "open");
    QUERY->add_arg(QUERY, "string", "path");

    // close()
    QUERY->add_mfun(QUERY, fgrd_close, "void", "close");
    // no params

    // read()
    QUERY->add_mfun(QUERY, fgrd_read, "int", "read");
    QUERY->add_arg(QUERY, "AbcMsg", "msg");
    QUERY->add_arg(QUERY, "int", "layer");

    // rewind()
    QUERY->add_mfun(QUERY, fgrd_rewind, "void", "rewind");
    // no params

    // numLayers()
    QUERY->add_mfun(QUERY, fgrd_numLayers, "int", "numLayers");
    // no params

    // this reserves a variable in the ChucK internal class to store
    // referene to the c++ class we defined above
    fgrd_data_offset = QUERY->add_mvar(QUERY, "int", "@fgrd_data", false);
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
CK_DLL_CTOR(fgrd_ctor)
{
    OBJ_MEMBER_INT(SELF, fgrd_data_offset) = 0;
    dbFGrid *c = new dbFGrid(API->vm->get_srate(API, SHRED));
    OBJ_MEMBER_INT(SELF, fgrd_data_offset) = (t_CKINT) c;
}

CK_DLL_DTOR(fgrd_dtor)
{
    dbFGrid *c = (dbFGrid *) OBJ_MEMBER_INT(SELF, fgrd_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, fgrd_data_offset) = 0;
        c = NULL;
    }
}

CK_DLL_MFUN(fgrd_open)
{
    dbFGrid * c = (dbFGrid *) OBJ_MEMBER_INT(SELF, fgrd_data_offset);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    int err = c->Open(filename);
    RETURN->v_int = err ? 0 : 1; // following midifilein return conventions
}

CK_DLL_MFUN(fgrd_close)
{
    dbFGrid *c = (dbFGrid *) OBJ_MEMBER_INT(SELF, fgrd_data_offset);
    int err = c->Close();
    RETURN->v_int = err ? 0 : 1;
}

CK_DLL_MFUN(fgrd_numLayers)
{
    dbFGrid * c = (dbFGrid *) OBJ_MEMBER_INT(SELF, fgrd_data_offset);
    RETURN->v_int = c->GetNumLayers();
}

CK_DLL_MFUN(fgrd_read)
{
    dbFGrid *c = (dbFGrid *) OBJ_MEMBER_INT(SELF, fgrd_data_offset);

    Chuck_Object *msg = GET_NEXT_OBJECT(ARGS);
    t_CKINT layer = GET_NEXT_INT(ARGS);
    RETURN->v_int = 0; // means error or nothing left to read
    if(layer >= 0 && layer < c->GetNumLayers())
    {
        #if 0
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
        #endif
        // else nothing left to do
    }
}

CK_DLL_MFUN(fgrd_rewind)
{
    dbFGrid *c = (dbFGrid *) OBJ_MEMBER_INT(SELF, fgrd_data_offset);
    c->Rewind();
    // no return atm
}