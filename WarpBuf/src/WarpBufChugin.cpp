#include "WarpBufChugin.h"

WarpBufChugin::WarpBufChugin(t_CKFLOAT srate)
{
    // sample rate
    m_srate = srate;
    memset(&sfinfo, 0, sizeof(SF_INFO));

    using namespace RubberBand;

    RubberBandStretcher::Options options = 0;

    //options |= RubberBandStretcher::OptionProcessOffline;
    options |= RubberBandStretcher::OptionProcessRealTime;  // NOT the default

    //options |= RubberBandStretcher::OptionStretchElastic;
    options |= RubberBandStretcher::OptionStretchPrecise;  // NOT the default

    //options |= RubberBandStretcher::OptionTransientsCrisp;
    //options |= RubberBandStretcher::OptionTransientsMixed;
    options |= RubberBandStretcher::OptionTransientsSmooth;  // NOT the default

    options |= RubberBandStretcher::OptionDetectorCompound;
    //options |= RubberBandStretcher::OptionDetectorPercussive;
    //options |= RubberBandStretcher::OptionDetectorSoft;

    options |= RubberBandStretcher::OptionPhaseLaminar;
    //options |= RubberBandStretcher::OptionPhaseIndependent;

    //options |= RubberBandStretcher::OptionThreadingAuto;
    options |= RubberBandStretcher::OptionThreadingNever;  // NOT the default
    //options |= RubberBandStretcher::OptionThreadingAlways;

    options |= RubberBandStretcher::OptionWindowStandard;
    //options |= RubberBandStretcher::OptionWindowShort;
    //options |= RubberBandStretcher::OptionWindowLong;

    options |= RubberBandStretcher::OptionSmoothingOff;
    //options |= RubberBandStretcher::OptionSmoothingOn;

    options |= RubberBandStretcher::OptionFormantShifted;
    //options |= RubberBandStretcher::OptionFormantPreserved;

    //options |= RubberBandStretcher::OptionPitchHighSpeed;
    options |= RubberBandStretcher::OptionPitchHighQuality;  // NOT the default
    //options |= RubberBandStretcher::OptionPitchHighConsistency;

    //options |= RubberBandStretcher::OptionChannelsApart;
    options |= RubberBandStretcher::OptionChannelsTogether;  // NOT the default

    m_rbstretcher = std::make_unique<RubberBand::RubberBandStretcher>(
        m_srate,
        2,
        options,
        1.,
        1.);

    m_nonInterleavedBuffer = new float* [channels];
    for (int i = 0; i < channels; i++) {
        m_nonInterleavedBuffer[i] = new float[ibs];
    }
    m_interleavedBuffer = new float[channels * ibs];

}

WarpBufChugin::~WarpBufChugin()
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
                m_nonInterleavedBuffer[c][i] = m_interleavedBuffer[i * channels + c];
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

// return true if the file was read
bool WarpBufChugin::read(const string& path) {
    
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