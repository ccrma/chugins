//-----------------------------------------------------------------------------
// Sampler.cpp - Implementation of Sampler class
// Granular sampler with pitch-shifting and time-stretching
//-----------------------------------------------------------------------------

#include "Sampler.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace {
constexpr double kPi = 3.14159265358979323846;  // <cmath>'s M_PI is not portable (MSVC)

// Parameter setters ignore non-finite values: a NaN reaching the DSP would
// otherwise poison playheads (and, downstream, the filter state) permanently.
inline bool isFinite(double v) { return std::isfinite(v); }
}  // namespace

//-----------------------------------------------------------------------------
// SamplerVoice implementation
//-----------------------------------------------------------------------------

SamplerVoice::SamplerVoice()
    : playhead(0.0), velocity(1.0), pingpongDirection(1.0), stopPosition(0.0), sliceStart(0.0), sliceEnd(0.0), grainPhase1(0.0),
      grainPhase2(0.5), grainPos1(0.0), grainPos2(0.0), envValue(0.0), envState(ENV_IDLE), holdCounter(0),
      releaseRequested(false), ageSamples(0), currentLevel(0.0) {}

void SamplerVoice::reset() {
    VoiceBase::reset();  // Reset base class members
    playhead = 0.0;
    velocity = 1.0;
    pingpongDirection = 1.0;
    stopPosition = 0.0;
    sliceStart = 0.0;
    sliceEnd = 0.0;
    grainPhase1 = 0.0;
    grainPhase2 = 0.5;
    grainPos1 = 0.0;
    grainPos2 = 0.0;
    envValue = 0.0;
    envState = ENV_IDLE;
    holdCounter = 0;
    releaseRequested = false;
    ageSamples = 0;
    currentLevel = 0.0;
}

//-----------------------------------------------------------------------------
// Sampler Constructor
//-----------------------------------------------------------------------------
Sampler::Sampler(double fs)
    :  // A non-positive / non-finite rate would make every envelope coefficient
       // and the file/system ratio inf or NaN; fall back to 48 kHz.
      m_systemSampleRate((isFinite(fs) && fs > 0.0) ? fs : 48000.0), m_buffer(nullptr), m_bufferFrames(0),
      m_channels(0), m_fileSampleRate(0.0), m_filename(""), m_reverse(false), m_transpose(0.0), m_rate(1.0),
      m_grainSize(0.05),  // 50ms default
      m_playbackMode(PLAYBACK_ONESHOT), m_loopStart(0.0), m_loopEnd(1.0), m_startMarker(0.0),
      m_endMarker(0.0), m_interpMode(INTERP_LINEAR), m_noteTracking(0), m_rootNote(60), m_gain(1.0),
      m_velocitySens(1.0), m_decay(0.0), m_fadeOut(0.0), m_envelopeEnable(false), m_attackTime(0.01),
      m_holdTime(0.0), m_releaseTime(0.1), m_attackCoeff(0.0), m_releaseCoeff(0.0) {
    // Calculate exponential envelope coefficients from default times
    // For 99% decay to target in specified time: coeff = 1 - exp(-4.6 / (time * sampleRate))
    if (m_attackTime > 0.0) {
        m_attackCoeff = 1.0 - std::exp(-4.6 / (m_attackTime * m_systemSampleRate));
    } else {
        m_attackCoeff = 1.0;  // Instant attack
    }

    if (m_releaseTime > 0.0) {
        m_releaseCoeff = 1.0 - std::exp(-4.6 / (m_releaseTime * m_systemSampleRate));
    } else {
        m_releaseCoeff = 1.0;  // Instant release
    }

    // Initialize note position map (MIDI notes 0-127)
    for (int i = 0; i < 128; i++) {
        m_notePositions[i] = 0.0;  // Default to start
    }
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
Sampler::~Sampler() { clearBuffer(); }

//-----------------------------------------------------------------------------
// File loading
//-----------------------------------------------------------------------------
bool Sampler::read(const std::string& filename) {
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(SF_INFO));

    SNDFILE* sndfile = sf_open(filename.c_str(), SFM_READ, &sfinfo);
    if (!sndfile) {
        std::cerr << "Sampler: Failed to open file \"" << filename << "\": " << sf_strerror(sndfile)
                  << std::endl;
        return false;
    }

    if (sfinfo.samplerate == 0) {
        std::cerr << "Sampler: File lacks sample rate in header" << std::endl;
        sf_close(sndfile);
        return false;
    }

    // readSample() addresses the buffer as mono or interleaved stereo; a
    // 4-channel file would otherwise be read with a stereo stride (garbage).
    if (sfinfo.channels < 1 || sfinfo.channels > 2) {
        std::cerr << "Sampler: Unsupported channel count " << sfinfo.channels
                  << " (only mono and stereo files are supported)" << std::endl;
        sf_close(sndfile);
        return false;
    }

    // Clear existing buffer
    clearBuffer();

    // Allocate new buffer
    m_bufferFrames = sfinfo.frames;
    m_channels = sfinfo.channels;

    // Check for potential overflow in buffer size calculation
    if (m_channels > 0 && static_cast<size_t>(m_bufferFrames) > SIZE_MAX / m_channels / sizeof(float)) {
        std::cerr << "Sampler: File too large - buffer size would overflow" << std::endl;
        sf_close(sndfile);
        return false;
    }

    m_buffer = new float[m_bufferFrames * m_channels];

    // Read entire file into memory
    sf_count_t framesRead = sf_readf_float(sndfile, m_buffer, m_bufferFrames);
    if (framesRead != m_bufferFrames) {
        std::cerr << "Sampler: Warning - read " << framesRead << " frames, expected " << m_bufferFrames
                  << std::endl;
        m_bufferFrames = framesRead;
    }

    sf_close(sndfile);

    // Store sample rate and calculate duration
    m_fileSampleRate = sfinfo.samplerate;
    double duration = (double)m_bufferFrames / m_fileSampleRate;

    // Store filename
    m_filename = filename;

    // Reset loop end to match new buffer
    m_loopEnd = 1.0;

    // Initialize start/end markers
    m_startMarker = 0.0;
    m_endMarker = duration;

    return true;
}

