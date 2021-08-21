#ifndef dbAbcFile_h
#define dbAbcFile_h

#include "AbcMidiFile.h"

#include <string>
#include <fstream>
#include <vector>
#include <deque>

/* IMidiWriter: we intercept midi events after abc parsing completes */
class dbAbcFile : public IMidiWriter 
{
public:
    dbAbcFile();
    ~dbAbcFile();

    int Open(std::string const &filepath);
    int Close();

    int GetNumTracks() { return this->m_numTracks; }
    
    int Read(int track1, MidiEvent *); // called in loop, XXX: return/fill MidiMsg
    int Rewind();

    int writeMetaEvent(long delta_time, int type, char const *data, int size) override;
    int writeMidiEvent(long delta_time, int type, int chan, char const *data, int size) override;
    int writeTempo(long tempo) override;

private:
    int clearPending(int track, MidiEvent *evt);

    int m_numTracks;
    int m_activeTrack; // -1 when not active
    int m_activePending;
    class AbcParser *m_parser;
    class AbcStore *m_store;
    std::vector<std::deque<MidiEvent>> m_pendingEvents; // indexed by track
};

#endif