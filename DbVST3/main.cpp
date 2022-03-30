// this should align with the correct versions of these ChucK files
#include "chuck_dl.h"

#include <stdio.h>
#include <limits.h>
#include <iostream>
#include <string>
#include <memory> // std::shared_ptr

#include "chugin.h" // VST3Chugin

CK_DLL_CTOR(dbvst3_ctor);
CK_DLL_DTOR(dbvst3_dtor);

CK_DLL_MFUN(dbvst3_loadPlugin);
CK_DLL_MFUN(dbvst3_ready);
CK_DLL_MFUN(dbvst3_setVerbosity);
CK_DLL_MFUN(dbvst3_printModules);
CK_DLL_MFUN(dbvst3_getNumModules);
CK_DLL_MFUN(dbvst3_selectModule);
CK_DLL_MFUN(dbvst3_getModuleName);
CK_DLL_MFUN(dbvst3_getNumParameters);
CK_DLL_MFUN(dbvst3_getParameterName);
CK_DLL_MFUN(dbvst3_getParameter);
CK_DLL_MFUN(dbvst3_setParameter);
CK_DLL_MFUN(dbvst3_setParameterByName);
CK_DLL_MFUN(dbvst3_setInputRouting);
CK_DLL_MFUN(dbvst3_setOutputRouting);
CK_DLL_MFUN(dbvst3_noteOn);
CK_DLL_MFUN(dbvst3_noteOff);
CK_DLL_MFUN(dbvst3_midiEvent);

CK_DLL_TICKF(dbvst3_multitick);

static t_CKINT dbvst3_data_offset = 0;
static t_CKINT dbvst3_datastr_offset = 0;

