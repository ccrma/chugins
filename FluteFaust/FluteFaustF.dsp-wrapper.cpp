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
#define mydsp FluteFaust

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
static t_CKUINT FluteFaust_offset_data = 0;
static int g_sr = 44100;
static int g_nChans = 1;

CK_DLL_CTOR(FluteFaust_ctor)
{
    // return data to be used later
    FluteFaust *d = new FluteFaust;
    OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data) = (t_CKUINT)d;
    d->init(g_sr);
    d->ck_frame_in = new SAMPLE*[g_nChans];
    d->ck_frame_out = new SAMPLE*[g_nChans];
}

CK_DLL_DTOR(FluteFaust_dtor)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);

    delete[] d->ck_frame_in;
    delete[] d->ck_frame_out;
    
    delete d;
    
    OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data) = 0;
}

// mono tick
CK_DLL_TICK(FluteFaust_tick)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    
    d->ck_frame_in[0] = &in;
    d->ck_frame_out[0] = out;

    d->compute(1, d->ck_frame_in, d->ck_frame_out);
    
    return TRUE;
}

// multichannel tick
CK_DLL_TICKF(FluteFaust_tickf)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    
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

CK_DLL_MFUN(FluteFaust_ctrl_fHslider4)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider4 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider4);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider4)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider4);
}


CK_DLL_MFUN(FluteFaust_ctrl_fHslider3)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider3 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider3);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider3)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider3);
}


CK_DLL_MFUN(FluteFaust_ctrl_fHslider2)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider2 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider2);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider2)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider2);
}


CK_DLL_MFUN(FluteFaust_ctrl_fHslider5)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider5 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider5);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider5)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider5);
}


CK_DLL_MFUN(FluteFaust_ctrl_fHslider6)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider6 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider6);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider6)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider6);
}


CK_DLL_MFUN(FluteFaust_ctrl_fHslider1)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider1 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider1);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider1)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider1);
}


CK_DLL_MFUN(FluteFaust_ctrl_fHslider0)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider0 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider0);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider0)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider0);
}


CK_DLL_MFUN(FluteFaust_ctrl_fHslider7)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    d->fHslider7 = (SAMPLE)GET_CK_FLOAT(ARGS);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider7);
}

CK_DLL_MFUN(FluteFaust_cget_fHslider7)
{
    FluteFaust *d = (FluteFaust*)OBJ_MEMBER_UINT(SELF, FluteFaust_offset_data);
    RETURN->v_float = (t_CKFLOAT)(d->fHslider7);
}




CK_DLL_QUERY(FluteFaust_query)
{
    g_sr = QUERY->srate;

	FluteFaust temp; // needed to get IO channel count

    QUERY->setname(QUERY, "FluteFaust");
    
    QUERY->begin_class(QUERY, "FluteFaust", "UGen");
    QUERY->doc_class(QUERY, "FluteFaust");
    QUERY->add_ex(QUERY, "FluteFaust-test.ck");
    
    QUERY->add_ctor(QUERY, FluteFaust_ctor);
    QUERY->add_dtor(QUERY, FluteFaust_dtor);
    
    g_nChans = std::max(temp.getNumInputs(), temp.getNumOutputs());
    
    if(g_nChans == 1)
        QUERY->add_ugen_func(QUERY, FluteFaust_tick, NULL, g_nChans, g_nChans);
    else
        QUERY->add_ugen_funcf(QUERY, FluteFaust_tickf, NULL, g_nChans, g_nChans);

    // add member variable
    FluteFaust_offset_data = QUERY->add_mvar( QUERY, "int", "@FluteFaust_data", FALSE );
    if( FluteFaust_offset_data == CK_INVALID_OFFSET ) goto error;

    
    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider4 , "float", "pressure" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider4 , "float", "pressure" );
    QUERY->add_arg( QUERY, "float", "pressure" );
    QUERY->doc_func(QUERY, "float value controls pressure" );
    

    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider3 , "float", "breathGain" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider3 , "float", "breathGain" );
    QUERY->add_arg( QUERY, "float", "breathGain" );
    QUERY->doc_func(QUERY, "float value controls breathGain" );
    

    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider2 , "float", "breathCutoff" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider2 , "float", "breathCutoff" );
    QUERY->add_arg( QUERY, "float", "breathCutoff" );
    QUERY->doc_func(QUERY, "float value controls breathCutoff" );
    

    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider5 , "float", "vibratoFreq" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider5 , "float", "vibratoFreq" );
    QUERY->add_arg( QUERY, "float", "vibratoFreq" );
    QUERY->doc_func(QUERY, "float value controls vibratoFreq" );
    

    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider6 , "float", "vibratoGain" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider6 , "float", "vibratoGain" );
    QUERY->add_arg( QUERY, "float", "vibratoGain" );
    QUERY->doc_func(QUERY, "float value controls vibratoGain" );
    

    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider1 , "float", "tubeLength" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider1 , "float", "tubeLength" );
    QUERY->add_arg( QUERY, "float", "tubeLength" );
    QUERY->doc_func(QUERY, "float value controls tubeLength" );
    

    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider0 , "float", "mouthPosition" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider0 , "float", "mouthPosition" );
    QUERY->add_arg( QUERY, "float", "mouthPosition" );
    QUERY->doc_func(QUERY, "float value controls mouthPosition" );
    

    QUERY->add_mfun( QUERY, FluteFaust_cget_fHslider7 , "float", "outGain" );
    
    QUERY->add_mfun( QUERY, FluteFaust_ctrl_fHslider7 , "float", "outGain" );
    QUERY->add_arg( QUERY, "float", "outGain" );
    QUERY->doc_func(QUERY, "float value controls outGain" );
    


    // end import
	QUERY->end_class(QUERY);
	
    return TRUE;

error:
    // end import
	QUERY->end_class(QUERY);

    return FALSE;
}
