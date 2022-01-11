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
// XXX: we don't use MidiMsg or AbcMsg cuz we wish to convey alt data.  
CK_DLL_CTOR( fgrdMsg_ctor );
CK_DLL_DTOR( fgrdMsg_dtor );
static t_CKINT fgrdMsg_type_offset; 
static t_CKINT fgrdMsg_layer_offset; 
static t_CKINT fgrdMsg_note_offset;  // also used for CC "MPE"
static t_CKINT fgrdMsg_value_offset; // velocity, CC, waittime
static t_CKINT fgrdMsg_id_offset; 
static t_CKINT fgrdMsg_chan_offset; 
static t_CKINT fgrdMsg_name_offset;  // for custom MPE names

/* ---------------------------------------------------------------------------*/
CK_DLL_CTOR( fgrd_ctor );
CK_DLL_DTOR( fgrd_dtor );

CK_DLL_MFUN( fgrd_setVerbosity );
CK_DLL_MFUN( fgrd_open );
CK_DLL_MFUN( fgrd_rewind );
CK_DLL_MFUN( fgrd_rewindSection );
CK_DLL_MFUN( fgrd_read );
CK_DLL_MFUN( fgrd_numLayers );
CK_DLL_MFUN( fgrd_beatSize );
CK_DLL_MFUN( fgrd_barSize );