//-----------------------------------------------------------------------------
// Playback control
//-----------------------------------------------------------------------------
void Sampler::noteOn(int note, int velocity) {
    if (note < 0 || note > 127) return;
    if (!m_buffer || m_bufferFrames == 0) return;

    // THRU mode is monophonic - stop all active voices for realistic drum choking
    // (essential for hi-hat open/closed, cymbal muting, etc.)
    if (m_playbackMode == PLAYBACK_THRU) {
        m_voiceManager.allNotesOff();
    }

    // Allocate a voice (finds free voice or steals one)
    SamplerVoice* voice = m_voiceManager.allocateVoice(note);
    if (!voice) return;

    // Apply velocity sensitivity curve. Clamp first: pow(negative, non-integer)
    // is NaN, which would poison every sample of the voice and the mix.
    double normVel = std::max(0, std::min(127, velocity)) / 127.0;
    if (m_velocitySens == 0.0) {
        voice->velocity = 1.0;  // No velocity sensitivity
    } else {
        voice->velocity = std::pow(normVel, m_velocitySens);
    }
    voice->currentLevel = voice->velocity;  // until the first tick refines it

    // Calculate start/end marker positions in samples
    double startMarkerSample = m_startMarker * m_fileSampleRate;
    double endMarkerSample = m_endMarker * m_fileSampleRate;

    // Convert note position from seconds to samples
    double notePosSample = m_notePositions[note] * m_fileSampleRate;

    // Clamp note position to marker range
    double startPos = std::max(startMarkerSample, std::min(endMarkerSample, notePosSample));

    // The note's slice runs from its position to the next higher note position
    // (or the end marker) — ONESHOT auto-slicing. Clamped so a position past
    // the end of file cannot make a voice run beyond the buffer.
    double currentNotePos = m_notePositions[note];
    double nextNotePos = -1.0;  // -1 means "not found"
    for (int i = 0; i < 128; i++) {
        if (m_notePositions[i] > currentNotePos) {
            if (nextNotePos < 0.0 || m_notePositions[i] < nextNotePos) {
                nextNotePos = m_notePositions[i];
            }
        }
    }
    double sliceEnd = (nextNotePos >= 0.0) ? nextNotePos * m_fileSampleRate : endMarkerSample;
    sliceEnd = std::max(startPos, std::min(endMarkerSample, sliceEnd));

    // Initialize playhead, direction and (ONESHOT) stop position.
    if (m_reverse) {
        if (m_playbackMode == PLAYBACK_LOOP || m_playbackMode == PLAYBACK_PINGPONG) {
            // Loop/ping-pong: start at loop end
            double loopEndSample = std::max(m_loopStart, m_loopEnd) * m_bufferFrames;
            loopEndSample = std::min(loopEndSample, endMarkerSample);
            voice->playhead = loopEndSample;
        } else if (m_playbackMode == PLAYBACK_THRU) {
            // Thru mode reverse: start at end of file
            voice->playhead = (double)m_bufferFrames;
        } else {
            // One-shot reverse: play the note's own slice backwards, from its
            // last frame (sliceEnd is the *next* slice's first frame) down to
            // the note position.
            voice->playhead = std::max(startPos, sliceEnd - 1.0);
        }
        voice->pingpongDirection = -1.0;
        voice->stopPosition = (m_playbackMode == PLAYBACK_THRU) ? 0.0 : startPos;
    } else {
        // Forward playback: start at note position (within marker range)
        voice->playhead = startPos;
        voice->pingpongDirection = 1.0;
        voice->stopPosition = (m_playbackMode == PLAYBACK_THRU) ? (double)m_bufferFrames : sliceEnd;
    }
    // ONESHOT grain reads are confined to the slice (see confineGrainPos).
    voice->sliceStart = startPos;
    voice->sliceEnd = sliceEnd;

    // Reset grain phases for a clean start. Both grains read from the note
    // position: grain 2 begins at phase 0.5 (window = 1) so the sample *at* the
    // note position is heard at full level on the first tick, while grain 1
    // (window = 0) fades in behind it. Offsetting grain 2 by half a grain
    // instead would silence the onset transient and play material 25 ms
    // ahead first.
    voice->grainPhase1 = 0.0;
    voice->grainPhase2 = 0.5;
    voice->grainPos1 = voice->playhead;
    voice->grainPos2 = voice->playhead;

    // Start envelope attack phase if envelope is enabled
    if (m_envelopeEnable) {
        voice->envState = ENV_ATTACK;
        voice->envValue = 0.0;
        // Reset hold tracking
        voice->holdCounter = 0;
        voice->releaseRequested = false;
    } else {
        voice->envValue = 1.0;
    }
}

