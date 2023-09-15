//-----------------------------------------------------------------------------
// name: Faust.cpp
// desc: Faust ChucK chugin
//
// authors: Ge Wang (ge@ccrma.stanford.edu)
//          Romain Michon (rmichon@ccrma.stanford.edu)
// date: Spring 2016
// author: David Braun (braun@ccrma.stanford.edu)
// date: Summer 2022
//
// NOTE: be mindful of chuck/chugin compilation, particularly on OSX
//       compiled for 10.5 chuck may not work well with 10.10 chugin!
//-----------------------------------------------------------------------------

// note that 4096 would cause weird audio artifacts
#ifndef MAX_INPUTS
  #define MAX_INPUTS 2048
#endif
#ifndef MAX_OUTPUTS
  #define MAX_OUTPUTS 2048
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
//#include <filesystem>
using namespace std;

// faust include
#include "faust/dsp/llvm-dsp.h"
#include "faust/dsp/proxy-dsp.h"
#include "faust/dsp/poly-llvm-dsp.h"
#include "faust/dsp/poly-interpreter-dsp.h"

#include "faust/gui/meta.h"
#include "faust/gui/FUI.h"
#include "faust/gui/MidiUI.h"
#include "faust/gui/UI.h"
#include "faust/gui/PathBuilder.h"
#include "faust/gui/GUI.h"
//#include "faust/gui/JSONUI.h"
#include "faust/gui/SoundUI.h"

#include "faust/midi/rt-midi.h"
#include "faust/midi/RtMidi.cpp"

#include "TMutex.h"

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
CK_DLL_MFUN(faust_nvoices_get);
CK_DLL_MFUN(faust_nvoices_set);
CK_DLL_MFUN(faust_noteon);
CK_DLL_MFUN(faust_noteoff);
CK_DLL_MFUN(faust_pitchwheel);
CK_DLL_MFUN(faust_progchange);
CK_DLL_MFUN(faust_ctrlchange);
CK_DLL_MFUN(faust_assets_set);
CK_DLL_MFUN(faust_libraries_set);
CK_DLL_MFUN(faust_groupvoices_set);
CK_DLL_MFUN(faust_dynamicvoices_set);
CK_DLL_MFUN(faust_panic);
CK_DLL_MFUN(faust_dump);
CK_DLL_MFUN(faust_ok);
CK_DLL_MFUN(faust_error);
CK_DLL_MFUN(faust_code);


// this is a special offset reserved for Chugin internal data
t_CKINT faust_data_offset = 0;

#ifndef FAUSTFLOAT
  #define FAUSTFLOAT float
#endif

list<GUI*> GUI::fGuiList;
ztimedmap GUI::gTimedZoneMap;
static int numCompiled = 0;

#ifdef WIN32

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// Find path to .dll */
// https://stackoverflow.com/a/57738892/12327461
HMODULE hMod;
std::wstring MyDLLPathFull;
std::wstring MyDLLDir;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  hMod = hModule;
  const int BUFSIZE = 4096;
  wchar_t buffer[BUFSIZE];
  if (::GetModuleFileNameW(hMod, buffer, BUFSIZE - 1) <= 0) {
    return TRUE;
  }

  MyDLLPathFull = buffer;

  size_t found = MyDLLPathFull.find_last_of(L"/\\");
  MyDLLDir = MyDLLPathFull.substr(0, found);

  return TRUE;
}

#else

// this applies to both __APPLE__ and linux?

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>

// https://stackoverflow.com/a/51993539/911207
const char* getMyDLLPath(void) {
  Dl_info dl_info;
  dladdr((void*)getMyDLLPath, &dl_info);
  return (dl_info.dli_fname);
}
#endif

