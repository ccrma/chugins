#include "chugin.h"
// #include "chuck_dl.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <map>
#include <string>
#include <cmath>
#include <algorithm>

//-------------------------------------------------------------------
// Generic min and max using C++ inline
//-------------------------------------------------------------------

inline int      lsr (int x, int n)          { return int(((unsigned int)x) >> n); }
inline int      int2pow2 (int x)            { int r=0; while ((1<<r)<x) r++; return r; }


/******************************************************************************
 *******************************************************************************
 
 FAUST META DATA
 
 *******************************************************************************
 *******************************************************************************/

struct Meta : std::map<std::string, std::string>
{
    void declare(const char* key, const char* value)
    {
        (*this)[key] = value;
    }
};

/* UI class - do-nothing (from FAUST/minimal.cpp) */

#ifdef WIN32
#ifdef interface
#undef interface
#endif // interface
#endif // WIN32

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

class Soundfile;

class UI
{
    public:
        virtual ~UI() {}
        
        // active widgets
        virtual void addButton(const char* label, FAUSTFLOAT* zone) = 0;
        virtual void addCheckButton(const char* label, FAUSTFLOAT* zone) = 0;
        virtual void addVerticalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) = 0;
        virtual void addHorizontalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) = 0;
        virtual void addNumEntry(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) = 0;
        
        // passive widgets
        virtual void addHorizontalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max) = 0;
        virtual void addVerticalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max) = 0;
        
        // layout widgets
        virtual void openTabBox(const char* label) = 0;
        virtual void openHorizontalBox(const char* label) = 0;
        virtual void openVerticalBox(const char* label) = 0;
        virtual void closeBox() = 0;
        
        // soundfiles
        
        virtual void addSoundfile(const char* label, const char* filename, Soundfile** sf_zone) = 0;
        
        virtual void declare(FAUSTFLOAT* zone, const char* key, const char* value) {}
};

class dsp
{
    public:
        virtual ~dsp() {}

        virtual int getNumInputs() = 0;
        virtual int getNumOutputs() = 0;
        virtual void buildUserInterface(UI* interface) = 0;
        virtual int getSampleRate() = 0;
        virtual void init(int samplingRate) = 0;
        virtual void instanceInit(int sample_rate) = 0;
        virtual void instanceConstants(int sample_rate) = 0;
        virtual void instanceResetUserInterface() = 0;
        virtual void instanceClear() = 0;
        virtual dsp* clone() = 0;
        virtual void metadata(Meta* m) = 0;
        virtual void compute(int len, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) = 0;

        SAMPLE ** ck_frame_in;
        SAMPLE ** ck_frame_out;
};

/*
 * FAUST intrinsic
 */
<<includeIntrinsic>>

/*
 * FAUST defines UI values as private, but provides no getters/setters.
 * In our particular case it's way more convenient to access them directly
 * than to set up a complicated UI structure.  Also get rid of everything
 * being "virtual", since it may stop the compiler from inlining properly!
 */
#define private public
#define virtual

/* Rename the class the name of our DSP. */
#define mydsp DjembeMIDI

/*
 * FAUST class
 */
<<includeclass>>

#undef private
#undef virtual
#undef mydsp

/*
 * ChucK glue code
 */
static t_CKUINT DjembeMIDI_offset_data = 0;
static int g_sr = 44100;
static int g_nChans = 1;

CK_DLL_CTOR(DjembeMIDI_ctor)
{
    // return data to be used later
    DjembeMIDI *d = new DjembeMIDI;
    OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data) = (t_CKUINT)d;
    d->init(g_sr);
    d->ck_frame_in = new SAMPLE*[g_nChans];
    d->ck_frame_out = new SAMPLE*[g_nChans];
}

CK_DLL_DTOR(DjembeMIDI_dtor)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);

    delete[] d->ck_frame_in;
    delete[] d->ck_frame_out;
    
    delete d;
    
    OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data) = 0;
}

// mono tick
CK_DLL_TICK(DjembeMIDI_tick)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    
    d->ck_frame_in[0] = &in;
    d->ck_frame_out[0] = out;

    d->compute(1, d->ck_frame_in, d->ck_frame_out);
    
    return TRUE;
}

// multichannel tick
CK_DLL_TICKF(DjembeMIDI_tickf)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    
    for(int f = 0; f < nframes; f++)
    {
        // fake-deinterleave
        for(int c = 0; c < g_nChans; c++)
        {
            d->ck_frame_in[c] = &in[f*g_nChans+c];
            d->ck_frame_out[c] = &out[f*g_nChans+c];
        }
        
        d->compute(1, d->ck_frame_in, d->ck_frame_out);
    }
    
    return TRUE;
}

CK_DLL_MFUN(DjembeMIDI_ctrl_fHslider0)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    d->fHslider0 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider0);
}

CK_DLL_MFUN(DjembeMIDI_cget_fHslider0)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider0);
}