// Release one voice: run the envelope's release stage if the envelope is
// enabled, otherwise stop immediately (in every playback mode — a looping
// voice must still be stoppable by noteOff).
void Sampler::releaseVoice(SamplerVoice* voice) {
    if (m_envelopeEnable) {
        if (voice->envState == ENV_IDLE) {
            // Voice was started while the envelope was off (envValue == 1);
            // put it in SUSTAIN so the release below actually runs.
            voice->envState = ENV_SUSTAIN;
        }
        // Request release (delayed until the minimum hold time has elapsed).
        voice->releaseRequested = true;
    } else {
        voice->active = false;
    }
}

// Turn off all active voices
void Sampler::noteOff() {
    m_voiceManager.forEachActiveVoice([this](SamplerVoice* voice) { releaseVoice(voice); });
}

// Turn off voices playing specific note
void Sampler::noteOff(int note) {
    m_voiceManager.forEachVoiceByNote(note, [this](SamplerVoice* voice) { releaseVoice(voice); });
}

// Immediately stop all voices
void Sampler::allNotesOff() { m_voiceManager.allNotesOff(); }

void Sampler::setNotePosition(int note, double position) {
    if (note >= 0 && note < 128) {
        // Store position in seconds (clamped to non-negative). Positions past
        // the end of the file are clamped to the end marker at noteOn.
        if (!isFinite(position)) return;
        m_notePositions[note] = std::max(0.0, position);
    }
}

//-----------------------------------------------------------------------------
// Audio processing (tick) - mix all active voices in stereo
//-----------------------------------------------------------------------------
void Sampler::tick(float* in, float* out) {
    if (!m_buffer || m_bufferFrames == 0) {
        out[0] = 0.0;
        out[1] = 0.0;
        return;
    }

    // Increment global sample counter for voice timestamps
    m_voiceManager.tick();

    // Mix all active voices in stereo
    double outputLeft = 0.0;
    double outputRight = 0.0;
    m_voiceManager.forEachActiveVoice([this, &outputLeft, &outputRight](SamplerVoice* voice) {
        StereoSample voiceOutput = tickVoice(voice);
        outputLeft += voiceOutput.left;
        outputRight += voiceOutput.right;
    });

    out[0] = (float)outputLeft;
    out[1] = (float)outputRight;
}

//-----------------------------------------------------------------------------
// Voice processing
//-----------------------------------------------------------------------------