std::string getPathToFaustLibraries() {
  // Get the path to the directory containing basics.lib, stdfaust.lib etc.

  try {
#ifdef WIN32
    const std::wstring ws_shareFaustDir = MyDLLDir + L"\\faustlibraries";
    // std::cerr << "MyDLLDir: ";
    // std::wcerr << MyDLLDir << L'\n';
    // convert const wchar_t to char
    // https://stackoverflow.com/a/4387335
    const wchar_t* wc_shareFaustDir = ws_shareFaustDir.c_str();
    // Count required buffer size (plus one for null-terminator).
    size_t size = (wcslen(wc_shareFaustDir) + 1) * sizeof(wchar_t);
    char* char_shareFaustDir = new char[size];
    std::wcstombs(char_shareFaustDir, wc_shareFaustDir, size);

    std::string p(char_shareFaustDir);

    delete[] char_shareFaustDir;
    return p;
#elif __APPLE__
	  // look for faustlibraries inside the bundle
    // OSX only : access to the Faust bundle
    CFBundleRef fauck_bundle = CFBundleGetBundleWithIdentifier(
        CFSTR("edu.stanford.chuck.FaucK"));
    CFURLRef fauck_ref = CFBundleCopyBundleURL(fauck_bundle);
    UInt8 bundle_path[512];
    Boolean res =
        CFURLGetFileSystemRepresentation(fauck_ref, true, bundle_path, 512);
    assert(res);

    // Built the complete resource path
    std::string resourcePath = std::string((const char*)bundle_path) +
                    std::string("/Contents/Resources/");
    return resourcePath;
#else
    // this applies to __APPLE__ and LINUX
    const char* myDLLPath = getMyDLLPath();
    // std::cerr << "myDLLPath: " << myDLLPath << std::endl;
    //std::filesystem::path p = std::filesystem::path(myDLLPath);
    //p = p.parent_path() / "faustlibraries";
    //std::cerr << "p.string(): " << p.string() << endl;
    //return p.string();
    return "";
#endif
  } catch (...) {
    throw std::runtime_error("Error getting path to faustlibraries.");
  }
}

//-----------------------------------------------------------------------------
// name: class FauckUI
// desc: Faust ChucK UI -> map of complete hierarchical path and zones
//-----------------------------------------------------------------------------
class FauckUI : public UI, public PathBuilder
{
protected:
    // name to pointer map
    map<string, FAUSTFLOAT*> fZoneMap;
    
    // insert into map
    void insertMap( string label, FAUSTFLOAT * zone )
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

    // -- soundfiles
    void addSoundfile(const char* label, const char* filename, Soundfile** sf_zone)
    {}
    
    // -- metadata declarations
    void declare(FAUSTFLOAT* zone, const char* key, const char* val)
    {}
    
    // set/get
    void setValue( const string& path, FAUSTFLOAT value )
    {
        // append "/chuck/" if necessary
        string p = path.length() > 0 && path[0] == '/' ? path : string("/chuck/")+path;

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
    
    float getValue(const string& path)
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
        map<string, FAUSTFLOAT*>::iterator iter = fZoneMap.begin();
        // go
        for( ; iter != fZoneMap.end(); iter++ )
        {
            // print
            cerr << iter->first << " : " << *(iter->second) << endl;
        }
    }
    
    // map access
    map<string, FAUSTFLOAT*>& getMap() { return fZoneMap; }
    // get map size
    int getParamsCount() { return fZoneMap.size(); }
    // get param path
    string getParamPath(int index)
    {
        map<string, FAUSTFLOAT*>::iterator it = fZoneMap.begin();
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
        m_poly_factory = NULL;
        m_dsp = NULL;
        m_dsp_poly = NULL;
        m_ui = NULL;
        m_midi_ui = NULL;
        m_soundUI = NULL;
        // zero
        m_input = NULL;
        m_output = NULL;
        // default
        m_numInputChannels = 0;
        m_numOutputChannels = 0;
        // auto import
        m_autoImport = "// Faust Chugin auto import:\n \
        import(\"stdfaust.lib\");\n";
        m_assetsDirPath = string("");
        m_faustLibrariesPath = string("");
        
        clearMIDI();
    }
    
    // destructor
    ~Faust()
    {
        // clear
        clear();
        clearBufs();
    }
    
    int getNVoices() {
        return m_nvoices;
    }
    
    int setNVoices(int nvoices) {
        nvoices = max(0, nvoices);
        m_nvoices = nvoices;
        return m_nvoices;
    }

    // Note: MIDI channel is numbered in[1..16] in this layer.
    // Channel 0 means "all channels" when receiving or sending.
    
    void noteOn(int pitch, int velocity) {
        if (m_dsp_poly) {
            int channel = 0;
            m_dsp_poly->keyOn(channel, pitch, velocity);
        }
    }
    
    void noteOff(int pitch, int velocity) {
        if (m_dsp_poly) {
            int channel = 0;
            m_dsp_poly->keyOff(channel, pitch, velocity);
        }
    }
    
    void sendAllNotesOff(int channel) {
        if (m_dsp_poly) {
            m_dsp_poly->ctrlChange(channel, m_dsp_poly->ALL_NOTES_OFF, 0);
        }
    }
    
