#include "WarpBufChugin.h"

WarpBufChugin::WarpBufChugin(t_CKFLOAT srate)
{
    // sample rate
    m_srate = srate;
    memset(&sfinfo, 0, sizeof(SF_INFO));

    this->resetStretcher();
}

void
WarpBufChugin::resetStretcher() {

    using namespace RubberBand;

    RubberBandStretcher::Options options = 0;

    options |= RubberBandStretcher::OptionProcessRealTime;
    options |= RubberBandStretcher::OptionThreadingNever;
    options |= RubberBandStretcher::OptionPitchHighQuality;

    m_rbstretcher = std::make_unique<RubberBand::RubberBandStretcher>(
        m_srate,
        sfinfo.channels,
        options,
        1.,
        1.);

}

WarpBufChugin::~WarpBufChugin()
{
    clearBufs();
    m_rbstretcher.release();
}

// clear
void
WarpBufChugin::clearBufs()
{
    if (m_retrieveBuffer != NULL)
    {
        for (int i = 0; i < m_channels; i++) {
            SAFE_DELETE_ARRAY(m_retrieveBuffer[i]);
        }
    }
    SAFE_DELETE_ARRAY(m_retrieveBuffer);

    SAFE_DELETE_ARRAY(m_interleavedBuffer);

    if (m_nonInterleavedBuffer != NULL)
    {
        for (int i = 0; i < m_channels; i++) {
            SAFE_DELETE_ARRAY(m_nonInterleavedBuffer[i]);
        }
    }
    SAFE_DELETE_ARRAY(m_nonInterleavedBuffer);
}

// allocate
void
WarpBufChugin::allocate(int numChannels, int numSamples)
{
    if (m_numAllocated == numSamples && m_channels == numChannels) {
        return;
    }

    // clear
    clearBufs();

    m_channels = numChannels;
    m_numAllocated = numSamples;

    m_nonInterleavedBuffer = new float* [m_channels];
    for (int i = 0; i < m_channels; i++) {
        m_nonInterleavedBuffer[i] = new float[ibs];
    }
       
    m_interleavedBuffer = new float[m_channels * ibs];
    
    m_retrieveBuffer = new float * [m_channels];
    // allocate buffers for each channel
    for (int i = 0; i < 2; i++) {
        // single sample for each
        m_retrieveBuffer[i] = new float[numSamples];
    }
}

double
WarpBufChugin::getPlayhead() {
    return m_playHeadBeats;
}

void
WarpBufChugin::setPlayhead(double playhead) {

    m_playHeadBeats = playhead;

    double playhead_seconds;
    double _;
    m_clipInfo.beat_to_seconds(m_playHeadBeats, playhead_seconds, _);
    sfReadPos = playhead_seconds * sfinfo.samplerate;
    sf_seek(sndfile, sfReadPos, SEEK_SET);
    // seeking doesn't change sfReadPos
}

double
WarpBufChugin::getTranspose() {

    double scale = m_rbstretcher->getPitchScale();

    double transpose = 12. * std::log2(scale);

    return transpose;
}

void
WarpBufChugin::setTranspose(double transpose) {

    double scale = std::pow(2., transpose/12.);

    m_rbstretcher->setPitchScale(scale);
}

double
WarpBufChugin::getBPM() {
    return m_bpm;
}

void
WarpBufChugin::setBPM(double bpm) {
    if (bpm <= 0) {
        std::cerr << "Error: BPM must be positive." << std::endl;
        return;
    }
    m_bpm = bpm;
}

void
WarpBufChugin::tick(SAMPLE* in, SAMPLE* out, int nframes)
{
    allocate(sfinfo.channels, nframes);

    bool past_end_marker_and_loop_off = m_playHeadBeats > m_clipInfo.end_marker && !m_clipInfo.loop_on;
    if (m_channels==0 || sfinfo.channels == 0 || past_end_marker_and_loop_off || (!m_play)) {
        // write zeros
        for (int chan = 0; chan < WARPBUF_MAX_OUTPUTS; chan++) {
            for (int i = 0; i < nframes; i++) {
                out[chan + 2 * i] = 0.;
            }
        }
        return;
    }

    double _;
    double instant_bpm = -1.;

    m_clipInfo.beat_to_seconds(m_playHeadBeats, _, instant_bpm);

    double loop_end_seconds = 0.;
    m_clipInfo.beat_to_seconds(m_clipInfo.loop_end, loop_end_seconds, _);

    m_playHeadBeats += m_bpm * double(nframes) / (60. * m_srate);

    double ratio = (m_srate / sfinfo.samplerate);
    if (instant_bpm > 0) {
        ratio *= instant_bpm / m_bpm;
    }
    m_rbstretcher->setTimeRatio(ratio);

    int count = -1;
    int numAvailable = m_rbstretcher->available();
    int allowedReadCount = 0;
    while (numAvailable < nframes) {

        if (sfReadPos > -1 && sfReadPos < sfinfo.frames) {
            allowedReadCount = m_clipInfo.loop_on ? std::min(ibs, (int)(loop_end_seconds * sfinfo.samplerate - sfReadPos)) : ibs;
            count = sf_readf_float(sndfile, m_interleavedBuffer, allowedReadCount);
            sfReadPos += count;
        }
        else {
            count = 0;
        }

        if (count <= 0) {
            if (m_clipInfo.loop_on) {
                // we are looping, so seek to the loop start of the audio file
                setPlayhead(m_clipInfo.loop_start);
            }

            if (sfReadPos < 0 || sfReadPos >= sfinfo.frames) {
                count = 1;
                // fill with zeros
                for (int c = 0; c < m_channels; c++) {
                    for (int i = 0; i < count; i++) {
                        m_interleavedBuffer[i * m_channels + c] = 0.;
                    }
                }
            }
        }
        
        for (int c = 0; c < m_channels; ++c) {
            for (int i = 0; i < count; ++i) {
                m_nonInterleavedBuffer[c][i] = m_interleavedBuffer[i * m_channels + c];
            }
        }

        m_rbstretcher->process(m_nonInterleavedBuffer, count, false);
        numAvailable = m_rbstretcher->available();
    }

    m_rbstretcher->retrieve(m_retrieveBuffer, nframes);

    //// copy from buffer to output.
    // out needs to receive alternating left/right channels.
    for (int chan = 0; chan < m_channels; chan++) {
        auto chanPtr = m_retrieveBuffer[chan];
        for (int i = 0; i < nframes; i++) {
            out[chan + 2 * i] = *chanPtr++;
        }
    }
}

// return true if the file was read
bool
WarpBufChugin::read(const string& path) {
    
    memset(&sfinfo, 0, sizeof(SF_INFO));

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

    this->resetStretcher();

    return true;
}