// Process a single voice and return its stereo output
Sampler::StereoSample Sampler::tickVoice(SamplerVoice* voice) {
    // Calculate note tracking offset (disabled in THRU mode)
    double noteOffset = 0.0;
    if (m_playbackMode != PLAYBACK_THRU && m_noteTracking != 0) {
        noteOffset = (double)(voice->note - m_rootNote);
    }

    // Calculate effective pitch and rate based on note tracking mode
    double effectiveTranspose = m_transpose;
    double effectiveRate = m_rate;

    if (m_playbackMode != PLAYBACK_THRU && m_noteTracking != 0) {
        if (m_noteTracking == 1) {
            // TRADITIONAL: Note affects both pitch and time (coupled, like tape speed)
            effectiveTranspose = m_transpose + noteOffset;
            effectiveRate = m_rate * std::pow(2.0, noteOffset / 12.0);
        } else if (m_noteTracking == 2) {
            // GRANULAR: Note affects only pitch (decoupled)
            effectiveTranspose = m_transpose + noteOffset;
            effectiveRate = m_rate;
        }
    }
    // If m_noteTracking == 0 (OFF), noteOffset stays 0 and no tracking occurs

    // Calculate pitch (playback rate through grains). Scale by file/system
    // sample-rate ratio so a 44.1 kHz file rendered at 48 kHz keeps its
    // original pitch and duration (playhead/grain positions are in file frames).
    double srRatio = (m_fileSampleRate > 0.0) ? m_fileSampleRate / m_systemSampleRate : 1.0;
    double pitchScale = std::pow(2.0, effectiveTranspose / 12.0) * srRatio;

    // Calculate grain size in samples
    int grainSamples = (int)(m_grainSize * m_systemSampleRate);
    if (grainSamples < MIN_GRAIN_SAMPLES) grainSamples = MIN_GRAIN_SAMPLES;

    // Region the grains may read. loop_start/loop_end are set independently,
    // so they may be equal or crossed; use the ordered pair. In ONESHOT the
    // region is the voice's slice; in THRU the whole file.
    double regionLo, regionHi;
    if (m_playbackMode == PLAYBACK_LOOP || m_playbackMode == PLAYBACK_PINGPONG) {
        regionLo = std::min(m_loopStart, m_loopEnd) * m_bufferFrames;
        regionHi = std::max(m_loopStart, m_loopEnd) * m_bufferFrames;
    } else if (m_playbackMode == PLAYBACK_ONESHOT) {
        regionLo = voice->sliceStart;
        regionHi = voice->sliceEnd;
    } else {
        regionLo = 0.0;
        regionHi = (double)m_bufferFrames;
    }

    // Generate output from two overlapping grains reading from offset positions
    StereoSample out1 = generateGrain(voice, voice->grainPhase1, voice->grainPos1, regionLo, regionHi);
    StereoSample out2 = generateGrain(voice, voice->grainPhase2, voice->grainPos2, regionLo, regionHi);

    // Update envelope if enabled — or if this voice's envelope is mid-flight
    // (envelope was disabled while it was attacking/releasing): let it finish
    // rather than freezing at a partial level forever.
    if (m_envelopeEnable || voice->envState != ENV_IDLE) {
        updateVoiceEnvelope(voice);
    }

    // Mix the two grains and apply envelope. The two Hann windows are half a
    // grain apart, so they sum to exactly 1: at unity gain the output level
    // matches the source (no 0.5 factor, which would sit at -6 dB).
    double level = voice->velocity * voice->envValue;

    // Per-hit shaping (independent of the AHR envelope).
    if (m_decay > 0.0) {
        // -60 dB after m_decay seconds: exp(-ln(1000) * t / decay)
        level *= std::exp(-6.907755 * (double)voice->ageSamples / (m_decay * m_systemSampleRate));
    }
    if (m_fadeOut > 0.0 && (m_playbackMode == PLAYBACK_ONESHOT || m_playbackMode == PLAYBACK_THRU)) {
        // Linear fade over the last m_fadeOut seconds before the stop boundary.
        // Time to the boundary in system samples = remaining file frames /
        // (file frames advanced per system sample).
        double remainingFrames = (voice->stopPosition - voice->playhead) * voice->pingpongDirection;
        double framesPerSample = std::max(1e-9, effectiveRate * srRatio);
        double samplesLeft = remainingFrames / framesPerSample;
        double fadeSamples = m_fadeOut * m_systemSampleRate;
        if (samplesLeft < fadeSamples) {
            level *= std::max(0.0, samplesLeft / fadeSamples);
        }
    }
    // Level seen by STEAL_QUIETEST. A voice still attacking is about to get
    // loud, so rank it by where it is heading rather than steal it first.
    voice->currentLevel = (voice->envState == ENV_ATTACK) ? voice->velocity : level;
    double gainEnv = level * m_gain;
    voice->ageSamples++;
    StereoSample output;
    output.left = (out1.left + out2.left) * gainEnv;
    output.right = (out1.right + out2.right) * gainEnv;

    // Advance grain phases
    double grainIncrement = 1.0 / grainSamples;
    voice->grainPhase1 += grainIncrement;
    voice->grainPhase2 += grainIncrement;

    // Advance grain read positions (pitch only, not time)
    double pitchIncrement = pitchScale;
    voice->grainPos1 += pitchIncrement * voice->pingpongDirection;
    voice->grainPos2 += pitchIncrement * voice->pingpongDirection;

    // When grain completes, reset with hop based on rate (time stretch)
    double grainHop = grainSamples * effectiveRate * srRatio;  // Rate controls time stretch
    // Wrap by subtracting 1 (not resetting to 0) so the two phases keep their
    // exact half-grain offset and the Hann windows keep summing to 1; the
    // grain read position is likewise offset by the overshoot.
    if (voice->grainPhase1 >= 1.0) {
        voice->grainPhase1 -= 1.0;
        voice->grainPos1 =
            voice->playhead + voice->grainPhase1 * grainSamples * pitchScale * voice->pingpongDirection;
    }
    if (voice->grainPhase2 >= 1.0) {
        voice->grainPhase2 -= 1.0;
        voice->grainPos2 =
            voice->playhead + voice->grainPhase2 * grainSamples * pitchScale * voice->pingpongDirection;
    }

    // Advance main playhead (controls time, affected by rate)
    voice->playhead += grainHop * voice->pingpongDirection / grainSamples;

    // Handle playback boundaries based on mode. Treat a loop shorter than one
    // frame as a frozen playhead — never fmod(x, 0), which is NaN and would
    // poison every later buffer read.
    double loopLength = regionHi - regionLo;

    if (m_playbackMode == PLAYBACK_LOOP) {
        // LOOP mode: wrap playhead at loop boundaries. Grain read positions
        // are not touched: they keep running and confineGrainPos() wraps
        // their reads into the loop, so the grain that straddles the loop
        // point plays continuously across it.
        if (loopLength < 1.0) {
            voice->playhead = regionLo;
        } else if (voice->pingpongDirection > 0) {
            // Playing forward
            if (voice->playhead >= regionHi) {
                voice->playhead = regionLo + fmod(voice->playhead - regionLo, loopLength);
            }
        } else {
            // Playing reverse
            if (voice->playhead <= regionLo) {
                double overshoot = regionLo - voice->playhead;
                voice->playhead = regionHi - fmod(overshoot, loopLength);
            }
        }
    } else if (m_playbackMode == PLAYBACK_PINGPONG) {
        // PING-PONG mode: bounce at loop boundaries. fmod folds the overshoot
        // into the loop first so a hop longer than the loop (extreme rate on
        // a tiny loop) can't push the playhead outside the region for good.
        // Grains that had already run past the bounce point are mirrored with
        // the playhead so they continue seamlessly in the new direction.
        if (loopLength < 1.0) {
            voice->playhead = regionLo;
        } else if (voice->pingpongDirection > 0 && voice->playhead >= regionHi) {
            voice->pingpongDirection = -1.0;
            voice->playhead = regionHi - fmod(voice->playhead - regionHi, loopLength);
            if (voice->grainPos1 > regionHi) voice->grainPos1 = 2.0 * regionHi - voice->grainPos1;
            if (voice->grainPos2 > regionHi) voice->grainPos2 = 2.0 * regionHi - voice->grainPos2;
        } else if (voice->pingpongDirection < 0 && voice->playhead <= regionLo) {
            voice->pingpongDirection = 1.0;
            voice->playhead = regionLo + fmod(regionLo - voice->playhead, loopLength);
            if (voice->grainPos1 < regionLo) voice->grainPos1 = 2.0 * regionLo - voice->grainPos1;
            if (voice->grainPos2 < regionLo) voice->grainPos2 = 2.0 * regionLo - voice->grainPos2;
        }
    } else {
        // ONESHOT: stop at the voice's slice boundary (auto-slicing via
        // notePos). THRU: play through the end marker to the end of file
        // (or, reversed, from the end of file down to 0). stopPosition holds
        // the boundary for the voice's direction in both modes.
        if (voice->pingpongDirection > 0 ? voice->playhead >= voice->stopPosition
                                         : voice->playhead <= voice->stopPosition) {
            voice->active = false;
            return {0.0, 0.0};
        }
    }

    return output;
}