    void setAssetsDir(const string & assetsDir) {
        m_assetsDirPath = assetsDir;
    }
    
    void setLibrariesDir(const string & librariesDir) {
        m_faustLibrariesPath = librariesDir;
    }
    
    void setGroupVoices(bool groupVoices) {
        m_groupVoices = groupVoices;
    }
    
    void setDynamicVoices(bool dynamicVoices) {
        m_dynamicVoices = dynamicVoices;
    }
    
    void panic() {
        sendAllNotesOff(0);
    }
    
    void pitchWheel(int channel, int wheel) {
        if (m_dsp_poly) {
            m_dsp_poly->pitchWheel(channel, wheel);
        }
    }

    void progChange(int channel, int pgm) {
        if (m_dsp_poly) {
            m_dsp_poly->progChange(channel, pgm);
        }
    }

    void ctrlChange(int channel, int ctrl, int value) {
        if (m_dsp_poly) {
            m_dsp_poly->ctrlChange(channel, ctrl, value);
        }
    }
    
    // clear
    void clear()
    {
        // todo: do something with m_midi_handler
        if (m_dsp_poly) {
            m_midi_handler.removeMidiIn(m_dsp_poly);
            m_midi_handler.stopMidi();
        }
        if (m_midi_ui) {
            m_midi_ui->removeMidiIn(m_dsp_poly);
            m_midi_ui->stop();
        }

        CK_SAFE_DELETE(m_dsp);
        CK_SAFE_DELETE(m_ui);
        CK_SAFE_DELETE(m_dsp_poly);
        CK_SAFE_DELETE(m_midi_ui);
        CK_SAFE_DELETE(m_soundUI);
        
        deleteDSPFactory(m_factory); m_factory = NULL;
        CK_SAFE_DELETE(m_poly_factory);
        
        clearMIDI();
    }
    
    void clearMIDI() {
        if (m_dsp_poly) {
            m_dsp_poly->instanceClear();
        }
    }
    
    // clear
    void clearBufs()
    {
        if( m_input != NULL )
        {
            for( int i = 0; i < m_numInputChannels; i++ )
                CK_SAFE_DELETE_ARRAY(m_input[i]);
        }
        if( m_output != NULL )
        {
            for( int i = 0; i < m_numOutputChannels; i++ )
                CK_SAFE_DELETE_ARRAY(m_output[i]);
        }
        CK_SAFE_DELETE_ARRAY(m_input);
        CK_SAFE_DELETE_ARRAY(m_output);
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
        int argc = 0;
        const char** argv = new const char* [128];
        
        if (m_faustLibrariesPath != "") {
            argv[argc++] = "--import-dir";
            argv[argc++] = m_faustLibrariesPath.c_str();
        }

        auto pathToFaustLibraries = getPathToFaustLibraries();

        argv[argc++] = "-I";
        argv[argc++] = pathToFaustLibraries.c_str();

        //argv[argc++] = "-vec";
        //argv[argc++] = "-vs";
        //argv[argc++] = "128";
        //argv[argc++] = "-dfs";
        
        // save
        m_code = code;
        
        // auto import
        string theCode = m_autoImport + "\n" + code;
        
        // optimization level
        const int optimize = -1;
        
#if __APPLE__
    string target = getDSPMachineTarget();
#else
    string target = string("");
#endif
        
        const bool polyphonyIsOn = m_nvoices > 0;
        
        if (polyphonyIsOn) {
            // create new factory
            m_poly_factory = createPolyDSPFactoryFromString("chuck", theCode,
                argc, argv, target.c_str(), m_errorString, optimize);
        } else {
            // create new factory
            m_factory = createDSPFactoryFromString("chuck", theCode,
                argc, argv, target.c_str(), m_errorString, optimize );
        }
        
        if (argv) {
            for (int i = 0; i < argc; i++) {
                argv[i] = NULL;
            }
            argv = NULL;
        }

        // check for error
        if( m_errorString != "")
        {
            // output error
            cerr << "[Faust]: " << m_errorString << endl;
            // clear
            clear();
            // done
            return false;
        }
        
        //// print where faustlib is looking for stdfaust.lib and the other lib files.
        //auto pathnames = m_factory->getIncludePathnames();
        //cout << "pathnames:\n" << endl;
        //for (auto name : pathnames) {
        //    cout << name << "\n" << endl;
        //}
        //cout << "library list:\n" << endl;
        //auto librarylist = m_factory->getLibraryList();
        //for (auto name : librarylist) {
        //    cout << name << "\n" << endl;
        //}

    #if __APPLE__
        if (m_midi_enable) {
            // Only macOS can support virtual MIDI in.
            // Use case: you want to send MIDI programmatically to Faust from some other software/algorithm, not midi hardware
            m_midi_handler = rt_midi(m_midi_virtual_name, m_midi_virtual);
        }
    #endif

        if (polyphonyIsOn) {
            m_dsp_poly = m_poly_factory->createPolyDSPInstance(m_nvoices, m_dynamicVoices, m_groupVoices);
            if (!m_dsp_poly) {
                cerr << "[Faust]: Cannot create Poly DSP instance." << endl;
                return false;
            }
            if (m_midi_enable) {
                m_midi_handler.addMidiIn(m_dsp_poly);
            }
        }
        else {
            // create DSP instance
            m_dsp = m_factory->createDSPInstance();
            if (!m_dsp) {
                cerr << "[Faust]: Cannot create DSP instance." << endl;
                return false;
            }
        }
        
        dsp* theDsp = polyphonyIsOn ? m_dsp_poly : m_dsp;

        // make new UI
        if (m_midi_enable)
        {
            m_midi_ui = new MidiUI(&m_midi_handler);
            theDsp->buildUserInterface(m_midi_ui);
        }

        // make new UI
        m_ui = new FauckUI();
        // build ui
        theDsp->buildUserInterface( m_ui );
        // build sound ui
        if (m_assetsDirPath != "") {
            m_soundUI = new SoundUI(m_assetsDirPath.c_str(), m_srate);
            theDsp->buildUserInterface(m_soundUI);
        }

        // get channels
        int inputs = theDsp->getNumInputs();
        int outputs = theDsp->getNumOutputs();

        // see if we need to alloc
        if (inputs != m_numInputChannels || outputs != m_numOutputChannels) {
            // clear and allocate
            allocate(inputs, outputs);
        }

        // init
        theDsp->init((int)(m_srate + .5));

        if (m_midi_enable) {
            m_midi_ui->run();
        }

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
        for( string line; getline( fin, line ); )
            m_code += line + '\n';
        
        // eval it
        return eval( m_code );
    }
    
