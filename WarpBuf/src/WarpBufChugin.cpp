
// this should align with the correct versions of these ChucK files
#include "chuck_dl.h"
#include "chuck_def.h"
#include "chuck_instr.h"
#include "chuck_vm.h"

// general includes
#include <stdio.h>
#include <limits.h>
#include <cmath>

// STL includes
#include <iostream>
#include <string>
using namespace std;

#if defined(__CK_SNDFILE_NATIVE__)
#include <sndfile.h>
#else
#include "util_sndfile.h"
#endif

#include "rubberband/RubberBandStretcher.h"
#include "AbletonClipInfo.h"

// declaration of chugin constructor
CK_DLL_CTOR(warpbuf_ctor);
// declaration of chugin desctructor
CK_DLL_DTOR(warpbuf_dtor);

// functions
CK_DLL_MFUN(warpbuf_read);
CK_DLL_MFUN(warpbuf_reset);
CK_DLL_MFUN(warpbuf_getplayhead);
CK_DLL_MFUN(warpbuf_setplayhead);
CK_DLL_MFUN(warpbuf_getplay);
CK_DLL_MFUN(warpbuf_setplay);
CK_DLL_MFUN(warpbuf_getbpm);
CK_DLL_MFUN(warpbuf_setbpm);
CK_DLL_MFUN(warpbuf_gettranspose);
CK_DLL_MFUN(warpbuf_settranspose);
CK_DLL_MFUN(warpbuf_getstartmarker);
CK_DLL_MFUN(warpbuf_setstartmarker);
CK_DLL_MFUN(warpbuf_getendmarker);
CK_DLL_MFUN(warpbuf_setendmarker);
CK_DLL_MFUN(warpbuf_getloopenable);
CK_DLL_MFUN(warpbuf_setloopenable);
CK_DLL_MFUN(warpbuf_getloopstart);
CK_DLL_MFUN(warpbuf_setloopstart);
CK_DLL_MFUN(warpbuf_getloopend);
CK_DLL_MFUN(warpbuf_setloopend);

// multi-channel audio synthesis tick function
CK_DLL_TICKF(warpbuf_tick);

// this is a special offset reserved for Chugin internal data
t_CKINT warpbuf_data_offset = 0;

//-----------------------------------------------------------------------------
// name: class WarpBufChugin
// desc: WarpBufChugin for time-stretching and pitch-stretching (via Rubberband library)
//-----------------------------------------------------------------------------
class WarpBufChugin
{
public:
    // constructor
    WarpBufChugin(t_CKFLOAT srate)
    {
        // sample rate
        m_srate = srate;
        memset(&sfinfo, 0, sizeof(SF_INFO));

        using namespace RubberBand;

        RubberBandStretcher::Options options = 0;
        options |= RubberBandStretcher::OptionProcessRealTime;
        options |= RubberBandStretcher::OptionStretchPrecise;
        //options |= RubberBandStretcher::OptionPhaseIndependent;
        //options |= RubberBandStretcher::OptionWindowLong;
        //options |= RubberBandStretcher::OptionWindowShort;
        //options |= RubberBandStretcher::OptionSmoothingOn;
        //options |= RubberBandStretcher::OptionFormantPreserved;
        options |= RubberBandStretcher::OptionPitchHighQuality;
        //options |= RubberBandStretcher::OptionChannelsTogether;

        // Pick one of these:
        options |= RubberBandStretcher::OptionThreadingAuto;
        //options |= RubberBandStretcher::OptionThreadingNever;
        //options |= RubberBandStretcher::OptionThreadingAlways;

        // Pick one of these:
        options |= RubberBandStretcher::OptionTransientsSmooth;
        //options |= RubberBandStretcher::OptionTransientsMixed;
        //options |= RubberBandStretcher::OptionTransientsCrisp;

        // Pick one of these:
        options |= RubberBandStretcher::OptionDetectorCompound;
        //options |= RubberBandStretcher::OptionDetectorPercussive;
        //options |= RubberBandStretcher::OptionDetectorSoft;

        m_rbstretcher = std::make_unique<RubberBand::RubberBandStretcher>(
            m_srate,
            2,
            options,
            1.,
            1.);

        m_nonInterleavedBuffer = new float * [channels];
        for (int i = 0; i < channels; i++) {
            m_nonInterleavedBuffer[i] = new float[ibs];
        }
        m_interleavedBuffer = new float[channels * ibs];

    }