// Confine a grain read position to the region the voice may play. The two
// grains run up to one grain ahead of (or behind) the playhead — by
// (pitchScale - rate) * grainSize frames — so without this they would read
// past a loop point (a gap of silence, or material outside the loop, on every
// iteration), past a bounce point, or into the next slice of a sliced break
// (a pre-echo of the next hit on every pitched-up slice).
double Sampler::confineGrainPos(const SamplerVoice* voice, double pos, double regionLo, double regionHi) const {
    double dir = voice->pingpongDirection;
    switch (m_playbackMode) {
        case PLAYBACK_LOOP: {
            // Wrap only in the direction of travel: a voice whose note position
            // lies outside the loop legitimately plays up to the loop first.
            double len = regionHi - regionLo;
            if (len < 1.0) return regionLo;
            if (dir > 0 && pos >= regionHi) return regionLo + std::fmod(pos - regionLo, len);
            if (dir < 0 && pos < regionLo) return regionHi - std::fmod(regionLo - pos, len);
            return pos;
        }
        case PLAYBACK_PINGPONG:
            // Mirror at the bounce point (the playhead will do the same).
            if (dir > 0 && pos > regionHi) return 2.0 * regionHi - pos;
            if (dir < 0 && pos < regionLo) return 2.0 * regionLo - pos;
            return pos;
        case PLAYBACK_ONESHOT:
            // Outside the slice there is silence; readSample() treats a
            // negative position as such.
            if (pos < regionLo || pos >= regionHi) return -1.0;
            return pos;
        default:  // PLAYBACK_THRU: whole file, bounded by readSample()
            return pos;
    }
}

//-----------------------------------------------------------------------------
// Getters and setters
//-----------------------------------------------------------------------------
void Sampler::setTranspose(double semitones) {
    if (isFinite(semitones)) m_transpose = semitones;
}
double Sampler::getTranspose() const { return m_transpose; }

