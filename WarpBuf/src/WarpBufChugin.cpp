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

// allocate the buffers if the number of channels or number of samples changed
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
    for (int i = 0; i < m_channels; i++) {
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
    // If our sound file has zero channels, or for some reason the number of allocated channels (m_channels)
    // is zero, or play is disabled, or (loop is off and our playhead measured in beats is greater than
    // our end marker measured in beats)
    if (sfinfo.channels == 0 || m_channels == 0 || (!m_play) || past_end_marker_and_loop_off) {
        // Write zeros into our output buffer and return.
        // Note that we are not feeding the zeros to the stretcher.
        for (int chan = 0; chan < WARPBUF_MAX_OUTPUTS; chan++) {
            for (int i = 0; i < nframes; i++) {
                out[chan + WARPBUF_MAX_OUTPUTS * i] = 0.;
            }
        }
        return;
    }

    double _;
    double currentBPM = -1.;

    m_clipInfo.beat_to_seconds(m_playHeadBeats, _, currentBPM);

    double loop_end_seconds = 0.;
    m_clipInfo.beat_to_seconds(m_clipInfo.loop_end, loop_end_seconds, _);

    double ratio = (m_srate / sfinfo.samplerate);
    if (currentBPM > 0) {
        ratio *= currentBPM / m_bpm;
    }
    m_rbstretcher->setTimeRatio(ratio);

    // progress the playhead based on the number of frames.
    // Note that it would be slightly better to do this more incrementally inside
    // a loop, but that's hard to conceptualize with how a rubber band stretcher
    // makes samples retrievable.
    m_playHeadBeats += m_bpm * double(nframes) / (60. * m_srate);

    int count = 0;
    int numAvailable = m_rbstretcher->available();
    int allowedReadCount = 0;
    // In a while loop, we read from the sound file into an interleaved buffer and count the number of samples we read in a "count" variable.
    // Note that sf_readf_float can return -1 if no samples were read.
    // If count is zero:
    //     Then maybe we want to loop around and update our read position.
    //     If our new read position is still out of bounds, then we should just "count" 1 new frame and write zeros for it.
    //     Otherwise, our new read position is in bounds (but count is zero), so we can continue and expect new samples next pass in the while loop.
    // Once we've read new samples or written zeros into our interleaved buffer, we copy it into a non-interleaved buffer.
    // Then we tell Rubber Band to process the non-interleaved buffer and wait for enough available output samples.
    // When there are enough available samples, we retrieve them and copy them into chuck's output buffer.
    // Note that ChucK pretty much only asks for one frame at a time.
    while (numAvailable < nframes) {

        // If our sample read position is in bounds given our sound file
        if (sfReadPos > -1 && sfReadPos < sfinfo.frames) {
            // Every call to sf_readf_float might be slow, so if we're going to call it at all, it's best to ask for many samples.
            // The upper limit on the number of samples we'll ask for is `ibs`. However, if we're near the loop end and loop is enabled,
            // then we might have fewer samples to ask for: We don't want to read samples that are to the right of the loop end. Instead
            // we want to go to the loop start.
            allowedReadCount = m_clipInfo.loop_on ? std::min(ibs, (int)(loop_end_seconds * sfinfo.samplerate - sfReadPos)) : ibs;
            count = sf_readf_float(sndfile, m_interleavedBuffer, allowedReadCount);
            count = std::max(0, count);
            sfReadPos += count;
        }
        else {
            count = 0;
        }

        if (count < 1) {
            if (m_clipInfo.loop_on) {
                // We are looping, so seek to the loop start of the audio file.
                // This will change sfReadPos and hopefully put it in bounds.
                setPlayhead(m_clipInfo.loop_start);
            }

            // If our read pos is out of bounds, then we should write one frame of zeros.
            if (sfReadPos < 0 || sfReadPos >= sfinfo.frames) {
                count = 1;
                // fill with zeros
                for (int c = 0; c < m_channels; c++) {
                    for (int i = 0; i < count; i++) {
                        m_interleavedBuffer[i * m_channels + c] = 0.;
                    }
                }
            }
            else {
                // Our read pos is in bounds and count is still <= zero, so we can continue. We can expect to get samples next time.
                continue;
            }
        }
        
        // Copy from the interleaved buffer to the non interleaved buffer.
        for (int chan = 0; chan < m_channels; ++chan) {
            for (int i = 0; i < count; ++i) {
                m_nonInterleavedBuffer[chan][i] = m_interleavedBuffer[i * m_channels + chan];
            }
        }

        m_rbstretcher->process(m_nonInterleavedBuffer, count, false);
        numAvailable = m_rbstretcher->available();
    }

    m_rbstretcher->retrieve(m_retrieveBuffer, nframes);

    // Copy from m_retrieveBuffer to chuck's output.
    // out needs to receive alternating left/right channels.
    for (int chan = 0; chan < m_channels; chan++) {
        auto chanPtr = m_retrieveBuffer[chan];
        for (int i = 0; i < nframes; i++) {
            out[chan + m_channels * i] = *chanPtr++;
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
        const double bpm = 120.;
        const double end_in_beats = bpm * sfinfo.frames / (sfinfo.samplerate * 60.);
        m_clipInfo.loop_start = 0.;
        m_clipInfo.hidden_loop_start = 0.;
        m_clipInfo.start_marker = 0.;
        m_clipInfo.hidden_loop_end = end_in_beats;
        m_clipInfo.loop_end = end_in_beats;
        m_clipInfo.end_marker = end_in_beats;
    }

    this->setPlayhead(m_clipInfo.start_marker);

    this->resetStretcher();

    return true;
}