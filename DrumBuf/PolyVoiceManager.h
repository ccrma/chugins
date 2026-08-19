//-----------------------------------------------------------------------------
// PolyVoiceManager.h - Generic polyphonic voice management
// Provides voice allocation, stealing, and management using templates
//-----------------------------------------------------------------------------

#ifndef POLY_VOICE_MANAGER_H
#define POLY_VOICE_MANAGER_H

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <type_traits>

//-----------------------------------------------------------------------------
// VoiceBase - Minimal interface that all voices must implement
//-----------------------------------------------------------------------------
struct VoiceBase {
    bool active;              // True when voice is playing
    int note;                 // MIDI note number (-1 if not assigned)
    uint64_t timestamp;       // Sample counter when voice was triggered (for age tracking)

    VoiceBase() : active(false), note(-1), timestamp(0) {}

    // Virtual destructor for polymorphism
    virtual ~VoiceBase() {}

    // Reset voice to initial state (called after allocation)
    virtual void reset() {
        active = false;
        note = -1;
        timestamp = 0;
    }
};

//-----------------------------------------------------------------------------
// Voice Stealing Policy Enum
//-----------------------------------------------------------------------------
enum VoiceStealingPolicy {
    STEAL_OLDEST = 0,          // Steal oldest active voice
    STEAL_OLDEST_RELEASE = 1,  // Prefer oldest releasing voice, fallback to oldest
    STEAL_QUIETEST = 2,        // Steal voice with lowest amplitude (requires getAmplitude() in voice)
    STEAL_ROUND_ROBIN = 3      // Simple round-robin (fastest)
};

//-----------------------------------------------------------------------------
// PolyVoiceManager - Template class for managing a pool of voices
// Template parameters:
//   VoiceType: Must inherit from VoiceBase
//   MaxVoices: Maximum number of voices in the pool (default 64)
//-----------------------------------------------------------------------------
template <typename VoiceType, size_t MaxVoices = 64>
class PolyVoiceManager {
    // Compile-time check that VoiceType inherits from VoiceBase
    static_assert(std::is_base_of<VoiceBase, VoiceType>::value, "VoiceType must inherit from VoiceBase");

public:
    PolyVoiceManager()
        : m_polyphony(16), m_allocIndex(0), m_sampleCounter(0), m_stealingPolicy(STEAL_OLDEST_RELEASE) {}

    // ========================================================================
    // Configuration
    // ========================================================================

    // Set number of active voices (1 to MaxVoices). Voices above the new
    // limit are stopped: every traversal below is bounded by m_polyphony, so
    // leaving them active would strand them (unticked, uncountable, not
    // reachable by noteOff) until polyphony grows again and they resume.
    void setPolyphony(int voices) {
        int newPolyphony = std::max(1, std::min((int)MaxVoices, voices));
        for (int i = newPolyphony; i < m_polyphony; i++) {
            m_voices[i].reset();  // also clears `active`
        }
        m_polyphony = newPolyphony;
        m_allocIndex %= m_polyphony;
    }

    // Get current polyphony setting
    int getPolyphony() const { return m_polyphony; }

    // Set voice stealing policy
    void setStealingPolicy(VoiceStealingPolicy policy) { m_stealingPolicy = policy; }

    // Get current stealing policy
    VoiceStealingPolicy getStealingPolicy() const { return m_stealingPolicy; }

    // ========================================================================
    // Voice Allocation
    // ========================================================================

    // Allocate a voice for a note (finds free voice or steals one)
    VoiceType* allocateVoice(int note) {
        VoiceType* voice = findFreeVoice();
        if (!voice) {
            voice = stealVoice();
        }
        if (voice) {
            voice->reset();
            voice->note = note;
            voice->timestamp = m_sampleCounter;
            voice->active = true;
        }
        return voice;
    }

    // Find a free (inactive) voice, or nullptr if none available
    VoiceType* findFreeVoice() {
        for (int i = 0; i < m_polyphony; i++) {
            if (!m_voices[i].active) {
                return &m_voices[i];
            }
        }
        return nullptr;
    }

    // Steal a voice based on current policy
    VoiceType* stealVoice() {
        switch (m_stealingPolicy) {
        case STEAL_OLDEST_RELEASE:
            return stealOldestRelease();
        case STEAL_OLDEST:
            return stealOldest();
        case STEAL_ROUND_ROBIN:
            return stealRoundRobin();
        case STEAL_QUIETEST:
            return stealQuietest();
        default:
            return stealOldest();
        }
    }

