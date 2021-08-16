#ifndef dbAbcFile_h
#define dbAbcFile_h

#include <string>
#include <fstream>
#include <vector>

class dbABCFile
{
public:
    dbABCFile();
    ~dbABCFile();

    int Open(std::string const &filepath);
    int Close();

    int GetNumTracks() { return this->m_numTracks; }
    
    int Read(int track1); // called in loop, XXX: return/fill MidiMsg
    int Rewind();

private:
    int m_numTracks;
};

#endif