    ~WarpBufChugin()
    {
        clearBufs();
        delete[] m_interleavedBuffer;

        if (m_nonInterleavedBuffer != NULL)
        {
            for (int i = 0; i < channels; i++) {
                SAFE_DELETE_ARRAY(m_nonInterleavedBuffer[i]);
            }
        }
        SAFE_DELETE_ARRAY(m_nonInterleavedBuffer);

        m_rbstretcher.release();
    }


    // for Chugins extending UGen
    void tick(SAMPLE* in, SAMPLE* out, int nframes);

    void reset() { m_rbstretcher->reset(); }

    double getPlayhead();
    void setPlayhead(double playhead);
    double getTranspose();
    void setTranspose(double transpose);
    double getBPM();
    void setBPM(double bpm);
    bool getLoopEnable();
    void setLoopEnable(bool enable);

    bool read(const string& filename);

    bool getPlay() { return m_play; };
    void setPlay(bool play) { m_play = play; };

    double getStartMarker() { return m_clipInfo.start_marker; }
    void setStartMarker(double startMarker) { m_clipInfo.start_marker = startMarker; }
    double getEndMarker() { return m_clipInfo.end_marker; }
    void setEndMarker(double endMarker) { m_clipInfo.end_marker = endMarker; }
    double getLoopStart() { return m_clipInfo.loop_start; }
    void setLoopStart(double loopStart) { m_clipInfo.loop_start = loopStart; }
    double getLoopEnd() { return m_clipInfo.loop_end; }
    void setLoopEnd(double LoopEnd) { m_clipInfo.loop_end = LoopEnd; }
private:
    // sample rate
    t_CKFLOAT m_srate;

    std::unique_ptr<RubberBand::RubberBandStretcher> m_rbstretcher;

    float** m_retrieveBuffer = NULL; // non interleaved: [channels][ibs]
    float* m_interleavedBuffer = nullptr;  // interleaved: [channels*ibs]
    float** m_nonInterleavedBuffer = NULL;  // non interleaved: [channels][ibs]
    SNDFILE* sndfile;
    SF_INFO sfinfo;
    int sfReadPos = 0;
    bool m_fileWasRead = false;

    int numAllocated = 0;
    AbletonClipInfo m_clipInfo;

    const int ibs = 1024;
    const int channels = 2;
    double m_playHeadBeats = 0.; // measured in quarter notes
    bool m_play = true;
    double m_bpm = 120.;  // desired playback bpm (not the source bpm)

    void clearBufs();
    void allocate(int numSamples);
};

bool WarpBufChugin::getLoopEnable() {
    return m_clipInfo.loop_end;
}

void WarpBufChugin::setLoopEnable(bool enable) {
    m_clipInfo.loop_on = enable;
}

// clear
void WarpBufChugin::clearBufs()
{
    if (m_retrieveBuffer != NULL)
    {
        for (int i = 0; i < channels; i++) {
            SAFE_DELETE_ARRAY(m_retrieveBuffer[i]);
        }
    }
    SAFE_DELETE_ARRAY(m_retrieveBuffer);
}

// allocate
void WarpBufChugin::allocate(int numSamples)
{
    if (numAllocated == numSamples) {
        return;
    }
    numAllocated = numSamples;

    // clear
    clearBufs();

    m_retrieveBuffer = new float * [channels];
    // allocate buffers for each channel
    for (int i = 0; i < 2; i++) {
        // single sample for each
        m_retrieveBuffer[i] = new float[numSamples];
    }
}

double WarpBufChugin::getPlayhead() {
    return m_playHeadBeats;
}

void WarpBufChugin::setPlayhead(double playhead) {

    m_playHeadBeats = playhead;

    double playhead_seconds;
    double _;
    m_clipInfo.beat_to_seconds(m_playHeadBeats, playhead_seconds, _);

    sfReadPos = playhead_seconds * sfinfo.samplerate;
    sf_seek(sndfile, sfReadPos, SEEK_SET);
}