void Sampler::setRate(double rate) {
    if (isFinite(rate)) m_rate = std::max(0.001, rate);
}
double Sampler::getRate() const { return m_rate; }

void Sampler::setGrainSize(double seconds) {
    if (isFinite(seconds)) m_grainSize = std::max(0.001, std::min(1.0, seconds));
}
double Sampler::getGrainSize() const { return m_grainSize; }

void Sampler::setPlaybackMode(int mode) {
    if (mode >= 0 && mode <= 3) {
        m_playbackMode = (PlaybackMode)mode;
    }
}
int Sampler::getPlaybackMode() const { return (int)m_playbackMode; }

void Sampler::setLoopStart(double pos) {
    if (isFinite(pos)) m_loopStart = std::max(0.0, std::min(1.0, pos));
}
double Sampler::getLoopStart() const { return m_loopStart; }

void Sampler::setLoopEnd(double pos) {
    if (isFinite(pos)) m_loopEnd = std::max(0.0, std::min(1.0, pos));
}
double Sampler::getLoopEnd() const { return m_loopEnd; }

void Sampler::setGain(double dB) {
    // Clamp to [-120, +60] dB: pow(10, 500) is inf, which would drive the
    // post-mix filter state to NaN permanently.
    if (isFinite(dB)) m_gain = std::pow(10.0, std::max(-120.0, std::min(60.0, dB)) / 20.0);
}
double Sampler::getGain() const { return 20.0 * std::log10(std::max(1e-10, m_gain)); }

void Sampler::setEnvelope(bool enable) { m_envelopeEnable = enable; }
bool Sampler::getEnvelope() const { return m_envelopeEnable; }

void Sampler::setAttack(double seconds) {
    if (!isFinite(seconds)) return;
    m_attackTime = std::max(0.0, std::min(10.0, seconds));
    // Recalculate exponential coefficient
    if (m_attackTime > 0.0) {
        m_attackCoeff = 1.0 - std::exp(-4.6 / (m_attackTime * m_systemSampleRate));
    } else {
        m_attackCoeff = 1.0;  // Instant attack
    }
}
double Sampler::getAttack() const { return m_attackTime; }

void Sampler::setHold(double seconds) {
    if (!isFinite(seconds)) return;
    m_holdTime = std::max(0.0, std::min(10.0, seconds));
}
double Sampler::getHold() const { return m_holdTime; }

void Sampler::setRelease(double seconds) {
    if (!isFinite(seconds)) return;
    m_releaseTime = std::max(0.0, std::min(10.0, seconds));
    // Recalculate exponential coefficient
    if (m_releaseTime > 0.0) {
        m_releaseCoeff = 1.0 - std::exp(-4.6 / (m_releaseTime * m_systemSampleRate));
    } else {
        m_releaseCoeff = 1.0;  // Instant release
    }
}
double Sampler::getRelease() const { return m_releaseTime; }

void Sampler::setDecay(double seconds) {
    if (isFinite(seconds)) m_decay = std::max(0.0, std::min(30.0, seconds));
}
double Sampler::getDecay() const { return m_decay; }

void Sampler::setFadeOut(double seconds) {
    if (isFinite(seconds)) m_fadeOut = std::max(0.0, std::min(10.0, seconds));
}
double Sampler::getFadeOut() const { return m_fadeOut; }

void Sampler::setVelocitySens(double sens) {
    if (isFinite(sens)) m_velocitySens = std::max(0.0, std::min(10.0, sens));
}
double Sampler::getVelocitySens() const { return m_velocitySens; }

void Sampler::setStartMarker(double seconds) {
    if (m_fileSampleRate == 0.0 || !isFinite(seconds)) return;  // No file loaded
    m_startMarker = std::max(0.0, seconds);
    // Clamp to not exceed end marker
    if (m_startMarker > m_endMarker) {
        m_startMarker = m_endMarker;
    }
}
double Sampler::getStartMarker() const { return m_startMarker; }

void Sampler::setEndMarker(double seconds) {
    if (m_fileSampleRate == 0.0 || !isFinite(seconds)) return;  // No file loaded
    double duration = (double)m_bufferFrames / m_fileSampleRate;
    m_endMarker = std::min(duration, std::max(0.0, seconds));
    // Clamp to not go below start marker
    if (m_endMarker < m_startMarker) {
        m_endMarker = m_startMarker;
    }
}
double Sampler::getEndMarker() const { return m_endMarker; }

void Sampler::setReverse(bool reverse) { m_reverse = reverse; }
bool Sampler::getReverse() const { return m_reverse; }

void Sampler::setInterp(int mode) {
    if (mode >= 0 && mode <= 2) {
        m_interpMode = (InterpolationMode)mode;
    }
}
int Sampler::getInterp() const { return (int)m_interpMode; }

