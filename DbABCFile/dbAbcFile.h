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

    int Open(std::string const &filepath);
    int Close();

    int GetNumTracks() { return this->m_numTracks; }
    
    int Read(int track1); // called in loop
    int Rewind();

private:
    int m_numTracks;
};

#endif