    // dump (snapshot)
    void dump()
    {
        if(m_errorString.empty()){
        cerr << "---------------- DUMPING [Faust] PARAMETERS ---------------" << endl;
        m_ui->dumpParams();
        cerr << "Number of Inputs: " << m_numInputChannels << endl ;
        cerr << "Number of Outputs: " << m_numOutputChannels << endl ;
        cerr << "-----------------------------------------------------------" << endl;
        } else{
        cerr << "[Faust]: "<< m_errorString <<endl;
        }
    }
    
    void tick( SAMPLE * in, SAMPLE * out, int nframes ){
        
        const bool polyphonyIsOn = m_nvoices > 0;
        dsp* theDsp = polyphonyIsOn ? m_dsp_poly : m_dsp;
        
        if (!theDsp) {
            // write zeros and return
            for (int f = 0; f < nframes; f++)
            {
                for (int chan = 0; chan < m_numOutputChannels; chan++)
                {
                    out[f*m_numOutputChannels+chan] = 0;
                }
            }
            return;
        }
        
        bool needGuiMutex = m_nvoices > 0 && polyphonyIsOn && m_groupVoices;
                
        // If polyphony is enabled and we're grouping voices,
        // several voices might share the same parameters in a group.
        // Therefore we have to call updateAllGuis to update all dependent parameters.
        if (needGuiMutex) {
            if (m_guiUpdateMutex.Lock()) {
                // Have Faust update all GUIs.
                GUI::updateAllGuis();

                m_guiUpdateMutex.Unlock();
            }
        }
        
        for(int f = 0; f < nframes; f++)
        {
            for(int c = 0; c < m_numInputChannels; c++)
            {
                m_input[c][0] = in[f*m_numInputChannels+c];
            }
            
            theDsp->compute( 1, m_input, m_output );
            
            for(int c = 0; c < m_numOutputChannels; c++)
            {
                out[f*m_numOutputChannels+c] = m_output[c][0];
            }
        }
    }
    
