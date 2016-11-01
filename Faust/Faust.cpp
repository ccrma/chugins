//-----------------------------------------------------------------------------
// name: Faust.cpp
// desc: Faust ChucK chugin
//
// authors: Ge Wang (ge@ccrma.stanford.edu)
//          Romain Michon (rmichon@ccrma.stanford.edu)
// date: Spring 2016
//
// NOTE: be mindful of chuck/chugin compilation, particularly on OSX
//       compiled for 10.5 chuck may not work well with 10.10 chugin!
//-----------------------------------------------------------------------------

#ifndef MAX_INPUTS
	#define MAX_INPUTS 2
#endif
#ifndef MAX_OUTPUTS
	#define MAX_OUTPUTS 2
#endif

// this should align with the correct versions of these ChucK files
#include "chuck_dl.h"
#include "chuck_def.h"

// general includes
#include <stdio.h>
#include <limits.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
using namespace std;

// faust include
#include "faust/dsp/llvm-dsp.h"
#include "faust/gui/UI.h"
#include "faust/gui/PathBuilder.h"

// declaration of chugin constructor
CK_DLL_CTOR(faust_ctor);
// declaration of chugin desctructor
CK_DLL_DTOR(faust_dtor);
// multi-channel audio synthesis tick function
CK_DLL_TICKF(faust_tickf);

// example of getter/setter
CK_DLL_MFUN(faust_eval);
CK_DLL_MFUN(faust_compile);
CK_DLL_MFUN(faust_v_set);
CK_DLL_MFUN(faust_v_get);
CK_DLL_MFUN(faust_dump);
CK_DLL_MFUN(faust_ok);
CK_DLL_MFUN(faust_error);
CK_DLL_MFUN(faust_code);
CK_DLL_MFUN(faust_test);

// this is a special offset reserved for Chugin internal data
t_CKINT faust_data_offset = 0;

#ifndef FAUSTFLOAT
  #define FAUSTFLOAT float
#endif




//-----------------------------------------------------------------------------
// name: class FauckUI
// desc: Faust ChucK UI -> map of complete hierarchical path and zones
//-----------------------------------------------------------------------------
class FauckUI : public UI, public PathBuilder
{
protected:
    // name to pointer map
    std::map<std::string, FAUSTFLOAT*> fZoneMap;
    
    // insert into map
    void insertMap( std::string label, FAUSTFLOAT * zone )
    {
        // map
        fZoneMap[label] = zone;
    }
    
public:
    // constructor
    FauckUI() { }
    // destructor
    virtual ~FauckUI() { }
    
    // -- widget's layouts
    void openTabBox(const char* label)
    {
        fControlsLevel.push_back(label);
    }
    void openHorizontalBox(const char* label)
    {
        fControlsLevel.push_back(label);
    }
    void openVerticalBox(const char* label)
    {
        fControlsLevel.push_back(label);
    }
    void closeBox()
    {
        fControlsLevel.pop_back();
    }
    