double WarpBufChugin::getTranspose() {

    double scale = m_rbstretcher->getPitchScale();

    double transpose = 12. * std::log2(scale);

    return transpose;
}

void WarpBufChugin::setTranspose(double transpose) {

    float scale = std::pow(2., transpose/12.);

    m_rbstretcher->setPitchScale(scale);
}

double WarpBufChugin::getBPM() {
    return m_bpm;
}

void WarpBufChugin::setBPM(double bpm) {
    if (bpm <= 0) {
        std::cerr << "Error: BPM must be positive." << std::endl;
        return;
    }
    m_bpm = bpm;
}

void WarpBufChugin::tick(SAMPLE* in, SAMPLE* out, int nframes)
{
    bool past_end_marker_and_loop_off = m_playHeadBeats > m_clipInfo.end_marker && !m_clipInfo.loop_on;
    if (past_end_marker_and_loop_off || (!m_play) || (!m_fileWasRead)) {
        // write zeros
        for (int chan = 0; chan < channels; chan++) {
            for (int i = 0; i < nframes; i++) {
                out[chan + 2 * i] = 0.;
            }
        }
        return;
    }

    allocate(nframes);

    double _;
    double instant_bpm = -1.;

    m_clipInfo.beat_to_seconds(m_playHeadBeats, _, instant_bpm);

    double loop_end_seconds = 0.;
    m_clipInfo.beat_to_seconds(m_clipInfo.loop_end, loop_end_seconds, _);

    m_playHeadBeats += m_bpm * (double)nframes / (60. * m_srate);

    double ratio = (m_srate / sfinfo.samplerate);
    if (instant_bpm > 0) {
        ratio *= instant_bpm / m_bpm;
    }
    m_rbstretcher->setTimeRatio(ratio);

    int count = -1;
    int numAvailable = m_rbstretcher->available();
    while (numAvailable < nframes) {

        int allowedReadCount = m_clipInfo.loop_on ? std::min(ibs, (int)( loop_end_seconds*sfinfo.samplerate- sfReadPos)) : ibs;

        count = sf_readf_float(sndfile, m_interleavedBuffer, allowedReadCount);
        sfReadPos += count;
        if (count <= 0) {
            if (!m_clipInfo.loop_on) {
                // we're not looping, so just fill with zeros.
                count = ibs;
                for (size_t c = 0; c < channels; c++) {
                    for (int i = 0; i < count; i++) {
                        m_interleavedBuffer[i*channels+c] = 0.;
                    }
                }
            }
            else {
                // we are looping, so seek to the loop start of the audio file
                setPlayhead(m_clipInfo.loop_start);

                allowedReadCount = std::min(ibs, (int)(loop_end_seconds * sfinfo.samplerate - sfReadPos));
                count = sf_readf_float(sndfile, m_interleavedBuffer, allowedReadCount);
                sfReadPos += count;
            }
        }

        for (size_t c = 0; c < channels; ++c) {
            for (int i = 0; i < count; ++i) {
                float value = m_interleavedBuffer[i * channels + c];
                m_nonInterleavedBuffer[c][i] = value;
            }
        }

        m_rbstretcher->process(m_nonInterleavedBuffer, count, false);
        numAvailable = m_rbstretcher->available();
    }

    m_rbstretcher->retrieve(m_retrieveBuffer, nframes);

    //// copy from buffer to output.
    // out needs to receive alternating left/right channels.
    for (int chan = 0; chan < channels; chan++) {
        auto chanPtr = m_retrieveBuffer[chan];
        for (int i = 0; i < nframes; i++) {
            out[chan + 2 * i] = *chanPtr++;
        }
    }
}