    // for Chugins extending UGen
    /*
    SAMPLE tick( SAMPLE in )
    {
        // sanity check
        dsp* theDsp = polyphonyIsOn ? m_dsp_poly : m_dsp;

        if( theDsp == NULL ) return 0;
        
        // set input
        for( int i = 0; i < m_numInputChannels; i++ ) m_input[i][0] = in;
        // zero output
        for( int i = 0; i < m_numOutputChannels; i++ ) m_output[i][0] = 0;
        // compute samples
        theDsp->compute( 1, m_input, m_output );
        // average output
        t_CKFLOAT avg = 0;
        for( int i = 0; i < m_numOutputChannels; i++ ) avg += m_output[i][0];
        // return sample
        return avg / m_numOutputChannels;
    }
    */

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
    llvm_dsp_poly_factory* m_poly_factory;
    // faust DSP object
    dsp * m_dsp;
    dsp_poly* m_dsp_poly;
    // faust compiler error string
    string m_errorString;
    // auto import
    string m_autoImport;
    string m_faustLibrariesPath;
    string m_assetsDirPath;
    
    // faust input buffer
    FAUSTFLOAT ** m_input;
    FAUSTFLOAT ** m_output;
    
    // input and output
    int m_numInputChannels;
    int m_numOutputChannels;
    
    // UI
    FauckUI * m_ui;
    MidiUI* m_midi_ui = nullptr;
    SoundUI* m_soundUI = nullptr;
    
    TMutex m_guiUpdateMutex;

    bool m_groupVoices = true;
    bool m_dynamicVoices = true;
    int m_nvoices = 0;

    bool m_midi_enable = false;
    bool m_midi_virtual = false;
    string m_midi_virtual_name = string("");
    rt_midi m_midi_handler;
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
    
    // NOTE: if this is to be a UGen with more than 1 channel, 
    // e.g., a multichannel UGen -- will need to use add_ugen_funcf()
    // and declare a tickf function using CK_DLL_TICKF

    // add .eval()
    QUERY->add_mfun(QUERY, faust_eval, "int", "eval");
    // add argument
    QUERY->add_arg(QUERY, "string", "code");
    
    // add .compile()
    QUERY->add_mfun(QUERY, faust_compile, "int", "compile");
    // add argument
    QUERY->add_arg(QUERY, "string", "path");

    // add .v()
    QUERY->add_mfun(QUERY, faust_v_set, "float", "v");
    // add arguments
    QUERY->add_arg(QUERY, "string", "key");
    QUERY->add_arg(QUERY, "float", "value");

    // add .v()
    QUERY->add_mfun(QUERY, faust_v_get, "float", "v");
    // add argument
    QUERY->add_arg(QUERY, "string", "key");
    
    // add .nvoices()
    QUERY->add_mfun(QUERY, faust_nvoices_set, "int", "numVoices");
    // add arguments
    QUERY->add_arg(QUERY, "int", "value");

    // add .nvoices()
    QUERY->add_mfun(QUERY, faust_nvoices_get, "int", "numVoices");
    
    // add .noteOn()
    QUERY->add_mfun(QUERY, faust_noteon, "int", "noteOn");
    // add arguments
    QUERY->add_arg(QUERY, "int", "pitch");
    QUERY->add_arg(QUERY, "int", "velocity");
    
    // add .noteOff()
    QUERY->add_mfun(QUERY, faust_noteoff, "int", "noteOff");
    // add arguments
    QUERY->add_arg(QUERY, "int", "pitch");
    QUERY->add_arg(QUERY, "int", "velocity");
    
    // add .pitchWheel()
    QUERY->add_mfun(QUERY, faust_pitchwheel, "int", "pitchWheel");
    // add arguments
    QUERY->add_arg(QUERY, "int", "channel");
    QUERY->add_arg(QUERY, "int", "wheel");
    
    // add .progChange()
    QUERY->add_mfun(QUERY, faust_progchange, "int", "progChange");
    // add arguments
    QUERY->add_arg(QUERY, "int", "channel");
    QUERY->add_arg(QUERY, "int", "pgm");
    
    // add .ctrlChange()
    QUERY->add_mfun(QUERY, faust_ctrlchange, "int", "ctrlChange");
    // add arguments
    QUERY->add_arg(QUERY, "int", "channel");
    QUERY->add_arg(QUERY, "int", "ctrl");
    QUERY->add_arg(QUERY, "int", "value");
    
    // add .assetsDir()
    QUERY->add_mfun(QUERY, faust_assets_set, "int", "assetsDir");
    // add arguments
    QUERY->add_arg(QUERY, "string", "assetsDir");
    
    // add .librariesDir()
    QUERY->add_mfun(QUERY, faust_libraries_set, "int", "librariesDir");
    // add arguments
    QUERY->add_arg(QUERY, "string", "librariesDir");
    