//-----------------------------------------------------------------------------
//   A chugin to act as a simple audioprocessing host for VST3 plugins.
//   Current limitations:
//      - 32bit stereo input+output only.
//      - no editor hosting (we're a console app), so parameters can only be 
//        changed programatically (or via external preset files?).
//     
//   - Inspired by DBraun's VST chugin, but this doesn't support VST
//     nor rely on juce.  ie: it interops directly with the steinberg 
//     vst3 api. That api offers a GPL3 license which dovetails nicely 
//     with ChucK's and this.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// query function: chuck calls this when loading the Chugin
//-----------------------------------------------------------------------------
CK_DLL_QUERY(DbVST3Chugin) // macro produces string, needn't match class names.
{
    QUERY->setname(QUERY, "DbVST3"); // don't change this (?)

    // begin the class definition
    // can change the second argument to extend a different ChucK class
    QUERY->begin_class(QUERY, "DbVST3", "UGen");

    // register the constructor (probably no need to change)
    QUERY->add_ctor(QUERY, dbvst3_ctor);
    // register the destructor (probably no need to change)
    QUERY->add_dtor(QUERY, dbvst3_dtor);

    // 2, 2 to do stereo in and stereo out
    // can only add funcf or func, not both, so to obtain
    // 0->1, 0->2, 1->1 and 2->1 (side-chain), 2->2, and 
    // 3->2 (side-chain), we might need to instantiate 6 chugins
    // like: DbVST3_1_1, etc.
    // Til then we'll go 2->2 and expect it to cover these cases:
    // 0->1 (mono-instrument), 0->2 (stereo-instrument), 1->1 
    // (mono filter), 2->2 (stereo filter).
    QUERY->add_ugen_funcf(QUERY, dbvst3_multitick, NULL, 2, 2);

    QUERY->add_mfun(QUERY, dbvst3_loadPlugin, "int", "loadPlugin");
    QUERY->add_arg(QUERY, "string", "filename");

    QUERY->add_mfun(QUERY, dbvst3_ready, "int", "ready");

    QUERY->add_mfun(QUERY, dbvst3_setVerbosity, "void", "setVerbosity");
    QUERY->add_arg(QUERY, "int", "level");

    QUERY->add_mfun(QUERY, dbvst3_printModules, "void", "printModules");

    QUERY->add_mfun(QUERY, dbvst3_getNumModules, "int", "getNumModules");

    QUERY->add_mfun(QUERY, dbvst3_selectModule, "int", "selectModule");
    QUERY->add_arg(QUERY, "int", "index");

    QUERY->add_mfun(QUERY, dbvst3_getModuleName, "string", "getModuleName");

    QUERY->add_mfun(QUERY, dbvst3_getNumParameters, "int", "getNumParameters");

    QUERY->add_mfun(QUERY, dbvst3_getParameterName, "string", "getParameterName");
    QUERY->add_arg(QUERY, "int", "index");

    QUERY->add_mfun(QUERY, dbvst3_getParameter, "float", "getParameter");
    QUERY->add_arg(QUERY, "int", "index");

    QUERY->add_mfun(QUERY, dbvst3_setParameter, "int", "setParameter");
    QUERY->add_arg(QUERY, "int", "index");
    QUERY->add_arg(QUERY, "float", "value");

    QUERY->add_mfun(QUERY, dbvst3_setParameterByName, "int", "setParameter");
    QUERY->add_arg(QUERY, "string", "nm");
    QUERY->add_arg(QUERY, "float", "value");

    QUERY->add_mfun(QUERY, dbvst3_noteOn, "int", "noteOn");
    QUERY->add_arg(QUERY, "int", "noteNumber");
    QUERY->add_arg(QUERY, "float", "velocity");

    QUERY->add_mfun(QUERY, dbvst3_noteOff, "int", "noteOff");
    QUERY->add_arg(QUERY, "int", "noteNumber");
    QUERY->add_arg(QUERY, "float", "velocity");

    QUERY->add_mfun(QUERY, dbvst3_midiEvent, "void", "midiEvent");
    QUERY->add_arg(QUERY, "MidiMsg", "msg");

    // WIP/experimental
    QUERY->add_mfun(QUERY, dbvst3_setInputRouting, "void", "setInputRouting");
    QUERY->add_arg(QUERY, "string", "routing");

    QUERY->add_mfun(QUERY, dbvst3_setOutputRouting, "void", "setOutputRouting");
    QUERY->add_arg(QUERY, "string", "routing");

    // this reserves a variable in the ChucK internal class to store
    // referene to the c++ class we defined above
    dbvst3_data_offset = QUERY->add_mvar(QUERY, "int", "@dbvst3_data", false);
    // here is a chuck-size string member var (since we can't currently
    // allocate a Chuck_String on the plugin side).
    dbvst3_datastr_offset = QUERY->add_mvar(QUERY, "string", "str", false);

    // end the class definition
    // IMPORTANT: this MUST be called!
    QUERY->end_class(QUERY);

    // wasn't that a breeze?
    return TRUE;
}

// implementation for the constructor
CK_DLL_CTOR(dbvst3_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, dbvst3_data_offset) = 0;

    // instantiate our internal c++ class representation
    VST3Chugin* b_obj = new VST3Chugin(API->vm->get_srate(API, SHRED));

    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, dbvst3_data_offset) = (t_CKINT) b_obj;

    std::string defval;
    OBJ_MEMBER_STRING(SELF, dbvst3_datastr_offset) = (Chuck_String *)
        API->object->create_string(API, SHRED, defval);
}

// implementation for the destructor
CK_DLL_DTOR(dbvst3_dtor)
{
    // std::cerr << "VST3Chugin dtor\n"; // dtor isn't called if shred invokes me.exit()
    // get our c++ class pointer
    VST3Chugin* b_obj = (VST3Chugin *) OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    // check it
    if( b_obj )
    {
        // clean up
        delete b_obj;
        OBJ_MEMBER_INT(SELF, dbvst3_data_offset) = 0;
        b_obj = NULL;
    }
    // may not need to delete_string? (there's no api)
}

CK_DLL_MFUN(dbvst3_loadPlugin)
{
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);

    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);

    RETURN->v_int = b->loadPlugin(filename);
}

CK_DLL_MFUN(dbvst3_ready)
{
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    RETURN->v_int = b->ready();
}

CK_DLL_MFUN(dbvst3_setVerbosity)
{
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    t_CKINT v = GET_NEXT_INT(ARGS);
    b->setVerbosity(v);
}