bool WarpBufChugin::read(const string& path) {

    m_fileWasRead = false;
    
    sndfile = sf_open(path.c_str(), SFM_READ, &sfinfo);
    if (!sndfile) {
        cerr << "ERROR: Failed to open input file \"" << path << "\": "
            << sf_strerror(sndfile) << endl;
        return false;
    }

    if (sfinfo.samplerate == 0) {
        cerr << "ERROR: File lacks sample rate in header" << endl;
        return false;
    }

    m_rbstretcher->reset();

    m_fileWasRead = true;

    if (!m_clipInfo.readWarpFile((path + std::string(".asd")).c_str())) {
        // We didn't find a warp file, so assume it's 120 bpm.
        m_clipInfo.loop_start = 0.;
        m_clipInfo.hidden_loop_start = 0.;
        m_clipInfo.start_marker = 0.;
        m_clipInfo.hidden_loop_end = 120.* sfinfo.frames / (sfinfo.samplerate * 60.);
        m_clipInfo.loop_end = 120. * sfinfo.frames / (sfinfo.samplerate * 60.);
        m_clipInfo.end_marker = 120. * sfinfo.frames / (sfinfo.samplerate * 60.);
    }

    this->setPlayhead(m_clipInfo.start_marker);

    return true;
}
//-----------------------------------------------------------------------------
// query function: chuck calls this when loading the Chugin
//-----------------------------------------------------------------------------
CK_DLL_QUERY( WarpBuf )
{
    // hmm, don't change this...
    QUERY->setname(QUERY, "WarpBuf");

    // begin the class definition
    // can change the second argument to extend a different ChucK class
    QUERY->begin_class(QUERY, "WarpBuf", "UGen");

    // register the constructor (probably no need to change)
    QUERY->add_ctor(QUERY, warpbuf_ctor);
    // register the destructor (probably no need to change)
    QUERY->add_dtor(QUERY, warpbuf_dtor);

    // stereo out
    QUERY->add_ugen_funcf(QUERY, warpbuf_tick, NULL, 0, 2);

    QUERY->add_mfun(QUERY, warpbuf_getplay, "int", "play");

    QUERY->add_mfun(QUERY, warpbuf_setplay, "int", "play");
    QUERY->add_arg(QUERY, "float", "play");

    QUERY->add_mfun(QUERY, warpbuf_read, "int", "read");
    QUERY->add_arg(QUERY, "string", "filename");

    QUERY->add_mfun(QUERY, warpbuf_reset, "int", "reset");

    QUERY->add_mfun(QUERY, warpbuf_getplayhead, "float", "playhead");
    QUERY->add_mfun(QUERY, warpbuf_setplayhead, "float", "playhead");
    QUERY->add_arg(QUERY, "float", "playhead");

    QUERY->add_mfun(QUERY, warpbuf_getbpm, "float", "bpm");
    QUERY->add_mfun(QUERY, warpbuf_setbpm, "float", "bpm");
    QUERY->add_arg(QUERY, "float", "bpm");

    QUERY->add_mfun(QUERY, warpbuf_gettranspose, "float", "transpose");
    QUERY->add_mfun(QUERY, warpbuf_settranspose, "float", "transpose");
    QUERY->add_arg(QUERY, "float", "transpose");

    QUERY->add_mfun(QUERY, warpbuf_getstartmarker, "float", "startMarker");
    QUERY->add_mfun(QUERY, warpbuf_setstartmarker, "float", "startMarker");
    QUERY->add_arg(QUERY, "float", "startMarker");

    QUERY->add_mfun(QUERY, warpbuf_getendmarker, "float", "endMarker");
    QUERY->add_mfun(QUERY, warpbuf_setendmarker, "float", "endMarker");
    QUERY->add_arg(QUERY, "float", "endMarker");

    QUERY->add_mfun(QUERY, warpbuf_getloopenable, "int", "loop");
    QUERY->add_mfun(QUERY, warpbuf_setloopenable, "int", "loop");
    QUERY->add_arg(QUERY, "int", "enable");

    QUERY->add_mfun(QUERY, warpbuf_getloopstart, "float", "loopStart");
    QUERY->add_mfun(QUERY, warpbuf_setloopstart, "float", "loopStart");
    QUERY->add_arg(QUERY, "float", "loopStart");

    QUERY->add_mfun(QUERY, warpbuf_getloopend, "float", "loopEnd");
    QUERY->add_mfun(QUERY, warpbuf_setloopend, "float", "loopEnd");
    QUERY->add_arg(QUERY, "float", "loopEnd");

    // this reserves a variable in the ChucK internal class to store
    // referene to the c++ class we defined above
    warpbuf_data_offset = QUERY->add_mvar(QUERY, "int", "@b_data", false);

    // end the class definition
    // IMPORTANT: this MUST be called!
    QUERY->end_class(QUERY);

    return TRUE;
}