void Sampler::setNoteTracking(int mode) {
    if (mode >= 0 && mode <= 2) {
        m_noteTracking = mode;
    }
}
int Sampler::getNoteTracking() const { return m_noteTracking; }

void Sampler::setRootNote(int note) {
    if (note >= 0 && note <= 127) {
        m_rootNote = note;
    }
}
int Sampler::getRootNote() const { return m_rootNote; }

//-----------------------------------------------------------------------------
// Polyphony control
//-----------------------------------------------------------------------------

void Sampler::setPolyphony(int voices) { m_voiceManager.setPolyphony(voices); }

int Sampler::getPolyphony() const { return m_voiceManager.getPolyphony(); }

void Sampler::setStealingPolicy(int policy) {
    if (policy >= 0 && policy <= 3) {
        m_voiceManager.setStealingPolicy((VoiceStealingPolicy)policy);
    }
}

int Sampler::getStealingPolicy() const { return (int)m_voiceManager.getStealingPolicy(); }

int Sampler::getActiveVoices() const { return m_voiceManager.getActiveVoiceCount(); }

//-----------------------------------------------------------------------------
// Query methods
//-----------------------------------------------------------------------------
int Sampler::channels() const { return m_channels; }

double Sampler::samplerate() const { return m_fileSampleRate; }

int Sampler::samples() const { return (int)m_bufferFrames; }

double Sampler::length() const {
    // Return duration in seconds
    if (m_fileSampleRate == 0.0) return 0.0;
    return (double)m_bufferFrames / m_fileSampleRate;
}

bool Sampler::isLoaded() const { return m_buffer != nullptr && m_bufferFrames > 0; }

std::string Sampler::filename() const { return m_filename; }

void Sampler::dump() const {
    if (!m_buffer || m_bufferFrames == 0) {
        std::cout << "Sampler: No file loaded" << std::endl;
        return;
    }

    std::cout << "Sampler: \"" << m_filename << "\" - " << m_bufferFrames << " frames, " << m_channels
              << " channels, " << m_fileSampleRate << " Hz (" << length() << " seconds)" << std::endl;
}

//-----------------------------------------------------------------------------
// Private helper methods
//-----------------------------------------------------------------------------
void Sampler::clearBuffer() {
    if (m_buffer) {
        delete[] m_buffer;
        m_buffer = nullptr;
    }
    m_bufferFrames = 0;
    m_channels = 0;
    m_filename = "";
}

// Update envelope for a single voice
void Sampler::updateVoiceEnvelope(SamplerVoice* voice) {
    // Increment hold counter during ATTACK and SUSTAIN
    if (voice->envState == ENV_ATTACK || voice->envState == ENV_SUSTAIN) {
        voice->holdCounter++;
    }

    if (voice->envState == ENV_ATTACK) {
        // Exponential attack using single-pole filter
        // Formula: envValue += coefficient * (target - envValue)
        // When coefficient = 1.0 (attackTime = 0.0), this gives instant attack
        voice->envValue += m_attackCoeff * (1.0 - voice->envValue);

        // Switch to sustain when close enough to target (avoids endless asymptotic approach)
        if (voice->envValue >= 0.999) {
            voice->envValue = 1.0;
            voice->envState = ENV_SUSTAIN;
        }
    } else if (voice->envState == ENV_SUSTAIN) {
        // Check if release was requested and minimum hold time has elapsed
        uint64_t holdSamples = (uint64_t)std::max(0.0, m_holdTime * m_systemSampleRate);
        if (voice->releaseRequested && voice->holdCounter >= holdSamples) {
            // Hold time met, start release
            voice->envState = ENV_RELEASE;
        }
        // Otherwise: hold at 1.0
    } else if (voice->envState == ENV_RELEASE) {
        // Exponential release using single-pole filter
        // Formula: envValue += coefficient * (target - envValue)
        // Equivalent to: envValue *= (1.0 - coefficient) since target = 0.0
        // When coefficient = 1.0 (releaseTime = 0.0), this gives instant release
        voice->envValue += m_releaseCoeff * (0.0 - voice->envValue);

        // Switch to idle when close enough to zero
        if (voice->envValue <= 0.001) {
            voice->envValue = 0.0;
            voice->envState = ENV_IDLE;
            voice->active = false;  // Stop voice when envelope completes
        }
    }
    // ENV_IDLE: hold at 0.0
}

Sampler::StereoSample Sampler::generateGrain(const SamplerVoice* voice, double grainPhase, double grainPos,
                                             double regionLo, double regionHi) const {
    // Hann window for grain envelope
    double window = 0.5 * (1.0 - std::cos(2.0 * kPi * grainPhase));

    // Read stereo sample from buffer at this grain's position
    StereoSample sample = readSample(confineGrainPos(voice, grainPos, regionLo, regionHi), regionLo, regionHi);

    // Apply window envelope to both channels
    return {sample.left * window, sample.right * window};
}