CK_DLL_MFUN(DjembeMIDI_ctrl_fHslider3)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    d->fHslider3 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider3);
}

CK_DLL_MFUN(DjembeMIDI_cget_fHslider3)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider3);
}


CK_DLL_MFUN(DjembeMIDI_ctrl_fHslider2)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    d->fHslider2 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider2);
}

CK_DLL_MFUN(DjembeMIDI_cget_fHslider2)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider2);
}


CK_DLL_MFUN(DjembeMIDI_ctrl_fHslider1)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    d->fHslider1 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider1);
}

CK_DLL_MFUN(DjembeMIDI_cget_fHslider1)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider1);
}


CK_DLL_MFUN(DjembeMIDI_ctrl_fHslider4)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    d->fHslider4 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider4);
}

CK_DLL_MFUN(DjembeMIDI_cget_fHslider4)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider4);
}


CK_DLL_MFUN(DjembeMIDI_ctrl_fButton0)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    d->fButton0 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fButton0);
}

CK_DLL_MFUN(DjembeMIDI_cget_fButton0)
{
    DjembeMIDI *d = (DjembeMIDI*)OBJ_MEMBER_UINT(SELF, DjembeMIDI_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fButton0);
}




CK_DLL_QUERY(DjembeMIDI_query)
{
    g_sr = QUERY->srate;

	DjembeMIDI temp; // needed to get IO channel count

    QUERY->setname(QUERY, "DjembeMIDI");
    
    QUERY->begin_class(QUERY, "DjembeMIDI", "UGen");
    QUERY->doc_class(QUERY, "DjembeMIDI");
    QUERY->add_ex(QUERY, "DjembeMIDI-test.ck");
    
    QUERY->add_ctor(QUERY, DjembeMIDI_ctor);
    QUERY->add_dtor(QUERY, DjembeMIDI_dtor);
    
    g_nChans = std::max(temp.getNumInputs(), temp.getNumOutputs());
    
    if(g_nChans == 1)
        QUERY->add_ugen_func(QUERY, DjembeMIDI_tick, NULL, g_nChans, g_nChans);
    else
        QUERY->add_ugen_funcf(QUERY, DjembeMIDI_tickf, NULL, g_nChans, g_nChans);

    // add member variable
    DjembeMIDI_offset_data = QUERY->add_mvar( QUERY, "int", "@DjembeMIDI_data", FALSE );
    if( DjembeMIDI_offset_data == CK_INVALID_OFFSET ) goto error;

    
    QUERY->add_mfun( QUERY, DjembeMIDI_cget_fHslider0 , "float", "freq" );
    
    QUERY->add_mfun( QUERY, DjembeMIDI_ctrl_fHslider0 , "float", "freq" );
    QUERY->add_arg( QUERY, "float", "freq" );
    QUERY->doc_func(QUERY, "float value controls freq" );
    

    QUERY->add_mfun( QUERY, DjembeMIDI_cget_fHslider3 , "float", "gain" );
    
    QUERY->add_mfun( QUERY, DjembeMIDI_ctrl_fHslider3 , "float", "gain" );
    QUERY->add_arg( QUERY, "float", "gain" );
    QUERY->doc_func(QUERY, "float value controls gain" );
    

    QUERY->add_mfun( QUERY, DjembeMIDI_cget_fHslider2 , "float", "strikePosition" );
    
    QUERY->add_mfun( QUERY, DjembeMIDI_ctrl_fHslider2 , "float", "strikePosition" );
    QUERY->add_arg( QUERY, "float", "strikePosition" );
    QUERY->doc_func(QUERY, "float value controls strikePosition" );
    

    QUERY->add_mfun( QUERY, DjembeMIDI_cget_fHslider1 , "float", "strikeSharpness" );
    
    QUERY->add_mfun( QUERY, DjembeMIDI_ctrl_fHslider1 , "float", "strikeSharpness" );
    QUERY->add_arg( QUERY, "float", "strikeSharpness" );
    QUERY->doc_func(QUERY, "float value controls strikeSharpness" );
    

    QUERY->add_mfun( QUERY, DjembeMIDI_cget_fHslider4 , "float", "outGain" );
    
    QUERY->add_mfun( QUERY, DjembeMIDI_ctrl_fHslider4 , "float", "outGain" );
    QUERY->add_arg( QUERY, "float", "outGain" );
    QUERY->doc_func(QUERY, "float value controls outGain" );
    

    QUERY->add_mfun( QUERY, DjembeMIDI_cget_fButton0 , "float", "gate" );
    
    QUERY->add_mfun( QUERY, DjembeMIDI_ctrl_fButton0 , "float", "gate" );
    QUERY->add_arg( QUERY, "float", "gate" );
    QUERY->doc_func(QUERY, "float value controls gate" );
    


    // end import
	QUERY->end_class(QUERY);
	
    return TRUE;

error:
    // end import
	QUERY->end_class(QUERY);

    return FALSE;
}
