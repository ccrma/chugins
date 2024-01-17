// this should align with the correct versions of these ChucK files
#include <chugin.h>

// general includes
#include <stdio.h>
#include <limits.h>
#include <cmath>

// STL includes
#include <iostream>
#include <string>

#include <sndfile.h>

#include <rubberband/RubberBandStretcher.h>
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

    void reset();

    double getPlayhead();
    void setPlayhead(double playhead);
    double getTranspose();
    void setTranspose(double transpose);
    double getBPM();
    void setBPM(double bpm);

    bool read(const std::string& filename);

    bool getPlay() { return m_play; };
    void setPlay(bool play) { m_play = play; };

    double getStartMarker() { return m_clipInfo.start_marker; }
    void setStartMarker(double startMarker) { m_clipInfo.start_marker = startMarker; }
    double getEndMarker() { return m_clipInfo.end_marker; }
    void setEndMarker(double endMarker) { m_clipInfo.end_marker = endMarker; }
    double getLoopStart() { return m_clipInfo.loop_start; }
    void setLoopStart(double loopStart) { m_clipInfo.loop_start = loopStart; }
    double getLoopEnd() { return m_clipInfo.loop_end; }
    void setLoopEnd(double loopEnd) { m_clipInfo.loop_end = loopEnd; }
    bool getLoopEnable() { return m_clipInfo.loop_on; }
    void setLoopEnable(bool enable) { m_clipInfo.loop_on = enable; }

private:
    // sample rate
    t_CKFLOAT m_srate;

    std::unique_ptr<RubberBand::RubberBandStretcher> m_rbstretcher;

    // buffer related vars:
    const int interleaved_buffer_size = 1024;
    int m_numAllocated = 0;  // keep track of allocated samples
    int m_channels = 0;  // keep track of allocated channels
    float** m_retrieveBuffer = NULL; // non interleaved: [m_channels][nframes] where nframes is the dynamic number of frames chuck requests on tick
    float* m_interleavedBuffer = nullptr;  // interleaved: [m_channels*interleaved_buffer_size]
    float** m_nonInterleavedBuffer = NULL;  // non interleaved: [m_channels][interleaved_buffer_size]

    // soundfile vars:
    SNDFILE* sndfile;
    SF_INFO sfinfo;
    int sfReadPos = 0;

    AbletonClipInfo m_clipInfo;

    double m_playHeadBeats = 0.; // measured in quarter notes
    bool m_play = true;
    double m_bpm = 120.;  // desired playback bpm (not the source bpm)

    void clearBufs();
    void allocate(int numChannels, int numSamples);
    void recreateStretcher();
};