CK_DLL_MFUN(dbvst3_printModules)
{
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    b->printModules();
}

CK_DLL_MFUN(dbvst3_getNumModules)
{
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    RETURN->v_int = b->getNumModules();
}

CK_DLL_MFUN(dbvst3_selectModule)
{
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    t_CKINT index = GET_NEXT_INT(ARGS);
    RETURN->v_int = b->selectModule(index);
}

// implementation for tick function
CK_DLL_TICKF(dbvst3_multitick)
{
    // get our c++ class pointer
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);

    // invoke our tick function; store in the magical out variable
    if (b) b->multitick(in, out, nframes);

    // yes
    return TRUE;
}

CK_DLL_MFUN(dbvst3_getModuleName)
{
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    std::string s = b->getModuleName();
    Chuck_String *ckstr = OBJ_MEMBER_STRING(SELF, dbvst3_datastr_offset);
    ckstr->set(s);
    RETURN->v_string = ckstr;
}

CK_DLL_MFUN(dbvst3_getNumParameters)
{
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);

    RETURN->v_int = b->getNumParameters();
}

CK_DLL_MFUN(dbvst3_getParameterName)
{
    t_CKINT index = GET_NEXT_INT(ARGS);

    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    std::string s;
    Chuck_String *ckstr = OBJ_MEMBER_STRING(SELF, dbvst3_datastr_offset);
    if(0 == b->getParameterName(index, s))
        ckstr->set(s);
    RETURN->v_string = ckstr;
}

CK_DLL_MFUN(dbvst3_getParameter)
{
    t_CKINT index = GET_NEXT_INT(ARGS);

    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);

    RETURN->v_float = b->getParameter(index);
}

CK_DLL_MFUN(dbvst3_setParameter)
{
    t_CKINT index = GET_NEXT_INT(ARGS);
    t_CKFLOAT val = GET_NEXT_FLOAT(ARGS);
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    RETURN->v_int = b->setParameter(index, val);
}

CK_DLL_MFUN(dbvst3_setParameterByName)
{
    std::string nm = GET_NEXT_STRING_SAFE(ARGS);
    t_CKFLOAT val = GET_NEXT_FLOAT(ARGS);
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    RETURN->v_int = b->setParameter(nm, val);
}

// experimental
CK_DLL_MFUN(dbvst3_setInputRouting)
{
    std::string r = GET_NEXT_STRING_SAFE(ARGS);
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    b->setInputRouting(r);
}

// experimental
CK_DLL_MFUN(dbvst3_setOutputRouting)
{
    std::string r = GET_NEXT_STRING_SAFE(ARGS);
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    b->setOutputRouting(r);
}

CK_DLL_MFUN(dbvst3_noteOn)
{
    t_CKINT noteNumber = GET_NEXT_INT(ARGS);
    t_CKFLOAT velocity = GET_NEXT_FLOAT(ARGS);
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    RETURN->v_int = b->noteOn(noteNumber, velocity);
}

CK_DLL_MFUN(dbvst3_noteOff)
{
    t_CKINT noteNumber = GET_NEXT_INT(ARGS);
    t_CKFLOAT velocity = GET_NEXT_FLOAT(ARGS);
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    RETURN->v_int = b->noteOff(noteNumber, velocity);
} 

CK_DLL_MFUN(dbvst3_midiEvent)
{
    VST3Chugin* b = (VST3Chugin*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    Chuck_Object *msg = GET_NEXT_OBJECT(ARGS);

    /* object mvar offsets for MidiMsg not exported by chuck
     * so we'll hard code values.  Here we assume we're in
     * 64-bit land.
     */
    int data1 = OBJ_MEMBER_INT(msg, 8); // MidiMsg_offset_data1
    int data2 = OBJ_MEMBER_INT(msg, 16); // MidiMsg_offset_data2
    int data3 = OBJ_MEMBER_INT(msg, 24); // MidiMsg_offset_data3
    t_CKDUR when = OBJ_MEMBER_DUR(msg, 32); // MidiMsg_offset_when);

    RETURN->v_int = b->midiEvent(data1, data2, data3, when);
}