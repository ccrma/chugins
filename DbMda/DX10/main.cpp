/* ----------------------------------------------------------------
 The DbMdaDX10 chugin is a port of Paul Keller's open-source
 vst plugins here: http://mda.smartelectronix.com/ and
 here: https://sourceforge.net/projects/mda-vst

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

#include "mdaDX10Processor.h"

CK_DLL_CTOR( dbld_ctor );
CK_DLL_DTOR( dbld_dtor );

CK_DLL_MFUN( dbld_printPresets );
CK_DLL_MFUN( dbld_selectPreset );
CK_DLL_MFUN( dbld_getNumPresets );

CK_DLL_MFUN( dbld_printParams );
CK_DLL_MFUN( dbld_setParam );
CK_DLL_MFUN( dbld_getParam );

CK_DLL_MFUN( dbld_noteOn );
CK_DLL_MFUN( dbld_noteOff );

CK_DLL_MFUN( dbld_midiEvent );

CK_DLL_TICK( dbld_tick );

t_CKINT dbld_data_offset = 0;

/* -------------------------------------------------------------------- */

CK_DLL_QUERY(DbMdaDX10)
{
    QUERY->setname(QUERY, "DbMdaDX10");
    QUERY->begin_class(QUERY, "DbMdaDX10", "UGen");
    QUERY->add_ctor(QUERY, dbld_ctor);
    QUERY->add_dtor(QUERY, dbld_dtor);

    // presets -------------------------------------------
    QUERY->add_mfun(QUERY, dbld_printPresets, "void", "printPresets");
    // no args

    QUERY->add_mfun(QUERY, dbld_getNumPresets, "int", "getNumPresets");
    // no args

    QUERY->add_mfun(QUERY, dbld_selectPreset, "void", "selectPreset");
    QUERY->add_arg(QUERY, "int", "id");

    // params ---------------------------------------------
    QUERY->add_mfun(QUERY, dbld_printParams, "void", "printParams");

    QUERY->add_mfun(QUERY, dbld_setParam, "void", "setParam");
    QUERY->add_arg(QUERY, "int", "id");
    QUERY->add_arg(QUERY, "float", "value");

    QUERY->add_mfun(QUERY, dbld_getParam, "float", "getParam");
    QUERY->add_arg(QUERY, "int", "id");

    // midi ----------------------------------------------------
    QUERY->add_mfun(QUERY, dbld_noteOn, "int", "noteOn");
    QUERY->add_arg(QUERY, "int", "noteNumber");
    QUERY->add_arg(QUERY, "float", "velocity");

    QUERY->add_mfun(QUERY, dbld_noteOff, "int", "noteOff");
    QUERY->add_arg(QUERY, "int", "noteNumber");
    QUERY->add_arg(QUERY, "float", "velocity");

    QUERY->add_mfun(QUERY, dbld_midiEvent, "void", "midiEvent");
    QUERY->add_arg(QUERY, "MidiMsg", "msg");

    // mono tick --------------------------------------------------
    // vs: add_ugen_funcf
    QUERY->add_ugen_func(QUERY, dbld_tick, NULL, 0, 1);

    dbld_data_offset = QUERY->add_mvar(QUERY, "int", "@dbld_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(dbld_ctor)
{
    OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
    float srate = API->vm->get_srate(API, SHRED);
    DX10Processor *c = new DX10Processor(srate);
    OBJ_MEMBER_INT(SELF, dbld_data_offset) = (t_CKINT) c;
}

CK_DLL_DTOR(dbld_dtor)
{
    DX10Processor *c = (DX10Processor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
    }
}

CK_DLL_MFUN( dbld_printPresets )
{
    DX10Processor *c = (DX10Processor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    c->printPresets();
}

CK_DLL_MFUN( dbld_getNumPresets )
{
    DX10Processor *c = (DX10Processor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    RETURN->v_int = c->getNumPresets();
}

CK_DLL_MFUN( dbld_selectPreset )
{
    DX10Processor *c = (DX10Processor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int i = GET_NEXT_INT(ARGS); // 
    c->selectPreset(i);
}

CK_DLL_MFUN( dbld_printParams )
{
    DX10Processor *c = (DX10Processor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    c->printParams();
}

CK_DLL_MFUN( dbld_setParam )
{
    DX10Processor *c = (DX10Processor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int i = GET_NEXT_INT(ARGS); // 
    float f = GET_NEXT_FLOAT(ARGS);
    c->setParamValue(i, f);
}

CK_DLL_MFUN( dbld_getParam )
{
    t_CKINT index = GET_NEXT_INT(ARGS);
    DX10Processor *c = (DX10Processor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    RETURN->v_float = c->getParamValue(index);
}

CK_DLL_MFUN( dbld_noteOn )
{
    t_CKINT note = GET_NEXT_INT(ARGS);
    t_CKFLOAT vel = GET_NEXT_FLOAT(ARGS);
    DX10Processor *c = (DX10Processor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    c->addNoteOn(note, vel);
}

CK_DLL_MFUN( dbld_noteOff )
{
    t_CKINT note = GET_NEXT_INT(ARGS);
    t_CKFLOAT vel = GET_NEXT_FLOAT(ARGS);
    DX10Processor *c = (DX10Processor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    c->addNoteOff(note, vel);
}

CK_DLL_MFUN( dbld_midiEvent )
{
    DX10Processor *c = (DX10Processor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    Chuck_Object *msg = GET_NEXT_OBJECT(ARGS);

    /* object mvar offsets for MidiMsg not exported by chuck
     * so we'll hard code values.  Here we assume we're in
     * 64-bit land.
     */
    int data1 = OBJ_MEMBER_INT(msg, 8); // MidiMsg_offset_data1
    int data2 = OBJ_MEMBER_INT(msg, 16); // MidiMsg_offset_data2
    int data3 = OBJ_MEMBER_INT(msg, 24); // MidiMsg_offset_data3
    t_CKDUR when = OBJ_MEMBER_DUR(msg, 32); // MidiMsg_offset_when);

    c->addMidiEvent(data1, data2, data3);
}

CK_DLL_TICK(dbld_tick)
{
    DX10Processor *c = (DX10Processor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    c->processMono(&in, out, 1);
    return TRUE;
}