// implementation for the constructor
CK_DLL_CTOR(warpbuf_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, warpbuf_data_offset) = 0;

    // instantiate our internal c++ class representation
    WarpBufChugin * b_obj = new WarpBufChugin(API->vm->get_srate(API, SHRED));

    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, warpbuf_data_offset) = (t_CKINT) b_obj;
}

// implementation for the destructor
CK_DLL_DTOR(warpbuf_dtor)
{
    // get our c++ class pointer
    WarpBufChugin * chug = (WarpBufChugin *) OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    // check it
    if(chug)
    {
        // clean up
        delete chug;
        OBJ_MEMBER_INT(SELF, warpbuf_data_offset) = 0;
        chug = NULL;
    }
}

// implementation for tick function
CK_DLL_TICKF(warpbuf_tick)
{
    // get our c++ class pointer
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    // invoke our tick function; store in the magical out variable
    if (chug) chug->tick(in, out, nframes);

    // yes
    return TRUE;
}

CK_DLL_MFUN(warpbuf_reset)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->reset();
    RETURN->v_int = 1;
}

CK_DLL_MFUN(warpbuf_getplayhead)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_float = chug->getPlayhead();
}

CK_DLL_MFUN(warpbuf_setplayhead)
{
    float playhead = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setPlayhead(playhead);
    RETURN->v_float = playhead;
}

CK_DLL_MFUN(warpbuf_getplay)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_int = chug->getPlay();
}

CK_DLL_MFUN(warpbuf_setplay)
{
    bool play = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    chug->setPlay(play);
    RETURN->v_int = play;
}

CK_DLL_MFUN(warpbuf_read)
{
    string filename = GET_NEXT_STRING_SAFE(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    
    RETURN->v_int = chug->read(filename.c_str());
}

CK_DLL_MFUN(warpbuf_getbpm)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    
    RETURN->v_float = (float)chug->getBPM();
}

CK_DLL_MFUN(warpbuf_setbpm)
{
    t_CKFLOAT bpm = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setBPM(bpm);
    RETURN->v_float = bpm;
}

CK_DLL_MFUN(warpbuf_gettranspose)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    
    RETURN->v_float = (float)chug->getTranspose();
}

CK_DLL_MFUN(warpbuf_settranspose)
{
    t_CKFLOAT transpose = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setTranspose(transpose);
    RETURN->v_float = transpose;
}

CK_DLL_MFUN(warpbuf_getstartmarker)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_float = chug->getStartMarker();
}

CK_DLL_MFUN(warpbuf_setstartmarker)
{
    t_CKFLOAT startMarker = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setStartMarker(startMarker);
    RETURN->v_float = startMarker;
}

CK_DLL_MFUN(warpbuf_getendmarker)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_float = chug->getEndMarker();
}

CK_DLL_MFUN(warpbuf_setendmarker)
{
    t_CKFLOAT endMarker = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setEndMarker(endMarker);
    RETURN->v_float = endMarker;
}

CK_DLL_MFUN(warpbuf_getloopenable)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    
    RETURN->v_int = chug->getLoopEnable();
}

CK_DLL_MFUN(warpbuf_setloopenable)
{
    t_CKBOOL enable = GET_NEXT_INT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setLoopEnable(enable);
    RETURN->v_int = enable;
}

CK_DLL_MFUN(warpbuf_getloopstart)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    
    RETURN->v_float = chug->getLoopStart();
}

CK_DLL_MFUN(warpbuf_setloopstart)
{
    t_CKFLOAT loopStart = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setLoopStart(loopStart);
    RETURN->v_float = loopStart;
}

CK_DLL_MFUN(warpbuf_getloopend)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_float = chug->getLoopEnd();
}

CK_DLL_MFUN(warpbuf_setloopend)
{
    t_CKFLOAT loopEnd = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setLoopEnd(loopEnd);
    RETURN->v_float = loopEnd;
}