static t_CKINT fgrd_data_offset = 0; // offset to instance of DbAbc

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbFGrid)
{
    QUERY->setname(QUERY, "DbFGrid");

    /* ------------------------------------------------------------- */
    QUERY->begin_class(QUERY, "FGridMsg", "Object");
    QUERY->add_ctor(QUERY, fgrdMsg_ctor); // unused atm (needed if we get array creation)
    QUERY->add_dtor(QUERY, fgrdMsg_dtor); // unused atm
    fgrdMsg_type_offset = QUERY->add_mvar(QUERY, "int", "type", false);
    fgrdMsg_layer_offset = QUERY->add_mvar(QUERY, "int", "layer", false);
    fgrdMsg_note_offset = QUERY->add_mvar(QUERY, "float", "note", false);
    fgrdMsg_value_offset = QUERY->add_mvar(QUERY, "float", "value", false);
    fgrdMsg_id_offset = QUERY->add_mvar(QUERY, "int", "id", false);
    fgrdMsg_chan_offset = QUERY->add_mvar(QUERY, "int", "chan", false);
    fgrdMsg_name_offset = QUERY->add_mvar(QUERY, "string", "name", false);
    QUERY->end_class(QUERY);

    /* ------------------------------------------------------------- */
    QUERY->begin_class(QUERY, "DbFGrid", "Object");
    QUERY->add_ctor(QUERY, fgrd_ctor);
    QUERY->add_dtor(QUERY, fgrd_dtor);

    // setVerbosity(1)
    QUERY->add_mfun(QUERY, fgrd_setVerbosity, "void", "setVerbosity");
    QUERY->add_arg(QUERY, "int", "level");

    // open(path)
    QUERY->add_mfun(QUERY, fgrd_open, "int", "open");
    QUERY->add_arg(QUERY, "string", "path");

    // read()
    QUERY->add_mfun(QUERY, fgrd_read, "int", "read");
    QUERY->add_arg(QUERY, "FGridMsg", "msg");
    QUERY->add_arg(QUERY, "int", "soloLayer"); // set to -1 to read all layers

    // rewind()
    QUERY->add_mfun(QUERY, fgrd_rewind, "void", "rewind");
    // no params

    // rewindSection()
    QUERY->add_mfun(QUERY, fgrd_rewindSection, "void", "rewindSection");
    QUERY->add_arg(QUERY, "int", "sectionIndex");

    // numLayers()
    QUERY->add_mfun(QUERY, fgrd_numLayers, "int", "numLayers");
    // no params

    // beatUnit() eg .25 is 1/4 note
    QUERY->add_mfun(QUERY, fgrd_beatSize, "int", "beatSize"); 

    QUERY->add_mfun(QUERY, fgrd_barSize, "int", "barSize"); 

    // this reserves a variable in the ChucK internal class to store
    // referene to the c++ class we defined above
    fgrd_data_offset = QUERY->add_mvar(QUERY, "int", "@fgrd_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

/* ----------------------------------------------------------------------- */

CK_DLL_CTOR(fgrdMsg_ctor)
{
    // only needed to construct dynamic members
    std::string s; // empty
    OBJ_MEMBER_STRING(SELF, fgrdMsg_name_offset) = (Chuck_String *)
        API->object->create_string(API, SHRED, s);
}

CK_DLL_DTOR(fgrdMsg_dtor)
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

CK_DLL_MFUN(fgrd_setVerbosity)
{
    dbFGrid * c = (dbFGrid *) OBJ_MEMBER_INT(SELF, fgrd_data_offset);
    int verbosity = GET_NEXT_INT(ARGS);
    c->SetVerbosity(verbosity);
}

CK_DLL_MFUN(fgrd_open)
{
    dbFGrid * c = (dbFGrid *) OBJ_MEMBER_INT(SELF, fgrd_data_offset);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    int err = c->Open(filename);
    RETURN->v_int = err ? 0 : 1; // following midifilein return conventions
}

CK_DLL_MFUN(fgrd_numLayers)
{
    dbFGrid * c = (dbFGrid *) OBJ_MEMBER_INT(SELF, fgrd_data_offset);
    RETURN->v_int = c->GetNumLayers();
}

CK_DLL_MFUN(fgrd_barSize)
{
    dbFGrid * c = (dbFGrid *) OBJ_MEMBER_INT(SELF, fgrd_data_offset);
    RETURN->v_int = c->GetBarSize(); // from file signature
} 

CK_DLL_MFUN(fgrd_beatSize)
{
    dbFGrid * c = (dbFGrid *) OBJ_MEMBER_INT(SELF, fgrd_data_offset);
    RETURN->v_int = c->GetBeatSize(); // from file signature
}

CK_DLL_MFUN(fgrd_read)
{
    dbFGrid *c = (dbFGrid *) OBJ_MEMBER_INT(SELF, fgrd_data_offset);
    Chuck_Object *msg = GET_NEXT_OBJECT(ARGS);
    int soloLayer = GET_NEXT_INT(ARGS);
    dbFGrid::Event evt;
    int ret = c->Read(&evt, soloLayer);
    if(ret == 0)
    {
        OBJ_MEMBER_INT(msg, fgrdMsg_type_offset) = (int) evt.eType;
        OBJ_MEMBER_INT(msg, fgrdMsg_layer_offset) = evt.layer;
        OBJ_MEMBER_FLOAT(msg, fgrdMsg_value_offset) = evt.value;
        OBJ_MEMBER_FLOAT(msg, fgrdMsg_note_offset) = evt.note;
        OBJ_MEMBER_INT(msg, fgrdMsg_id_offset) = evt.ccID;
        OBJ_MEMBER_INT(msg, fgrdMsg_chan_offset) = evt.chan;

        Chuck_String *str = OBJ_MEMBER_STRING(msg, fgrdMsg_name_offset);
        if(evt.ccName)
            str->set(evt.ccName);
        else
            str->set("");
    }
    RETURN->v_int = ret;
}

CK_DLL_MFUN(fgrd_rewind)
{
    dbFGrid *c = (dbFGrid *) OBJ_MEMBER_INT(SELF, fgrd_data_offset);
    c->Rewind();
    // no return atm
}

CK_DLL_MFUN(fgrd_rewindSection)
{
    dbFGrid *c = (dbFGrid *) OBJ_MEMBER_INT(SELF, fgrd_data_offset);
    int sectionIndex = GET_NEXT_INT(ARGS);
    c->Rewind(sectionIndex);
    // no return atm
}