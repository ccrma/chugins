#include "WarpBufChugin.h"
#include <filesystem>

WarpBufChugin::WarpBufChugin(t_CKFLOAT srate)
{
    // sample rate
    m_srate = srate;
    memset(&sfinfo, 0, sizeof(SF_INFO));

    this->recreateStretcher();
}

WarpBufChugin::~WarpBufChugin()
{
    clearBufs();
    m_rbstretcher.release();
}

void
WarpBufChugin::reset() {
    m_rbstretcher->reset();
}

void
WarpBufChugin::recreateStretcher() {

    using namespace RubberBand;

    RubberBandStretcher::Options options = 0;

    options |= RubberBandStretcher::OptionProcessRealTime;
    options |= RubberBandStretcher::OptionStretchPrecise; // This setting is always used when running in real-time mode
    options |= RubberBandStretcher::OptionThreadingNever;
    options |= RubberBandStretcher::OptionPitchHighQuality;

    m_rbstretcher = std::make_unique<RubberBand::RubberBandStretcher>(
        m_srate,
        sfinfo.channels,
        options,
        1.,
        1.);
}

// clear
void
WarpBufChugin::clearBufs()
{
    if (m_retrieveBuffer != NULL)
    {
        for (int i = 0; i < m_channels; i++) {
            CK_SAFE_DELETE_ARRAY(m_retrieveBuffer[i]);
        }
    }
    CK_SAFE_DELETE_ARRAY(m_retrieveBuffer);

    CK_SAFE_DELETE_ARRAY(m_interleavedBuffer);

    if (m_nonInterleavedBuffer != NULL)
    {
        for (int i = 0; i < m_channels; i++) {
            CK_SAFE_DELETE_ARRAY(m_nonInterleavedBuffer[i]);
        }
    }
    CK_SAFE_DELETE_ARRAY(m_nonInterleavedBuffer);
}

