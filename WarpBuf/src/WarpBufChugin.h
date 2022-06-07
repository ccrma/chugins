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

#define WARPBUF_MAX_OUTPUTS 16

//-----------------------------------------------------------------------------
// name: class WarpBufChugin
// desc: WarpBufChugin for time-stretching and pitch-stretching (via Rubber Band library)
//-----------------------------------------------------------------------------
class WarpBufChugin
{
public:
    // constructor
    WarpBufChugin(t_CKFLOAT srate);

    ~WarpBufChugin();

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

    int m_numAllocated = 0;
    AbletonClipInfo m_clipInfo;

    const int ibs = 1024;  // interleaved buffer size
    int m_channels = 0;
    double m_playHeadBeats = 0.; // measured in quarter notes
    bool m_play = true;
    double m_bpm = 120.;  // desired playback bpm (not the source bpm)

    void clearBufs();
    void allocate(int numChannels, int numSamples);
    void resetStretcher();
};