Sampler::StereoSample Sampler::readSample(double position, double regionLo, double regionHi) const {
    if (!m_buffer || m_bufferFrames == 0 || m_channels == 0) return {0.0, 0.0};

    // Outside the buffer there is silence, not a held last sample. Written as
    // a negated range test so a NaN position (which compares false with
    // everything) also lands here instead of reaching the (int) cast — which
    // is undefined behaviour and, on x86-64, an INT_MIN index.
    if (!(position >= 0.0 && position < (double)m_bufferFrames)) return {0.0, 0.0};

    int index = (int)position;
    double frac = position - index;

    // Interpolation neighbours must not cross the region boundary either: the
    // last frame before a loop point or slice end would otherwise blend with
    // the first frame beyond it — a one-sample click on every loop / slice.
    // Only applied while the read itself is inside the region (a voice may
    // legitimately start outside the loop and play up to it).
    int nlo = 0, nhi = (int)m_bufferFrames - 1;
    int regionLoIdx = std::max(0, (int)regionLo);
    int regionHiIdx = std::min((int)m_bufferFrames, (int)std::ceil(regionHi));
    if (index >= regionLoIdx && index < regionHiIdx) {
        nlo = regionLoIdx;
        nhi = regionHiIdx - 1;
    }

    if (m_interpMode == INTERP_DROP) {
        // Drop/none - no interpolation, just return sample at integer index
        if (m_channels == 1) {
            // Mono file: duplicate to both channels
            double mono = m_buffer[index];
            return {mono, mono};
        } else {
            // Stereo file: return left and right
            return {m_buffer[index * 2], m_buffer[index * 2 + 1]};
        }
    } else if (m_interpMode == INTERP_LINEAR) {
        // Linear interpolation (default)
        if (m_channels == 1) {
            // Mono file
            double sample1 = m_buffer[index];
            if (frac > 0.0 && index < nhi) {
                double sample2 = m_buffer[index + 1];
                double interpolated = sample1 + frac * (sample2 - sample1);
                return {interpolated, interpolated};
            }
            return {sample1, sample1};
        } else {
            // Stereo file
            double left1 = m_buffer[index * 2];
            double right1 = m_buffer[index * 2 + 1];

            if (frac > 0.0 && index < nhi) {
                double left2 = m_buffer[(index + 1) * 2];
                double right2 = m_buffer[(index + 1) * 2 + 1];
                return {left1 + frac * (left2 - left1), right1 + frac * (right2 - right1)};
            }
            return {left1, right1};
        }
    } else {
        // 4-point Lagrange interpolation (INTERP_LAGRANGE)
        // Need samples at indices: index-1, index, index+1, index+2

        // Determine valid sample indices with boundary handling
        int idx_m1 = std::max(nlo, index - 1);
        int idx_0 = index;
        int idx_1 = std::min(nhi, index + 1);
        int idx_2 = std::min(nhi, index + 2);

        // Calculate Lagrange basis polynomials for x = frac (in range [0, 1])
        // h0(x) = -x*(x-1)*(x-2)/6
        // h1(x) = (x+1)*(x-1)*(x-2)/2
        // h2(x) = -(x+1)*x*(x-2)/2
        // h3(x) = (x+1)*x*(x-1)/6
        double x = frac;
        double h0 = -x * (x - 1.0) * (x - 2.0) / 6.0;
        double h1 = (x + 1.0) * (x - 1.0) * (x - 2.0) / 2.0;
        double h2 = -(x + 1.0) * x * (x - 2.0) / 2.0;
        double h3 = (x + 1.0) * x * (x - 1.0) / 6.0;

        if (m_channels == 1) {
            // Mono file: read 4 samples and duplicate to stereo
            double y_m1 = m_buffer[idx_m1];
            double y_0 = m_buffer[idx_0];
            double y_1 = m_buffer[idx_1];
            double y_2 = m_buffer[idx_2];

            double interpolated = y_m1 * h0 + y_0 * h1 + y_1 * h2 + y_2 * h3;
            return {interpolated, interpolated};
        } else {
            // Stereo file: interpolate left and right separately
            double left_m1 = m_buffer[idx_m1 * 2];
            double left_0 = m_buffer[idx_0 * 2];
            double left_1 = m_buffer[idx_1 * 2];
            double left_2 = m_buffer[idx_2 * 2];

            double right_m1 = m_buffer[idx_m1 * 2 + 1];
            double right_0 = m_buffer[idx_0 * 2 + 1];
            double right_1 = m_buffer[idx_1 * 2 + 1];
            double right_2 = m_buffer[idx_2 * 2 + 1];

            return {left_m1 * h0 + left_0 * h1 + left_1 * h2 + left_2 * h3,
                    right_m1 * h0 + right_0 * h1 + right_1 * h2 + right_2 * h3};
        }
    }
}
