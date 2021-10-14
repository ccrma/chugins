// this should align with the correct versions of these ChucK files
#include "chuck_dl.h"
#include "chuck_def.h"
#include "chuck_instr.h"
#include "chuck_vm.h"
#include "chuck_type.h"

#include <stdio.h>
#include <limits.h>
#include <iostream>
#include <string>
#include <memory> // std::shared_ptr

#include "DbVST3App.h"

CK_DLL_CTOR(dbvst3_ctor);
CK_DLL_DTOR(dbvst3_dtor);

CK_DLL_MFUN(dbvst3_loadPlugin);
CK_DLL_MFUN(dbvst3_printModules);
CK_DLL_MFUN(dbvst3_getNumModules);
CK_DLL_MFUN(dbvst3_selectModule);
CK_DLL_MFUN(dbvst3_getModuleName);
CK_DLL_MFUN(dbvst3_getNumParameters);
CK_DLL_MFUN(dbvst3_getParameterName);
CK_DLL_MFUN(dbvst3_getParameter);
CK_DLL_MFUN(dbvst3_setParameter);
CK_DLL_MFUN(dbvst3_setParameterByName);
CK_DLL_MFUN(dbvst3_noteOn);
CK_DLL_MFUN(dbvst3_noteOff);
CK_DLL_MFUN(dbvst3_midiEvent);

CK_DLL_TICKF(dbvst3_multitick);

static t_CKINT dbvst3_data_offset = 0;
static t_CKINT dbvst3_datastr_offset = 0;

//-----------------------------------------------------------------------------
// VST3Host
//   A chugin to act as a simple audioprocessing host for VST3 plugins.
//   Current limitations:
//      - 32bit stereo input+output only.
//      - no editor hosting (we're a console app), so parameters can only be 
//        changed programatically. (or external preset files?).
//     
//   - inspired by DBraun's VST chugin, but doesn't rely on juce
//     ie: it interops directly with the steinberg vst3 api.
//     That api offers a GPL3 license which dovetails nicely with
//     ChucK's and this.
//-----------------------------------------------------------------------------
static std::shared_ptr<DbVST3App> s_vstAppPtr; // shared across multiple instances

class DbVST3 // a chuck context associated with instantiation of our ugen
{
public:
    // constructor
    DbVST3(t_CKFLOAT srate, int nch=2)
    {
        if(s_vstAppPtr.get() == nullptr)
            s_vstAppPtr.reset(new DbVST3App());
        m_sampleRate = srate;
        m_nch = nch;
    }

    ~DbVST3()
    {
        m_dbVST3Ctx.Reset();
    }

    bool loadPlugin(const std::string& filename);
    void printModules();
    int getNumModules();
    int selectModule(int index); // returns 0 on success
    std::string getModuleName(); // of current module

    int getNumParameters();
    int getParameterName(int index, std::string &pnm);
    float getParameter(int index);
    bool setParameter(int index, float v);
    bool setParameter(std::string const &nm, float v);

    bool noteOn(int note, float velocity);
    bool noteOff(int note, float velocity);
    bool midiEvent(int data1, int data2, int data3, float when);

    void multitick(SAMPLE* in, SAMPLE* out, int nframes);

private:
    t_CKINT m_nch;
    t_CKFLOAT m_sampleRate;
    std::string m_pluginPath;

    DbVST3Ctx m_dbVST3Ctx;
};

bool DbVST3::loadPlugin(const std::string& filepath)
{
    int err = s_vstAppPtr->OpenPlugin(filepath, m_dbVST3Ctx);
    if(!err)
        err = m_dbVST3Ctx.InitProcessing(m_nch, m_nch, m_sampleRate);
    return err == 0;
}

void 
DbVST3::printModules()
{
    m_dbVST3Ctx.Print(std::cout, false/*detailed*/);
}

int 
DbVST3::getNumModules()
{
    return m_dbVST3Ctx.GetNumModules();
}

int 
DbVST3::selectModule(int m)
{
    return m_dbVST3Ctx.ActivateModule(m, m_nch, m_nch, m_sampleRate); // 0 == success, 
}

std::string 
DbVST3::getModuleName()
{
    return m_dbVST3Ctx.GetModuleName();
}

int 
DbVST3::getNumParameters() 
{
    return m_dbVST3Ctx.GetNumParameters();
}

int 
DbVST3::getParameterName(int index, std::string &nm) 
{
    return m_dbVST3Ctx.GetParameterName(index, nm);
}

float 
DbVST3::getParameter(int index) 
{
    return 0.f;
}

bool 
DbVST3::setParameter(int index, float v) 
{
    return m_dbVST3Ctx.SetParamValue(index, v);
}

bool 
DbVST3::setParameter(std::string const &nm, float v) 
{
    return m_dbVST3Ctx.SetParamValue(nm, v);
}

/* --------------------------------------------------------------------- */
bool 
DbVST3::noteOn(int noteNumber, float velocity) 
{
    // 144 is channel 0
    this->midiEvent(144, noteNumber,  int(velocity * 127), 0.);
    return true;
}

bool 
DbVST3::noteOff(int noteNumber, float velocity) 
{
    // 128 is channel 0
    this->midiEvent(128, noteNumber,  int(velocity * 127), 0.);
    return true;
}

bool
DbVST3::midiEvent(int data1, int data2, int data3, float dur)
{
    //std::cout << "DbVST3::midiEvent " << data1 << " " << data2 
    // << " " << data3 << " " << dur << "\n";
    // dur is zero unless playing from file
    return m_dbVST3Ctx.MidiEvent(data1, data2, data3); 
}

