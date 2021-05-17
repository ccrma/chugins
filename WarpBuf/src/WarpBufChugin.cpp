
// this should align with the correct versions of these ChucK files
#include "chuck_dl.h"
#include "chuck_def.h"
#include "chuck_instr.h"
#include "chuck_vm.h"

// general includes
#include <stdio.h>
#include <limits.h>

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

#include "portable_endian.h"

class ClipInfo {
    public:
        double loop_start;
        double loop_end;
        double sample_offset;
        double hidden_loop_start;
        double hidden_loop_end;
        double out_marker;
        bool loop_on = false;
        bool warp_on = false;
        double bpm = 120.;

        std::vector<std::pair<double, double>> warp_markers;

        void beat_to_seconds(double beat, float& seconds, float &bpm) {

            if (warp_markers.size() < 2) {
                std::cerr << "unable to find sample for beat. not enough markers. " << std::endl;
                return;
            }

            auto it = warp_markers;

            double p1, b1, p2, b2;

            p1 = warp_markers.at(0).first;
            b1 = warp_markers.at(0).second;
            
            for (auto it = ++warp_markers.begin(); it != warp_markers.end(); it++) {
                if ((*it).second >= beat) {

                    p2 = (*it).first;
                    b2 = (*it).second;

                    bpm = (b2 - b1) / (p2 - p1) * 60.0;
         
                    // interpolate between the two warp markers
                    float x = (beat - b1) / (b2 - b1);
                    
                    seconds = p1 + x * (p2 - p1);
                    return;
                }
                else {
                    p1 = (*it).first;
                    b1 = (*it).second;
                }
            }

            int last_index = warp_markers.size() - 1;
            p1 = warp_markers.at(last_index-1).first;
            b1 = warp_markers.at(last_index-1).second;
            p2 = warp_markers.at(last_index).first;
            b2 = warp_markers.at(last_index).second;

            bpm = (b2 - b1) / (p2 - p1) * 60.0;

            // interpolate between the two warp markers
            float x = (beat - b1) / (b2 - b1);

            seconds = p1 + x * (p2 - p1);
            return;

            std::cerr << "unable to find sample for beat: " << beat << std::endl;
            return;

        }

        int read_warp_marker(FILE* f, double* pos, double* beat) {
            return
                find_str(f, "WarpMarker") &&
                !fseek(f, 4, SEEK_CUR) &&
                read_double(f, pos) &&
                read_double(f, beat);
        }

        int read_loop_info(FILE* f) {
            return
                find_str(f, "SampleOverViewLevel") &&
                find_str(f, "SampleOverViewLevel") &&
                !fseek(f, 71, SEEK_CUR) &&
                read_double(f, &loop_start) &&
                read_double(f, &loop_end) &&
                read_double(f, &sample_offset) &&
                read_double(f, &hidden_loop_start) &&
                read_double(f, &hidden_loop_end) &&
                read_double(f, &out_marker) &&
                !fseek(f, 281, SEEK_CUR) &&
                read_bool(f, &loop_on)
                ;
        }

        void reset() {
            warp_markers.clear();
            warp_on = false;
            bpm = 120.;
        }

        void readWarpFile(const string& path) {

            reset();

            FILE* f = fopen(path.c_str(), "rb");
            if (!f) {
                std::cerr << "Error: Couldn't open file at path: " << path.c_str() << std::endl;
                return;
            }

            if (!read_loop_info(f)) {
                printf("Error: Couldn't find loop markers.\n");
            }
            else {
                //printf("loop_start: %.17g loop_end: %.17g sample_offset: %.17g hidden_loop_start: %.17g hidden_loop_end: %.17g out_marker: %.17g\n",
                //    loop_start, loop_end, sample_offset, hidden_loop_start, hidden_loop_end, out_marker);
            }
            rewind(f);

            double pos, beat;
            
            bool found_one = false;

            // the first appearance of "WarpMarker" isn't meaningful.
            find_str(f, "WarpMarker");

            long last_good_marker = 0;
            // Subsequent "WarpMarkers" are meaningful
            while (find_str(f, "WarpMarker")) {
                if (!fseek(f, 4, SEEK_CUR) &&
                    read_double(f, &pos) &&
                    read_double(f, &beat)) {

                    found_one = true;
                    warp_markers.push_back(std::make_pair(pos, beat));

                    last_good_marker = ftell(f);
                }
                else if (found_one) {
                    break;
                }
            }
            
            if (!fseek(f, last_good_marker, SEEK_SET) && !fseek(f, 7, SEEK_CUR) && read_bool(f, &loop_on)) {
                // Then we read the bool for loop_on
            }
            else {
                // Then we couldn't get to the byte for loop_on
            }

            if (warp_markers.size() > 1) {
                double p1 = warp_markers.at(0).first;
                double b1 = warp_markers.at(0).second;
                double p2 = warp_markers.at(1).first;
                double b2 = warp_markers.at(1).second;

                bpm = (b2 - b1) / (p2 - p1) * 60.0;
                //printf("BPM: %.17g\n", bpm);

                warp_on = true;
            }
            else {
                std::cout << "Error: Num warp markers is " << warp_markers.size() << "." << std::endl;
            }
        }
    private:

