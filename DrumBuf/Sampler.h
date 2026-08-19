//-----------------------------------------------------------------------------
// Sampler.h - Header file for the Sampler renderer
// Granular sampler with pitch-shifting and time-stretching
//-----------------------------------------------------------------------------

#ifndef SAMPLER_H
#define SAMPLER_H

#include "PolyVoiceManager.h"
#include <cstdint>
#include <sndfile.h>
#include <string>

// Envelope states
enum EnvState { ENV_IDLE = 0, ENV_ATTACK = 1, ENV_SUSTAIN = 2, ENV_RELEASE = 3 };

// Playback modes
enum PlaybackMode {
    PLAYBACK_ONESHOT = 0,   // Play once, stop at end marker
    PLAYBACK_LOOP = 1,      // Loop between loopStart/loopEnd (independent of the markers)
    PLAYBACK_PINGPONG = 2,  // Bounce between loopStart/loopEnd
    PLAYBACK_THRU = 3       // Play once, through end marker to end of file
};

// Interpolation modes (matching SndBuf API)
enum InterpolationMode {
    INTERP_DROP = 0,     // Drop/none - no interpolation (fastest)
    INTERP_LINEAR = 1,   // Linear interpolation (default)
    INTERP_LAGRANGE = 2  // 4-point Lagrange interpolation (higher quality)
};

//-----------------------------------------------------------------------------
// SamplerVoice - Per-voice playback state
//-----------------------------------------------------------------------------
struct SamplerVoice : public VoiceBase {
    // Playback state
    double playhead;           // Current position in buffer (samples)
    double velocity;           // Note velocity (0.0-1.0, normalized)
    double pingpongDirection;  // Direction: 1.0=forward, -1.0=reverse
    double stopPosition;       // Playhead position at which the voice stops (ONESHOT/THRU, file frames)
    double sliceStart;         // First frame of the ONESHOT slice this voice may read (file frames)
    double sliceEnd;           // One past the last frame of that slice (file frames)

    // Granular synthesis state
    double grainPhase1;  // Grain 1 phase (0.0-1.0)
    double grainPhase2;  // Grain 2 phase (0.0-1.0)
    double grainPos1;    // Grain 1 read position (samples)
    double grainPos2;    // Grain 2 read position (samples)

    // Envelope state
    double envValue;           // Current envelope level (0.0-1.0)
    EnvState envState;         // Envelope state machine
    uint64_t holdCounter;      // Samples elapsed since noteOn
    bool releaseRequested;     // True when noteOff called
    uint64_t ageSamples;       // System samples since noteOn (for decay)
    double currentLevel;       // Level after envelope/decay/fadeOut, before gain (for STEAL_QUIETEST)

    // Constructor - initialize to default values
    SamplerVoice();

    // Override reset from VoiceBase
    void reset() override;

    // Required for STEAL_OLDEST_RELEASE policy
    bool isReleasing() const { return envState == ENV_RELEASE; }

    // Required for STEAL_QUIETEST policy
    double getAmplitude() const { return currentLevel; }
};

//-----------------------------------------------------------------------------
// Sampler class
//-----------------------------------------------------------------------------
class Sampler {
public:
    Sampler(double fs);
    ~Sampler();

    // File loading
    bool read(const std::string& filename);

    // Playback control
    void noteOn(int note, int velocity);
    void noteOff();          // Turn off all voices
    void noteOff(int note);  // Turn off voices playing specific note
    void allNotesOff();      // Immediately stop all voices
    void setNotePosition(int note, double position);
    void setReverse(bool reverse);
    bool getReverse() const;

    // Polyphony control
    void setPolyphony(int voices);
    int getPolyphony() const;
    void setStealingPolicy(int policy);
    int getStealingPolicy() const;
    int getActiveVoices() const;

    // Query methods
    int channels() const;
    double samplerate() const;
    int samples() const;
    // Length of the loaded file in seconds; 0.0 when no file is loaded. Callers
    // need this to convert normalized marker positions into the seconds the
    // setStartMarker/setEndMarker pair expects.
    double length() const;
    bool isLoaded() const;
    std::string filename() const;
    void dump() const;  // Print loaded file information

    // Audio processing (stereo output: out[0] = left, out[1] = right)
    void tick(float* in, float* out);

    // Pitch/Time control
    void setTranspose(double semitones);
    double getTranspose() const;
    void setRate(double rate);
    double getRate() const;

    // Grain control
    void setGrainSize(double seconds);
    double getGrainSize() const;

    // Playback mode control
    void setPlaybackMode(int mode);
    int getPlaybackMode() const;
    void setLoopStart(double pos);
    double getLoopStart() const;
    void setLoopEnd(double pos);
    double getLoopEnd() const;

    // Gain control
    void setGain(double dB);
    double getGain() const;

    // Envelope control
    void setEnvelope(bool enable);
    bool getEnvelope() const;
    void setAttack(double seconds);
    double getAttack() const;
    void setHold(double seconds);
    double getHold() const;
    void setRelease(double seconds);
    double getRelease() const;

