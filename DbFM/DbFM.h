#ifndef DbFM_h
#define DbFM_h

#include "dexed/EngineMkI.h"
#include "dexed/PluginFx.h"
#include "dexed/msfa/lfo.h"

#include <memory>

class DbFM
{
public:
    DbFM(double sampleRate);

    void AddNoteOn(int note, float vel);
    void AddNoteOff(int note, float vel);
    void AddMidiEvent(uint8_t status, uint8_t data1, uint8_t data2);

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

    float m_extra_buf[N]; // N is k_RenderChunkSize
    int m_extra_buf_size;
};

#endif