/* --------------------------------------------------------------------- */
void 
DbVST3::multitick(SAMPLE* in, SAMPLE* out, int nframes)
{
    if(!m_dbVST3Ctx.Ready())
    {
        if(nframes == 1) // actual case
        {
            out[0] = 0.f;
            out[1] = 0.f;
        }
        else
        {
            for(int i = 0; i < nframes; i++)
            {
                out[i * 2] = 0.f;
                out[i * 2 + 1] = 0.f;
            }
        }
        return;
    }
    else
    {
        m_dbVST3Ctx.ProcessSamples(in, out, nframes);
    }
}

//-----------------------------------------------------------------------------
// query function: chuck calls this when loading the Chugin
//-----------------------------------------------------------------------------
CK_DLL_QUERY( DbVST3 )
{
    // hmm, don't change this...
    QUERY->setname(QUERY, "DbVST3");

    // begin the class definition
    // can change the second argument to extend a different ChucK class
    QUERY->begin_class(QUERY, "DbVST3", "UGen");

    // register the constructor (probably no need to change)
    QUERY->add_ctor(QUERY, dbvst3_ctor);
    // register the destructor (probably no need to change)
    QUERY->add_dtor(QUERY, dbvst3_dtor);

    // 2, 2 to do stereo in and stereo out
    // can only add funcf or func, not both
    QUERY->add_ugen_funcf(QUERY, dbvst3_multitick, NULL, 2, 2);

    QUERY->add_mfun(QUERY, dbvst3_loadPlugin, "int", "loadPlugin");
    QUERY->add_arg(QUERY, "string", "filename");

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
    DbVST3 * b_obj = new DbVST3(API->vm->get_srate(API, SHRED));

    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, dbvst3_data_offset) = (t_CKINT) b_obj;

    std::string defval;
    OBJ_MEMBER_STRING(SELF, dbvst3_datastr_offset) = (Chuck_String *)
        API->object->create_string(API, SHRED, defval);
}

// implementation for the destructor
CK_DLL_DTOR(dbvst3_dtor)
{
    // get our c++ class pointer
    DbVST3 * b_obj = (DbVST3 *) OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
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

    DbVST3* b = (DbVST3*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);

    RETURN->v_int = b->loadPlugin(filename);
}

CK_DLL_MFUN(dbvst3_printModules)
{
    DbVST3* b = (DbVST3*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    b->printModules();
}

CK_DLL_MFUN(dbvst3_getNumModules)
{
    DbVST3* b = (DbVST3*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    RETURN->v_int = b->getNumModules();
}

CK_DLL_MFUN(dbvst3_selectModule)
{
    DbVST3* b = (DbVST3*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    t_CKINT index = GET_NEXT_INT(ARGS);
    RETURN->v_int = b->selectModule(index);
}

// implementation for tick function
CK_DLL_TICKF(dbvst3_multitick)
{
    // get our c++ class pointer
    DbVST3* b = (DbVST3*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);

    // invoke our tick function; store in the magical out variable
    if (b) b->multitick(in, out, nframes);

    // yes
    return TRUE;
}

CK_DLL_MFUN(dbvst3_getModuleName)
{
    DbVST3* b = (DbVST3*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    std::string s = b->getModuleName();
    Chuck_String *ckstr = OBJ_MEMBER_STRING(SELF, dbvst3_datastr_offset);
    ckstr->set(s);
    RETURN->v_string = ckstr;
}

CK_DLL_MFUN(dbvst3_getNumParameters)
{
    DbVST3* b = (DbVST3*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);

    RETURN->v_int = b->getNumParameters();
}

CK_DLL_MFUN(dbvst3_getParameterName)
{
    t_CKINT index = GET_NEXT_INT(ARGS);

    DbVST3* b = (DbVST3*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    std::string s;
    Chuck_String *ckstr = OBJ_MEMBER_STRING(SELF, dbvst3_datastr_offset);
    if(0 == b->getParameterName(index, s))
        ckstr->set(s);
    RETURN->v_string = ckstr;
}

CK_DLL_MFUN(dbvst3_getParameter)
{
    t_CKINT index = GET_NEXT_INT(ARGS);

    DbVST3* b = (DbVST3*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);

    RETURN->v_float = b->getParameter(index);
}

CK_DLL_MFUN(dbvst3_setParameter)
{
    t_CKINT index = GET_NEXT_INT(ARGS);
    t_CKFLOAT val = GET_NEXT_FLOAT(ARGS);
    DbVST3* b = (DbVST3*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    RETURN->v_int = b->setParameter(index, val);
}

CK_DLL_MFUN(dbvst3_setParameterByName)
{
    std::string nm = GET_NEXT_STRING_SAFE(ARGS);
    t_CKFLOAT val = GET_NEXT_FLOAT(ARGS);
    DbVST3* b = (DbVST3*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    RETURN->v_int = b->setParameter(nm, val);
}

CK_DLL_MFUN(dbvst3_noteOn)
{
    t_CKINT noteNumber = GET_NEXT_INT(ARGS);
    t_CKFLOAT velocity = GET_NEXT_FLOAT(ARGS);
    DbVST3* b = (DbVST3*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    RETURN->v_int = b->noteOn(noteNumber, velocity);
}

CK_DLL_MFUN(dbvst3_noteOff)
{
    t_CKINT noteNumber = GET_NEXT_INT(ARGS);
    t_CKFLOAT velocity = GET_NEXT_FLOAT(ARGS);
    DbVST3* b = (DbVST3*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
    RETURN->v_int = b->noteOff(noteNumber, velocity);
} 

CK_DLL_MFUN(dbvst3_midiEvent)
{
    DbVST3* b = (DbVST3*)OBJ_MEMBER_INT(SELF, dbvst3_data_offset);
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