// Allocate the buffers if the number of channels or number of samples changed.
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
        m_nonInterleavedBuffer[i] = new float[interleaved_buffer_size];
    }
       
    m_interleavedBuffer = new float[m_channels * interleaved_buffer_size];
    
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
    sfReadPos = m_clipInfo.beat_to_sample(m_playHeadBeats, sfinfo.samplerate);
    sf_seek(sndfile, std::max(0, sfReadPos), SEEK_SET);
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
        std::fill_n(out, nframes * WARPBUF_MAX_OUTPUTS, 0.f);
        return;
    }

    double _;
    double clipBPM = -1.;

    m_clipInfo.beat_to_seconds(m_playHeadBeats, _, clipBPM);

    int loop_start_sample = m_clipInfo.beat_to_sample(m_clipInfo.loop_start, sfinfo.samplerate);
    int loop_end_sample = m_clipInfo.beat_to_sample(m_clipInfo.loop_end, sfinfo.samplerate);

    double ratio = (m_srate / sfinfo.samplerate);
    if (clipBPM > 0) {
        ratio *= clipBPM / m_bpm;
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
    // We also track our read position integer in `sfReadPos`.
    // If count is zero:
    //   Then maybe we want to loop around and update our read position.
    //   Or maybe we have some reason to write zeros into the interleaved buffer (discussed in code far below).
    // Once we've read new samples or written zeros into tje interleaved buffer, we copy it into a non-interleaved buffer.
    // Then we tell Rubber Band to process the non-interleaved buffer, and we wait for enough retrievable samples.
    // When there are enough samples, we retrieve them and copy them into chuck's output buffer.
    // Note that ChucK pretty much only asks for one frame at a time (`nframes` is 1).
    while (numAvailable < nframes) {

        // If our sample read position is in bounds given our sound file
        if (sfReadPos > -1 && sfReadPos < sfinfo.frames) {

            // We might want to artificially restrict the number of samples to try to read from the soundfile.
            if (m_clipInfo.loop_on) {
                // If we're looping, we want to prevent reading samples beyond the loop end sample point.
                // If we're not beyond the loop end sample read point, we can greedily grab `interleaved_buffer_size` samples.
                allowedReadCount = std::min(interleaved_buffer_size, loop_end_sample - sfReadPos);
            }
            else {
                // Not looping, so greedily grab `interleaved_buffer_size` samples.
                allowedReadCount = interleaved_buffer_size;
            }
            if (allowedReadCount) {
                count = sf_readf_float(sndfile, m_interleavedBuffer, allowedReadCount);
                // Note that sf_readf_float can return -1 if no samples were read, so we take a max with zero.
                count = std::max(0, count);
                sfReadPos += count;
            }
            else {
                count = 0;
            }
        }
        else {
            // Our sfReadPos is out of bounds, so we didn't "read" any samples: set count to zero.
            // We will probably want to write zeros into the buffer given to Rubber Band.
            count = 0;
        }

        if (count < 1) {

            bool do_write_zeros = false;
            if (m_clipInfo.loop_on) {

                if ((sfReadPos >= loop_end_sample) && (loop_end_sample > loop_start_sample)) {
                    // Our read position reached the loop_end_sample, so we should go back to the loop start.
                    // This will change sfReadPos and hopefully put it in bounds.
                    setPlayhead(m_clipInfo.loop_start);
                }
                else {
                    // Our read position didn't reach the loop_end sample. Maybe the loop_end_sample is actually
                    // to the right of the bounds of the file. We have to write zeros to kill time until we get there.
                    // Or maybe the loop_end is less than the loop_start, and we should write zeros.
                    do_write_zeros = true;
                }
            }

            // If we decided earlier to write zeros, or our read pos is out of bounds, then we should write one frame of zeros.
            if (do_write_zeros || sfReadPos < 0 || sfReadPos >= sfinfo.frames) {
                count = 1;
                sfReadPos += count;
                // fill with zeros
                for (int c = 0; c < m_channels; c++) {
                    for (int i = 0; i < count; i++) {
                        m_interleavedBuffer[i * m_channels + c] = 0.;
                    }
                }
            }
            else {
                // We didn't write zeros (or anything) to the interleaved buffer, so we have to "continue"
                // in order to not copy the invalid interleaved buffer to the nonInterleavedBuffer.
                // However, our read pos is in bounds, so we can expect to get samples next time.
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
WarpBufChugin::read(const std::string& path) {
    
    memset(&sfinfo, 0, sizeof(SF_INFO));

    sndfile = sf_open(path.c_str(), SFM_READ, &sfinfo);
    if (!sndfile) {
        std::cerr << "ERROR: Failed to open input file \"" << path << "\": "
            << sf_strerror(sndfile) << std::endl;
        return false;
    }

    if (sfinfo.samplerate == 0) {
        std::cerr << "ERROR: File lacks sample rate in header" << std::endl;
        return false;
    }

    auto asd_path = path + std::string(".asd");
    bool file_exists = std::filesystem::exists(asd_path);

    if (! (file_exists && m_clipInfo.readWarpFile(asd_path.c_str()))) {
        // We didn't find a warp file, so assume it's 120 bpm.
        const double bpm = 120.;
        const double end_in_beats = bpm * sfinfo.frames / (sfinfo.samplerate * 60.);
        //m_clipInfo.loop_on = true; // todo: maybe we want to do this. Let's just preserve the previous setting.
        //m_clipInfo.warp_on = true; // todo: maybe we want to do this. Let's just preserve the previous setting.
        m_clipInfo.loop_start = 0.;
        m_clipInfo.hidden_loop_start = 0.;
        m_clipInfo.start_marker = 0.;
        m_clipInfo.hidden_loop_end = end_in_beats;
        m_clipInfo.loop_end = end_in_beats;
        m_clipInfo.end_marker = end_in_beats;

        // reset the warp markers based on the default bpm:
        m_clipInfo.warp_markers.clear();
        m_clipInfo.warp_markers.push_back(std::make_pair(0, 0));
        double beats = 1. / 32.;
        double durSeconds = beats * (60. / bpm);
        m_clipInfo.warp_markers.push_back(std::make_pair(durSeconds, beats));
    }

    this->setPlayhead(m_clipInfo.start_marker);

    this->recreateStretcher();

    return true;
}