    // add .groupVoices()
    QUERY->add_mfun(QUERY, faust_groupvoices_set, "int", "groupVoices");
    // add arguments
    QUERY->add_arg(QUERY, "int", "groupVoices");
    
    // add .dynamicVoices()
    QUERY->add_mfun(QUERY, faust_dynamicvoices_set, "int", "dynamicVoices");
    // add arguments
    QUERY->add_arg(QUERY, "int", "dynamicVoices");
    
    // add .panic()
    QUERY->add_mfun(QUERY, faust_panic, "void", "panic");

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
    Faust * f_obj = new Faust(API->vm->get_srate(API, SHRED));
    
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
    string code = GET_NEXT_STRING_SAFE(ARGS);
    // eval it
    RETURN->v_int = f->eval( code );
}

CK_DLL_MFUN(faust_compile)
{
    // get our c++ class pointer
    Faust * f = (Faust *) OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get argument
    string code = GET_NEXT_STRING_SAFE(ARGS);
    // eval it
    RETURN->v_int = f->compile( code );
}

CK_DLL_MFUN(faust_v_set)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get name
    string name = GET_NEXT_STRING_SAFE(ARGS);
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
    string name = GET_NEXT_STRING_SAFE(ARGS);
    // call it
    RETURN->v_float = f->getParam( name );
}

CK_DLL_MFUN(faust_nvoices_set)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get value
    t_CKINT v = GET_NEXT_INT(ARGS);
    // call it
    v = f->setNVoices( v );
    // return it
    RETURN->v_int = v;
}

CK_DLL_MFUN(faust_nvoices_get)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // call it
    RETURN->v_int = f->getNVoices();
}

CK_DLL_MFUN(faust_noteon)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get value
    t_CKINT pitch = GET_NEXT_INT(ARGS);
    t_CKINT velocity = GET_NEXT_INT(ARGS);
    // call it
    f->noteOn(pitch, velocity);
    // return it
    RETURN->v_int = 1;
}

CK_DLL_MFUN(faust_noteoff)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get value
    t_CKINT pitch = GET_NEXT_INT(ARGS);
    t_CKINT velocity = GET_NEXT_INT(ARGS);
    // call it
    f->noteOff(pitch, velocity);
    // return it
    RETURN->v_int = 1;
}

CK_DLL_MFUN(faust_pitchwheel)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get value
    t_CKINT channel = GET_NEXT_INT(ARGS);
    t_CKINT wheel = GET_NEXT_INT(ARGS);
    // call it
    f->pitchWheel(channel, wheel);
    // return it
    RETURN->v_int = 1;
}

CK_DLL_MFUN(faust_progchange)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get value
    t_CKINT channel = GET_NEXT_INT(ARGS);
    t_CKINT pgm = GET_NEXT_INT(ARGS);
    // call it
    f->progChange(channel, pgm);
    // return it
    RETURN->v_int = 1;
}

CK_DLL_MFUN(faust_ctrlchange)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get value
    t_CKINT channel = GET_NEXT_INT(ARGS);
    t_CKINT ctrl = GET_NEXT_INT(ARGS);
    t_CKINT value = GET_NEXT_INT(ARGS);
    // call it
    f->ctrlChange(channel, ctrl, value);
    // return it
    RETURN->v_int = 1;
}

CK_DLL_MFUN(faust_assets_set)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get value
    string v = GET_NEXT_STRING_SAFE(ARGS);
    // call it
    f->setAssetsDir( v );
    // return it
    RETURN->v_int = 1;
}

CK_DLL_MFUN(faust_libraries_set)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get value
    string v = GET_NEXT_STRING_SAFE(ARGS);
    // call it
    f->setLibrariesDir( v );
    // return it
    RETURN->v_int = 1;
}

CK_DLL_MFUN(faust_groupvoices_set)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get value
    bool v = GET_NEXT_INT(ARGS);
    // call it
    f->setGroupVoices( v );
    // return it
    RETURN->v_int = 1;
}

CK_DLL_MFUN(faust_dynamicvoices_set)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // get value
    bool v = GET_NEXT_INT(ARGS);
    // call it
    f->setDynamicVoices( v );
    // return it
    RETURN->v_int = 1;
}

CK_DLL_MFUN(faust_panic)
{
    // get our c++ class pointer
    Faust * f = (Faust *)OBJ_MEMBER_INT(SELF, faust_data_offset);
    // call it
    f->panic();
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