        int rot(int head, int size) {
            if (++head >= size) return head - size;
            return head;
        }
        int find_str(FILE* f, const char* string) {
            const int size = strlen(string);
            //char buffer[size];
            char* buffer = (char*)malloc(size * sizeof(char));
            if (fread(buffer, 1, size, f) != size) return 0;
            int head = 0, c;

            for (;;) {
                int rHead = head;
                for (int i = 0; i < size; i++) {
                    if (buffer[rHead] != string[i]) goto next;
                    rHead = rot(rHead, size);
                }
                return 1;

            next:
                if ((c = getc(f)) == EOF) break;
                buffer[head] = c;
                head = rot(head, size);
            }

            return 0;
        }

        int read_double(FILE* f, double* x) {
            if (fread(x, 1, 8, f) != 8) return 0;
            *(uint64_t*)x = le64toh(*(uint64_t*)x);
            return 1;
        }

        int read_bool(FILE* f, bool* b) {
            if (fread(b, 1, 1, f) != 1) return 0;
            return 1;
        }
};

// declaration of chugin constructor
CK_DLL_CTOR(warpbuf_ctor);
// declaration of chugin desctructor
CK_DLL_DTOR(warpbuf_dtor);

// functions
CK_DLL_MFUN(warpbuf_read);

CK_DLL_MFUN(warpbuf_getplay);
CK_DLL_MFUN(warpbuf_setplay);
CK_DLL_MFUN(warpbuf_getbpm);
CK_DLL_MFUN(warpbuf_setbpm);
CK_DLL_MFUN(warpbuf_gettranspose);
CK_DLL_MFUN(warpbuf_settranspose);
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
        delete myChuckString;

        clearBufs();
        delete[] m_interleavedBuffer;

        if (m_nonInterleavedBuffer != NULL)
        {
            for (int i = 0; i < channels; i++)
                SAFE_DELETE_ARRAY(m_nonInterleavedBuffer[i]);
        }
        SAFE_DELETE_ARRAY(m_nonInterleavedBuffer);

        m_rbstretcher.release();
    }


    // for Chugins extending UGen
    void tick(SAMPLE* in, SAMPLE* out, int nframes);

    double getTranspose();
    void setTranspose(double transpose);
    double getBPM();
    void setBPM(double bpm);
    bool getLoopEnable();
    void setLoopEnable(bool enable);

    bool read(const string& filename);
    const int ibs = 1024;
    const int channels = 2;

    double m_playHeadBeats = 0.;

    double m_bpm = 120.;

    bool m_play = true;
    bool getPlay() { return m_play; };
    void setPlay(bool play) { m_play = play; };

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

    int numAllocated = 0;
    ClipInfo m_clipInfo;

    void clearBufs();
    void allocate(int numSamples);

protected:

    Chuck_String* myChuckString = new Chuck_String("");

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
        for (int i = 0; i < channels; i++)
            SAFE_DELETE_ARRAY(m_retrieveBuffer[i]);
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
    for (int i = 0; i < 2; i++)
    {
        // single sample for each
        m_retrieveBuffer[i] = new float[numSamples];
    }
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
    allocate(nframes);

    float _;
    float instant_bpm = -1.;

    m_clipInfo.beat_to_seconds(m_playHeadBeats, _, instant_bpm);

    float loop_start_seconds = 0.;
    m_clipInfo.beat_to_seconds(m_clipInfo.loop_start, loop_start_seconds, _);

    float loop_end_seconds = 0.;
    m_clipInfo.beat_to_seconds(m_clipInfo.loop_end, loop_end_seconds, _);

    bool past_loop_end_and_loop_off = m_playHeadBeats > m_clipInfo.loop_end && !m_clipInfo.loop_on;
    if (past_loop_end_and_loop_off || !m_play) {
        // write zeros
        for (int chan = 0; chan < channels; chan++) {
            auto chanPtr = m_retrieveBuffer[chan];
            for (int i = 0; i < nframes; i++)
            {
                out[chan + 2 * i] = 0.;
            }
        }
        return;
    }

    m_playHeadBeats += m_bpm * (double)nframes / (60. * m_srate);

    double ratio = (m_srate / sfinfo.samplerate);
    if (instant_bpm > 0 && m_clipInfo.warp_on) {
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
                m_playHeadBeats = m_clipInfo.loop_start;
                sfReadPos = loop_start_seconds * sfinfo.samplerate;
                sf_seek(sndfile, sfReadPos, SEEK_SET);

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
        for (int i = 0; i < nframes; i++)
        {
            out[chan + 2 * i] = *chanPtr++;
        }
    }
}

