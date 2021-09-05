#ifndef dbAbc_h
#define dbAbc_h

#include "AbcMidiFile.h"

#include <string>
#include <fstream>
#include <vector>
#include <deque>

/* IMidiWriter: we intercept midi events after abc parsing completes */
class dbAbc : public IMidiWriter 
{
public:
    dbAbc(unsigned int sampleRate);
    ~dbAbc();

    void Configure(std::vector<std::string> &cfg)
    {
        this->m_argv = cfg;
    }

    // if string doesn't start with 'X:', it's a filepath.
    int Open(std::string const &filepathOrString);
    int Close();

    int GetNumTracks() { return this->m_numTracks; }
    int Read(int track1, MidiEvent *); // called in loop, XXX: return/fill MidiMsg
    int Rewind();
    void SetBPM(float bpm); // set to 0 or negative to defer to abc tempo
    float GetBPM();

    /* IMidiWriter --- */
    int writeMetaEvent(long delta_time, int type, char const *data, int size) override;
    int writeMidiEvent(long delta_time, int type, int chan, char const *data, int size) override;
    int writeTempo(long tempo) override;

private:
    int clearPending(int track, MidiEvent *evt);
    long convertDur(long dt)
    {
        // dt measured in ticks  ticks * seconds/tick * samples/second => samples
        // chuck::dur is measured in samples
        return .5 + dt * this->m_samplesPerTick;
    }

    std::vector<std::string> m_argv;

    class AbcParser *m_parser;
    class AbcStore *m_store;

    bool m_ignoreTempoUpdates;
    float m_bpm;
    double m_tickSeconds;
    double m_samplesPerTick;
    int m_tempo;

    unsigned int m_sampleRate;
    int m_numTracks;
    int m_activeTrack; // -1 when not active
    int m_activePending;
    std::vector<std::deque<MidiEvent>> m_pendingEvents; // indexed by track
};

#endif