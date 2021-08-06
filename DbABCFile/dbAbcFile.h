#ifndef dbABCFile_h
#define dbABCFile_h

#include <string>
#include <fstream>
#include <vector>

// OBJECTS_ABC2MIDI:
//
// parseabc.o store.o genmidi.o midifile.o queues.o parser2.o stresspat.o music_utils.o
// depends: abc.h parseabc.h config.h

class dbABCFile
{
public:
    dbABCFile();
    ~dbABCFile();

    int Open(std::string filepath);
    int Close();

    unsigned long GetNextMidiEvent(std::vector<unsigned char> *midiEvent, 
        unsigned track = 0);

private:
    unsigned long getNextEvent(std::vector<unsigned char> *midiEvent, 
        unsigned track);

private:
    std::ifstream m_file;
    unsigned m_numTracks;
    float m_bpm;

};

#endif