    // -- active widgets
    void addButton(const char* label, FAUSTFLOAT* zone)
    {
        insertMap(buildPath(label), zone);
    }
    void addCheckButton(const char* label, FAUSTFLOAT* zone)
    {
        insertMap(buildPath(label), zone);
    }
    void addVerticalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT fmin, FAUSTFLOAT fmax, FAUSTFLOAT step)
    {
        insertMap(buildPath(label), zone);
    }
    void addHorizontalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT fmin, FAUSTFLOAT fmax, FAUSTFLOAT step)
    {
        insertMap(buildPath(label), zone);
    }
    void addNumEntry(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT fmin, FAUSTFLOAT fmax, FAUSTFLOAT step)
    {
        insertMap(buildPath(label), zone);
    }
    
    // -- passive widgets
    void addHorizontalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT fmin, FAUSTFLOAT fmax)
    {
        insertMap(buildPath(label), zone);
    }
    void addVerticalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT fmin, FAUSTFLOAT fmax)
    {
        insertMap(buildPath(label), zone);
    }
    
    // -- metadata declarations
    void declare(FAUSTFLOAT* zone, const char* key, const char* val)
    {}
    
    // set/get
    void setValue( const std::string& path, FAUSTFLOAT value )
    {
        // append "/0x00/" if necessary
        string p = path.length() > 0 && path[0] == '/' ? path : string("/0x00/")+path;

        // TODO: should check if path valid?
        if( fZoneMap.find(p) == fZoneMap.end() )
        {
            // error
            cerr << "[Faust]: cannot set parameter named: " << path;
            if( path != p ) cerr << " OR " << p << endl;
            else cerr << endl;
            return;
        }
        
        // set it!
        *fZoneMap[p] = value;
    }
    
    float getValue(const std::string& path)
    {
        // append "/0x00/" if necessary
        string p = path.length() > 0 && path[0] == '/' ? path : string("/0x00/")+path;

        // TODO: should check if path valid?
        if( fZoneMap.find(p) == fZoneMap.end() )
        {
            // error
            cerr << "[Faust]: cannot get parameter named: " << path;
            if( path != p ) cerr << " OR " << p << endl;
            else cerr << endl;
            return 0;
        }
        
        return *fZoneMap[path];
    }
    
    void dumpParams()
    {
        // iterator
        std::map<std::string, FAUSTFLOAT*>::iterator iter = fZoneMap.begin();
        // print
        cerr << "---------------- DUMPING [Faust] PARAMETERS ---------------" << endl;
        // go
        for( ; iter != fZoneMap.end(); iter++ )
        {
            // print
            cerr << iter->first << " : " << *(iter->second) << endl;
        }
        // done
        cerr << "-----------------------------------------------------------" << endl;
    }
    
    // map access
    std::map<std::string, FAUSTFLOAT*>& getMap() { return fZoneMap; }
    // get map size
    int getParamsCount() { return fZoneMap.size(); }
    // get param path
    std::string getParamPath(int index)
    {
        std::map<std::string, FAUSTFLOAT*>::iterator it = fZoneMap.begin();
        while (index-- > 0 && it++ != fZoneMap.end()) {}
        return (*it).first;
    }
};




//-----------------------------------------------------------------------------
// name: class Faust
// desc: class definition of internal chugin data
//-----------------------------------------------------------------------------
class Faust
{
public:
    // constructor
    Faust( t_CKFLOAT fs)
    {
        // sample rate
        m_srate = fs;
        // clear
        m_factory = NULL;
        m_dsp = NULL;
        m_ui = NULL;
        // zero
        m_input = NULL;
        m_output = NULL;
        // default
        m_numInputChannels = 0;
        m_numOutputChannels = 0;
        // auto import
        m_autoImport = "// Faust Chugin auto import:\n \
        import(\"stdfaust.lib\");\n";
    }
    
    // destructor
    ~Faust()
    {
        // clear
        clear();
        clearBufs();
    }
    
    // clear
    void clear()
    {
        // clean up, possibly
        if( m_factory != NULL )
        {
            deleteDSPFactory( m_factory );
            m_factory = NULL;
        }
        SAFE_DELETE(m_dsp);
        SAFE_DELETE(m_ui);
    }
    
    // clear
    void clearBufs()
    {
        if( m_input != NULL )
        {
            for( int i = 0; i < m_numInputChannels; i++ )
                SAFE_DELETE_ARRAY(m_input[i]);
        }
        if( m_output != NULL )
        {
            for( int i = 0; i < m_numOutputChannels; i++ )
                SAFE_DELETE_ARRAY(m_output[i]);
        }
        SAFE_DELETE_ARRAY(m_input);
        SAFE_DELETE_ARRAY(m_output);
    }
    
    // allocate
    void allocate( int inputChannels, int outputChannels )
    {
        // clear
        clearBufs();
        
        // set
        m_numInputChannels = min(inputChannels, MAX_INPUTS);
        m_numOutputChannels = min(outputChannels, MAX_OUTPUTS);

        // allocate channels
        m_input = new FAUSTFLOAT *[m_numInputChannels];
        m_output = new FAUSTFLOAT *[m_numOutputChannels];
        // allocate buffers for each channel
        for( int i = 0; i < m_numInputChannels; i++ )
        {
            // single sample for each
            m_input[i] = new FAUSTFLOAT[1];
        }
        for( int i = 0; i < m_numOutputChannels; i++ )
        {
            // single sample for each
            m_output[i] = new FAUSTFLOAT[1];
        }
    }
    
