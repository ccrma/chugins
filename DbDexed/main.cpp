/* ----------------------------------------------------------------
 The DbDexed chugin is a port of GPl3 dexed FM synth.
 Set README.md for more.
 ---------------------------------------------------------------------
*/
#include "DbDexed.h" /* this must preceed chuck includes */

#include "chuck_dl.h"
#include "chuck_def.h"

#include <iostream>

/* -------------------------------------------------------------------- */

CK_DLL_CTOR( dbld_ctor );
CK_DLL_DTOR( dbld_dtor );

CK_DLL_MFUN( dbld_getNumPresets );
CK_DLL_MFUN( dbld_selectPreset );
CK_DLL_MFUN( dbld_getPresetName );
CK_DLL_MFUN( dbld_printPresets );
CK_DLL_MFUN( dbld_loadCartridge );

// printParams?
// getparam, setparam

CK_DLL_MFUN( dbld_noteOn );
CK_DLL_MFUN( dbld_noteOff );
CK_DLL_MFUN( dbld_midiEvent );
CK_DLL_TICK( dbld_tick );

static t_CKINT dbld_data_offset = 0;
static t_CKINT dbld_datastr_offset = 0;

/* -------------------------------------------------------------------- */

CK_DLL_QUERY(DbDexed)
{
    QUERY->setname(QUERY, "DbDexed");
    QUERY->begin_class(QUERY, "DbDexed", "UGen");
    QUERY->add_ctor(QUERY, dbld_ctor);
    QUERY->add_dtor(QUERY, dbld_dtor);

    // presets ----------------------------------------------------
    QUERY->add_mfun(QUERY, dbld_getNumPresets, "int", "getNumPresets");
    // no args

    QUERY->add_mfun(QUERY, dbld_printPresets, "void", "printPresets");
    // no args

    QUERY->add_mfun(QUERY, dbld_selectPreset, "void", "selectPreset");
    QUERY->add_arg(QUERY, "int", "presetNumber");

    QUERY->add_mfun(QUERY, dbld_getPresetName, "string", "getPresetName");
    QUERY->add_arg(QUERY, "int", "presetNumber");

    QUERY->add_mfun(QUERY, dbld_loadCartridge, "int", "loadCartridge");
    QUERY->add_arg(QUERY, "string", "filename");

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

    // hack to return a string, might be a better way?
    dbld_datastr_offset = QUERY->add_mvar(QUERY, "string", "str", false);

    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(dbld_ctor)
{
    OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
    float srate = API->vm->get_srate(API, SHRED);
    DbDexed *x = new DbDexed(srate);
    OBJ_MEMBER_INT(SELF, dbld_data_offset) = (t_CKINT) x;

    std::string defval;
    OBJ_MEMBER_STRING(SELF, dbld_datastr_offset) = (Chuck_String *)
        API->object->create_string(API, SHRED, defval);
}

CK_DLL_DTOR(dbld_dtor)
{
    DbDexed *x = (DbDexed *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    if(x)
    {
        delete x;
        OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
    }
}

/* ------------------------------------------------------------------------ */

CK_DLL_MFUN( dbld_getNumPresets )
{
    // a cartridge always has 32 presets
    RETURN->v_int = 32;
}

CK_DLL_MFUN( dbld_getPresetName )
{
    DbDexed *x = (DbDexed *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int i = GET_NEXT_INT(ARGS); // 
    std::string nm;
    x->GetProgramName(i, nm);
    Chuck_String *ckstr = OBJ_MEMBER_STRING(SELF, dbld_datastr_offset);
    ckstr->set(nm);
    RETURN->v_string = ckstr;
}

CK_DLL_MFUN( dbld_selectPreset )
{
    DbDexed *x = (DbDexed *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int i = GET_NEXT_INT(ARGS); // 
    x->SetCurrentProgram(i);
}

CK_DLL_MFUN( dbld_printPresets )
{
    DbDexed *x = (DbDexed *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    std::vector<std::string> nms;
    x->GetProgramNames(nms);
    for(int i=0;i<nms.size();i++)
        std::cerr << i << ": " << nms[i] << "\n";
}

CK_DLL_MFUN( dbld_loadCartridge )
{
    DbDexed *x = (DbDexed *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    int err = x->LoadCartridge(filename.c_str());
    if(err != 0)
        std::cerr << "DbDexed problem opening " << filename << " " << err  << "\n";
    RETURN->v_int = err;
}

/* ------------------------------------------------------------------------ */
CK_DLL_MFUN( dbld_noteOn )
{
    t_CKINT note = GET_NEXT_INT(ARGS);
    t_CKFLOAT vel = GET_NEXT_FLOAT(ARGS);
    DbDexed *x = (DbDexed *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    x->AddNoteOn(note, vel);
}

CK_DLL_MFUN( dbld_noteOff )
{
    t_CKINT note = GET_NEXT_INT(ARGS);
    t_CKFLOAT vel = GET_NEXT_FLOAT(ARGS);
    DbDexed *x = (DbDexed *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    x->AddNoteOff(note, vel);
}

CK_DLL_MFUN( dbld_midiEvent )
{
    DbDexed *x = (DbDexed *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    Chuck_Object *msg = GET_NEXT_OBJECT(ARGS);

    /* object mvar offsets for MidiMsg not exported by chuck
     * so we'll hard code values.  Here we assume we're in
     * 64-bit land.
     */
    int data1 = OBJ_MEMBER_INT(msg, 8); // MidiMsg_offset_data1
    int data2 = OBJ_MEMBER_INT(msg, 16); // MidiMsg_offset_data2
    int data3 = OBJ_MEMBER_INT(msg, 24); // MidiMsg_offset_data3
    // t_CKDUR when = OBJ_MEMBER_DUR(msg, 32); // MidiMsg_offset_when);
    // when is currently ignored.
    x->AddMidiEvent(data1, data2, data3);
}

CK_DLL_TICK(dbld_tick)
{
    DbDexed *x = (DbDexed *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    x->GetSamples(1, out);
    return TRUE;
}
