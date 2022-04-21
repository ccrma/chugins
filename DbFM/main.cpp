/* ----------------------------------------------------------------
 The DbFM chugin is a port of google's open-source fm synth.
 Set README.md for more.
 ---------------------------------------------------------------------
*/
#include "DbFM.h" /* this must preceed chuck includes */

#include "chuck_dl.h"
#include "chuck_def.h"

CK_DLL_CTOR( dbld_ctor );
CK_DLL_DTOR( dbld_dtor );

// load cartridge, 
// select preset
// enumerate presets?
// getparam, setparam

CK_DLL_MFUN( dbld_noteOn );
CK_DLL_MFUN( dbld_noteOff );
CK_DLL_MFUN( dbld_midiEvent );
CK_DLL_TICK( dbld_tick );
t_CKINT dbld_data_offset = 0;

/* -------------------------------------------------------------------- */

CK_DLL_QUERY(DbFM)
{
    QUERY->setname(QUERY, "DbFM");
    QUERY->begin_class(QUERY, "DbFM", "UGen");
    QUERY->add_ctor(QUERY, dbld_ctor);
    QUERY->add_dtor(QUERY, dbld_dtor);

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
    DbFM *x = new DbFM(srate);
    OBJ_MEMBER_INT(SELF, dbld_data_offset) = (t_CKINT) x;
}

CK_DLL_DTOR(dbld_dtor)
{
    DbFM *x = (DbFM *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    if(x)
    {
        delete x;
        OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
    }
}

CK_DLL_MFUN( dbld_noteOn )
{
    t_CKINT note = GET_NEXT_INT(ARGS);
    t_CKFLOAT vel = GET_NEXT_FLOAT(ARGS);
    DbFM *x = (DbFM *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    x->AddNoteOn(note, vel);
}

CK_DLL_MFUN( dbld_noteOff )
{
    t_CKINT note = GET_NEXT_INT(ARGS);
    t_CKFLOAT vel = GET_NEXT_FLOAT(ARGS);
    DbFM *x = (DbFM *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    x->AddNoteOff(note, vel);
}

CK_DLL_MFUN( dbld_midiEvent )
{
    DbFM *x = (DbFM *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    Chuck_Object *msg = GET_NEXT_OBJECT(ARGS);

    /* object mvar offsets for MidiMsg not exported by chuck
     * so we'll hard code values.  Here we assume we're in
     * 64-bit land.
     */
    int data1 = OBJ_MEMBER_INT(msg, 8); // MidiMsg_offset_data1
    int data2 = OBJ_MEMBER_INT(msg, 16); // MidiMsg_offset_data2
    int data3 = OBJ_MEMBER_INT(msg, 24); // MidiMsg_offset_data3
    t_CKDUR when = OBJ_MEMBER_DUR(msg, 32); // MidiMsg_offset_when);
    // when is currently ignored.
    x->AddMidiEvent(data1, data2, data3);
}

CK_DLL_TICK(dbld_tick)
{
    DbFM *x = (DbFM *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    x->GetSamples(1, out);
    return TRUE;
}