    // Velocity control
    void setVelocitySens(double sens);
    double getVelocitySens() const;
    // Per-hit shaping, independent of the AHR envelope:
    //   decay    — seconds from onset to -60 dB (exponential); 0 = off
    //   fade_out — linear fade over the last N seconds before a ONESHOT/THRU
    //              voice reaches its stop boundary (slice end); 0 = off
    void setDecay(double seconds);
    double getDecay() const;
    void setFadeOut(double seconds);
    double getFadeOut() const;

    // Marker control
    void setStartMarker(double seconds);
    double getStartMarker() const;
    void setEndMarker(double seconds);
    double getEndMarker() const;

    // Interpolation control
    void setInterp(int mode);
    int getInterp() const;

    // Note tracking control
    void setNoteTracking(int mode);
    int getNoteTracking() const;
    void setRootNote(int note);
    int getRootNote() const;

private:
    // Stereo sample struct for clean stereo handling
    struct StereoSample {
        double left;
        double right;
    };
    // Private helper methods
    void clearBuffer();
    void updateVoiceEnvelope(SamplerVoice* voice);
    void releaseVoice(SamplerVoice* voice);  // noteOff for one voice
    StereoSample tickVoice(SamplerVoice* voice);
    StereoSample generateGrain(const SamplerVoice* voice, double grainPhase, double grainPos, double regionLo,
                               double regionHi) const;
    double confineGrainPos(const SamplerVoice* voice, double pos, double regionLo, double regionHi) const;
    StereoSample readSample(double position, double regionLo, double regionHi) const;

    // ========================================================================
    // Constants
    // ========================================================================
    static constexpr int MIN_GRAIN_SAMPLES = 10;  // Minimum grain size in samples to prevent artifacts

    // ========================================================================
    // Audio System
    // ========================================================================
    double m_systemSampleRate;  // System sample rate (Hz)

    // ========================================================================
    // Sample Buffer
    // ========================================================================
    float* m_buffer;            // Audio sample buffer (interleaved if multi-channel)
    sf_count_t m_bufferFrames;  // Total number of frames in buffer
    int m_channels;             // Number of channels in loaded file (1=mono, 2=stereo)
    double m_fileSampleRate;    // File's original sample rate (for sample-rate independent marker times)
    std::string m_filename;     // Path to currently loaded file

    // ========================================================================
    // Voice Management
    // ========================================================================
    PolyVoiceManager<SamplerVoice, 64> m_voiceManager;  // Polyphonic voice manager (max 64 voices)

    // ========================================================================
    // Playback Configuration
    // ========================================================================
    bool m_reverse;  // Reverse playback toggle (affects voice initialization in noteOn)

    // ========================================================================
    // Pitch and Time Control
    // ========================================================================
    double m_transpose;  // Pitch transposition in semitones (0 = original pitch)
    double m_rate;       // Time-stretching rate (1.0 = normal speed, 0.5 = half speed)
    double m_grainSize;  // Grain size in seconds (typical: 0.05 - 0.2)

    // ========================================================================
    // Playback Mode Control
    // ========================================================================
    PlaybackMode m_playbackMode;  // Playback mode: ONESHOT, LOOP, PINGPONG, or THRU
    double m_loopStart;           // Loop start position (0.0 - 1.0, normalized to file length)
    double m_loopEnd;             // Loop end position (0.0 - 1.0, normalized to file length)

    // ========================================================================
    // Position Markers
    // ========================================================================
    double m_startMarker;  // Start marker in seconds (sample-rate independent)
    double m_endMarker;    // End marker in seconds (sample-rate independent)

    // ========================================================================
    // Interpolation Mode
    // ========================================================================
    InterpolationMode m_interpMode;  // Sample interpolation mode (LINEAR or LAGRANGE)

    // ========================================================================
    // MIDI Note Mapping
    // ========================================================================
    double m_notePositions[128];  // Start position for each MIDI note, in seconds (see setNotePosition)

    // ========================================================================
    // Note Tracking
    // ========================================================================
    int m_noteTracking;  // Note tracking mode: 0=OFF (default), 1=TRADITIONAL (coupled), 2=GRANULAR
                         // (decoupled)
    int m_rootNote;      // Root note for pitch tracking (default 60 = middle C)

    // ========================================================================
    // Gain Control
    // ========================================================================
    double m_gain;          // Linear gain multiplier (set from dB via setGain)
    double m_velocitySens;  // Velocity sensitivity (0 = none, 1 = linear, >1 = exponential)
    double m_decay;         // Per-hit exponential decay to -60 dB (s), 0 = off
    double m_fadeOut;       // Linear fade before the stop boundary (s), 0 = off

    // ========================================================================
    // AHR Envelope Configuration (Exponential with Hold)
    // ========================================================================
    bool m_envelopeEnable;  // Enable/disable AHR (Attack-Hold-Release) envelope
    double m_attackTime;    // Attack time in seconds
    double m_holdTime;      // Minimum hold time in seconds (0.0 = immediate release on noteOff)
    double m_releaseTime;   // Release time in seconds
    double m_attackCoeff;   // Pre-calculated coefficient for exponential attack (single-pole filter)
    double m_releaseCoeff;  // Pre-calculated coefficient for exponential release (single-pole filter)
};

#endif  // SAMPLER_H