bool WarpBufChugin::read(const string& path) {
    memset(&sfinfo, 0, sizeof(SF_INFO));
    m_clipInfo.readWarpFile(path + std::string(".asd"));

    m_playHeadBeats = m_clipInfo.loop_start + m_clipInfo.sample_offset;

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

    float start_seconds;
    float _;
    m_clipInfo.beat_to_seconds(m_playHeadBeats, start_seconds, _);

    sf_seek(sndfile, start_seconds*sfinfo.samplerate, SEEK_SET);

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

    QUERY->add_mfun(QUERY, warpbuf_getbpm, "int", "bpm");
    QUERY->add_mfun(QUERY, warpbuf_setbpm, "int", "bpm");
    QUERY->add_arg(QUERY, "float", "bpm");

    QUERY->add_mfun(QUERY, warpbuf_gettranspose, "int", "transpose");
    QUERY->add_mfun(QUERY, warpbuf_settranspose, "int", "transpose");
    QUERY->add_arg(QUERY, "float", "transpose");

    QUERY->add_mfun(QUERY, warpbuf_getloopenable, "int", "loop");
    QUERY->add_mfun(QUERY, warpbuf_setloopenable, "int", "loop");
    QUERY->add_arg(QUERY, "int", "enable");

    QUERY->add_mfun(QUERY, warpbuf_getloopstart, "int", "loopStart");
    QUERY->add_mfun(QUERY, warpbuf_setloopstart, "int", "loopStart");
    QUERY->add_arg(QUERY, "float", "loopStart");

    QUERY->add_mfun(QUERY, warpbuf_getloopend, "int", "loopEnd");
    QUERY->add_mfun(QUERY, warpbuf_setloopend, "int", "loopEnd");
    QUERY->add_arg(QUERY, "float", "loopEnd");

    // this reserves a variable in the ChucK internal class to store
    // referene to the c++ class we defined above
    warpbuf_data_offset = QUERY->add_mvar(QUERY, "int", "@b_data", false);

    // end the class definition
    // IMPORTANT: this MUST be called!
    QUERY->end_class(QUERY);

    // wasn't that a breeze?
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
    WarpBufChugin * b_obj = (WarpBufChugin *) OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    // check it
    if( b_obj )
    {
        // clean up
        delete b_obj;
        OBJ_MEMBER_INT(SELF, warpbuf_data_offset) = 0;
        b_obj = NULL;
    }
}

// implementation for tick function
CK_DLL_TICKF(warpbuf_tick)
{
    // get our c++ class pointer
    WarpBufChugin* b = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    // invoke our tick function; store in the magical out variable
    if (b) b->tick(in, out, nframes);

    // yes
    return TRUE;
}

CK_DLL_MFUN(warpbuf_getplay)
{
    WarpBufChugin* b = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_int = b->getPlay();
}

CK_DLL_MFUN(warpbuf_setplay)
{
    bool play = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* b = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    b->setPlay(play);
    RETURN->v_int = true;
}

CK_DLL_MFUN(warpbuf_read)
{
    string filename = GET_NEXT_STRING_SAFE(ARGS);

    WarpBufChugin* b = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    
    RETURN->v_int = b->read(filename.c_str());
}

CK_DLL_MFUN(warpbuf_getbpm)
{
    WarpBufChugin* b = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    
    RETURN->v_float = b->getBPM();
}

CK_DLL_MFUN(warpbuf_setbpm)
{
    t_CKFLOAT bpm = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* b = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    b->setBPM(bpm);
    RETURN->v_int = true;
}

CK_DLL_MFUN(warpbuf_gettranspose)
{
    WarpBufChugin* b = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    
    RETURN->v_float = b->getTranspose();
}

CK_DLL_MFUN(warpbuf_settranspose)
{
    t_CKFLOAT transpose = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* b = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    b->setTranspose(transpose);
    RETURN->v_int = true;
}

CK_DLL_MFUN(warpbuf_getloopenable)
{

    WarpBufChugin* b = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    
    RETURN->v_int = b->getLoopEnable();
}

CK_DLL_MFUN(warpbuf_setloopenable)
{
    t_CKBOOL enable = GET_NEXT_INT(ARGS);

    WarpBufChugin* b = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    b->setLoopEnable(enable);
    RETURN->v_int = true;
}

CK_DLL_MFUN(warpbuf_getloopstart)
{
    WarpBufChugin* b = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    
    RETURN->v_float = b->getLoopStart();
}

CK_DLL_MFUN(warpbuf_setloopstart)
{
    t_CKFLOAT loopStart = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* b = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    b->setLoopStart(loopStart);
    RETURN->v_int = true;
}

CK_DLL_MFUN(warpbuf_getloopend)
{
    WarpBufChugin* b = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_float = b->getLoopEnd();
}

CK_DLL_MFUN(warpbuf_setloopend)
{
    t_CKFLOAT loopEnd = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* b = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    b->setLoopEnd(loopEnd);
    RETURN->v_int = true;
}