    // eval
    bool eval( const string & code )
    {
        // clean up
        clear();
        
        // arguments
        const int argc = 0;
        const char ** argv = NULL;
        // optimization level
        const int optimize = -1;
        
        // save
        m_code = code;
        
        // auto import
        string theCode = m_autoImport + "\n" + code;
        
        // create new factory
        m_factory = createDSPFactoryFromString( "chuck", theCode,
            argc, argv, "", m_errorString, optimize );
        
        // check for error
        if( m_errorString != "" )
        {
            // output error
            cerr << "[Faust]: " << m_errorString << endl;
            // clear
            clear();
            // done
            return false;
        }
        
        // create DSP instance
        m_dsp = m_factory ->createDSPInstance();
        
        // make new UI
        m_ui = new FauckUI();
        // build ui
        m_dsp->buildUserInterface( m_ui );
        
        // get channels
        int inputs = m_dsp->getNumInputs();
        int outputs = m_dsp->getNumOutputs();
        
        // see if we need to alloc
        if( inputs > m_numInputChannels || outputs > m_numOutputChannels )
        {
            // clear and allocate
            allocate( inputs, outputs );
        }
        
        // init
        m_dsp->init( (int)(m_srate + .5) );
        
        return true;
    }

    // compile
    bool compile( const string & path )
    {
        // open file
        ifstream fin( path.c_str() );
        // check
        if( !fin.good() )
        {
            // error
            cerr << "[Faust]: ERROR opening file: '" << path << "'" << endl;
            return false;
        }
        
        // clear code string
        m_code = "";
        // get it
        for( string line; std::getline( fin, line ); )
            m_code += line + '\n';
        
        // eval it
        return eval( m_code );
    }
    
    // dump (snapshot)
    void dump()
    {
        if( m_ui != NULL && m_dsp != NULL ) m_ui->dumpParams();
    }

	void tick( SAMPLE * in, SAMPLE * out, int nframes ){
		if( m_dsp != NULL ){		
			for(int f = 0; f < nframes; f++)
        	{			
				for(int c = 0; c < m_numInputChannels; c++)
            	{
					m_input[c][0] = in[f*m_numInputChannels+c];				
				}
				m_dsp->compute( 1, m_input, m_output );
				for(int c = 0; c < m_numOutputChannels; c++)
            	{
					out[f*m_numOutputChannels+c] = m_output[c][0];				
				}
			}
		}
	}

    // set parameter example
    t_CKFLOAT setParam( const string & n, t_CKFLOAT p )
    {
        // sanity check
        if( !m_ui ) return 0;

        // set the value
        m_ui->setValue( n, p );
        
        // return
        return p;
    }

    // get parameter example
    t_CKFLOAT getParam( const string & n )
    { return m_ui->getValue(n); }
    
    // get code
    string code() { return m_code; }
    
private:
    // sample rate
    t_CKFLOAT m_srate;
    // code text (pre any modifications)
    string m_code;
    // llvm factory
    llvm_dsp_factory * m_factory;
    // faust DSP object
    dsp * m_dsp;
    // faust compiler error string
    string m_errorString;
    // auto import
    string m_autoImport;
    
    // faust input buffer
    FAUSTFLOAT ** m_input;
    FAUSTFLOAT ** m_output;
    
    // input and output
    int m_numInputChannels;
    int m_numOutputChannels;
    
    // UI
    FauckUI * m_ui;
};




