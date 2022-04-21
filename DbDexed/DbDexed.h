#ifndef DbDexed_h
#define DbDexed_h

#include "dexed/EngineMkI.h"
#include "dexed/PluginFx.h"
#include "dexed/PluginData.h" // cartridge
#include "dexed/msfa/lfo.h"

#include <memory>

class DbDexed
{
public:
    DbDexed(double sampleRate);
    ~DbDexed();

    void AddNoteOn(int note, float vel);
    void AddNoteOff(int note, float vel);
    void AddMidiEvent(uint8_t status, uint8_t data1, uint8_t data2);

    void LoadCartridge(char const *path);
    void SetCurrentProgram(int index);

    void GetSamples(int numSamples, float *buffer);

private:
    double m_sampleRate;

    #define k_MaxActiveVoices 16

    struct Voice
    {
        int channel;
        int midi_note;
        int velocity;
        bool keydown;
        bool sustained;
        bool live;

        int mpePitchBend;
        int mpePressure;
        int mpeTimbre;
    
        class Dx7Note *dx7_note; // delay construction 'til synthTuningState is known
    } m_voices[k_MaxActiveVoices];
    Lfo m_lfo; // could migrate the lfo into ProcessorVoice
    EngineMkI m_engine;
    PluginFx m_fx;
    std::shared_ptr<class TuningState> m_synthTuningState;
    Controllers m_controllers;

    int m_currentNote; // for voice allocation
    bool m_sustain;
    bool m_monoMode; // ie: not polyphonic (as for glisando, etc)
    int m_extra_buf_size;
    float m_extra_buf[N]; // N is k_RenderChunkSize, defined in synth.h

    // cartridge has 32 programs, loaded from preset file.
    // from a loaded cart, we nominate and extract single program.
    Cartridge m_cartridge; 
    int m_currentProgramIndex;
    uint8_t m_currentProgram[161];
    void unpackOpSwitch(char); // upack 6 operator switches (for algorithm)

    /* midi key event -------------------- */
    bool m_normalizeDxVelocity;
    void keydown(uint8_t channel, uint8_t pitch, uint8_t vel);
    void keyup(uint8_t chan, uint8_t pitch, uint8_t vel);
    int tuningTranspositionShift();

};

#endif