    // ========================================================================
    // Voice Finding
    // ========================================================================

    // Find first active voice playing a specific note
    VoiceType* findVoiceByNote(int note) {
        for (int i = 0; i < m_polyphony; i++) {
            if (m_voices[i].active && m_voices[i].note == note) {
                return &m_voices[i];
            }
        }
        return nullptr;
    }

    // Find all voices playing a specific note (callback for each)
    template <typename Func>
    void forEachVoiceByNote(int note, Func callback) {
        for (int i = 0; i < m_polyphony; i++) {
            if (m_voices[i].active && m_voices[i].note == note) {
                callback(&m_voices[i]);
            }
        }
    }

    // Iterate all active voices
    template <typename Func>
    void forEachActiveVoice(Func callback) {
        for (int i = 0; i < m_polyphony; i++) {
            if (m_voices[i].active) {
                callback(&m_voices[i]);
            }
        }
    }

    // ========================================================================
    // Voice Control
    // ========================================================================

    // Deactivate all voices playing a specific note
    void releaseNote(int note) {
        for (int i = 0; i < m_polyphony; i++) {
            if (m_voices[i].active && m_voices[i].note == note) {
                m_voices[i].active = false;
            }
        }
    }

    // Deactivate all voices
    void allNotesOff() {
        for (size_t i = 0; i < MaxVoices; i++) {
            m_voices[i].active = false;
            m_voices[i].reset();
        }
    }

    // ========================================================================
    // Timing
    // ========================================================================

    // Increment sample counter (call once per audio sample)
    void tick() { m_sampleCounter++; }

    // Get current sample counter value
    uint64_t getSampleCounter() const { return m_sampleCounter; }

    // ========================================================================
    // Direct Access & Queries
    // ========================================================================

    // Get voice by index (0 to MaxVoices-1)
    VoiceType* getVoice(int index) {
        if (index >= 0 && index < (int)MaxVoices) {
            return &m_voices[index];
        }
        return nullptr;
    }

    // Count active voices
    int getActiveVoiceCount() const {
        int count = 0;
        for (int i = 0; i < m_polyphony; i++) {
            if (m_voices[i].active) count++;
        }
        return count;
    }

private:
    VoiceType m_voices[MaxVoices];         // Voice pool
    int m_polyphony;                       // Current active polyphony
    int m_allocIndex;                      // For round-robin allocation
    uint64_t m_sampleCounter;              // Global sample counter for timestamps
    VoiceStealingPolicy m_stealingPolicy;  // Current stealing policy

    // ========================================================================
    // Stealing Algorithms (implementations must be in header for templates)
    // ========================================================================

    // Steal oldest active voice
    VoiceType* stealOldest() {
        if (m_polyphony == 0) return nullptr;

        VoiceType* oldest = &m_voices[0];
        for (int i = 1; i < m_polyphony; i++) {
            if (m_voices[i].timestamp < oldest->timestamp) {
                oldest = &m_voices[i];
            }
        }
        return oldest;
    }

    // Steal oldest releasing voice, fallback to oldest
    VoiceType* stealOldestRelease() {
        // First pass: find oldest releasing voice
        VoiceType* oldestRelease = nullptr;
        for (int i = 0; i < m_polyphony; i++) {
            if (m_voices[i].active && m_voices[i].isReleasing()) {
                if (!oldestRelease || m_voices[i].timestamp < oldestRelease->timestamp) {
                    oldestRelease = &m_voices[i];
                }
            }
        }
        if (oldestRelease) return oldestRelease;

        // Fallback: steal oldest voice in any state
        return stealOldest();
    }

    // Steal using round-robin (cycles through voices)
    VoiceType* stealRoundRobin() {
        if (m_polyphony == 0) return nullptr;

        m_allocIndex %= m_polyphony;  // stay inside the active range
        VoiceType* voice = &m_voices[m_allocIndex];
        m_allocIndex = (m_allocIndex + 1) % m_polyphony;
        return voice;
    }

    // Steal quietest voice (requires getAmplitude() method in VoiceType)
    VoiceType* stealQuietest() {
        if (m_polyphony == 0) return nullptr;

        VoiceType* quietest = &m_voices[0];
        double minAmp = m_voices[0].getAmplitude();

        for (int i = 1; i < m_polyphony; i++) {
            double amp = m_voices[i].getAmplitude();
            if (amp < minAmp) {
                minAmp = amp;
                quietest = &m_voices[i];
            }
        }
        return quietest;
    }
};

#endif  // POLY_VOICE_MANAGER_H