//-----------------------------------------------------------------------------
// query function: chuck calls this when loading the Chugin
// NOTE: developer will need to modify this function to
// add additional functions to this Chugin
//-----------------------------------------------------------------------------
CK_DLL_QUERY( Faust )
{
    // hmm, don't change this...
    QUERY->setname(QUERY, "Faust");
    
    // begin the class definition
    // can change the second argument to extend a different ChucK class
    QUERY->begin_class(QUERY, "Faust", "UGen");

    // register the constructor (probably no need to change)
    QUERY->add_ctor(QUERY, faust_ctor);
    // register the destructor (probably no need to change)
    QUERY->add_dtor(QUERY, faust_dtor);
    
    // for UGen's only: add tick function
	QUERY->add_ugen_funcf(QUERY, faust_tickf, NULL, MAX_INPUTS, MAX_OUTPUTS);

    // add .eval()
    QUERY->add_mfun(QUERY, faust_eval, "int", "eval");
    // add argument
    QUERY->add_arg(QUERY, "string", "code");
    
    // add .compile()
    QUERY->add_mfun(QUERY, faust_compile, "int", "compile");
    // add argument
    QUERY->add_arg(QUERY, "string", "path");

    // add .test()
    QUERY->add_mfun(QUERY, faust_test, "int", "test");
    // add argument
    QUERY->add_arg(QUERY, "int", "code");

    // add .v()
    QUERY->add_mfun(QUERY, faust_v_set, "float", "v");
    // add arguments
    QUERY->add_arg(QUERY, "string", "key");
    QUERY->add_arg(QUERY, "float", "value");

    // add .v()
    QUERY->add_mfun(QUERY, faust_v_get, "float", "v");
    // add argument
    QUERY->add_arg(QUERY, "string", "key");

    // add .dump()
    QUERY->add_mfun(QUERY, faust_dump, "void", "dump");
    
    // add .ok()
    QUERY->add_mfun(QUERY, faust_ok, "int", "ok");

    // add .error()
    QUERY->add_mfun(QUERY, faust_error, "string", "error");

    // add .code()
    QUERY->add_mfun(QUERY, faust_code, "string", "code");
    
    // this reserves a variable in the ChucK internal class to store 
    // referene to the c++ class we defined above
    faust_data_offset = QUERY->add_mvar(QUERY, "int", "@f_data", false);

    // end the class definition
    // IMPORTANT: this MUST be called!
    QUERY->end_class(QUERY);

    // wasn't that a breeze?
    return TRUE;
}




// implementation for the constructor
CK_DLL_CTOR(faust_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, faust_data_offset) = 0;
    
    // instantiate our internal c++ class representation
    Faust * f_obj = new Faust(API->vm->get_srate());
    
    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, faust_data_offset) = (t_CKINT) f_obj;
}


// implementation for the destructor
CK_DLL_DTOR(faust_dtor)
{
    // get our c++ class pointer
    Faust * f_obj = (Faust *) OBJ_MEMBER_INT(SELF, faust_data_offset);
    // check it
    if( f_obj )
    {
        // clean up
        delete f_obj;
        OBJ_MEMBER_INT(SELF, faust_data_offset) = 0;
        f_obj = NULL;
    }
}

// implementation for tick function
CK_DLL_TICKF(faust_tickf)
{
    // get our c++ class pointer
    Faust * f_obj = (Faust *) OBJ_MEMBER_INT(SELF, faust_data_offset);

    // invoke our tick function; store in the magical out variable
    if(f_obj) f_obj->tick(in, out, nframes);

    // yes
    return TRUE;
}

CK_DLL_MFUN(faust_eval)
{
    // get our c++ class pointer
    Faust * f = (Faust *) OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get argument
    std::string code = GET_NEXT_STRING(ARGS)->str;
    // eval it
    RETURN->v_int = f->eval( code );
}

CK_DLL_MFUN(faust_test)
{
    // get argument
    t_CKINT x = GET_NEXT_INT(ARGS);
    // print
    cerr << "TEST: " << x << endl;
    // eval it
    RETURN->v_int = x;
}

CK_DLL_MFUN(faust_compile)
{
    // get our c++ class pointer
    Faust * f = (Faust *) OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get argument
    std::string code = GET_NEXT_STRING(ARGS)->str;
    // eval it
    RETURN->v_int = f->compile( code );
}

CK_DLL_MFUN(faust_v_set)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get name
    std::string name = GET_NEXT_STRING(ARGS)->str;
    // get value
    t_CKFLOAT v = GET_NEXT_FLOAT(ARGS);
    // call it
    f->setParam( name, v );
    // return it
    RETURN->v_float = v;
}

CK_DLL_MFUN(faust_v_get)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get name
    std::string name = GET_NEXT_STRING(ARGS)->str;
    // call it
    RETURN->v_float = f->getParam( name );
}

CK_DLL_MFUN(faust_dump)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // call it
    f->dump();
}

CK_DLL_MFUN(faust_ok)
{
}

CK_DLL_MFUN(faust_error)
{
}

CK_DLL_MFUN(faust_code)
{
//    // get our c++ class pointer
//    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
//    // chuck string TODO: verify memory
//    Chuck_String * str = (Chuck_String *)instantiate_and_initialize_object( &t_string, NULL );
//    // set
//    str->str = f->code();
//    // return
//    RETURN->v_string = str;
}
