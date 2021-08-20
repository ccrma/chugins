#include "dbAbcFile.h"
#include "AbcStore.h"

#include <cstring>
#include <iostream>
#include <cassert>

dbAbcFile::dbAbcFile() :
    m_parser(nullptr),
    m_store(nullptr)
{}

dbAbcFile::~dbAbcFile()
{
    this->Close();
}

int
dbAbcFile::Open(std::string const &fp)
{
    this->Close();
    this->m_parser = new AbcParser();
    this->m_store = new AbcStore(this->m_parser);
    char const *argv[] = {"dbAbcFile", fp.c_str(), "-perform"};

    std::string filename;
    const int argc = 3; // set to 2 to output file (debugging)
    this->m_store->Init(argc, argv, &filename); // 
    std::ifstream istr(fp.c_str());
    if(istr.good())
    {
        // because we've requested "_perform_", the abc parse result 
        // is left "hanging" within AbcStore+AbcGenMidi. Now we're read
        // for calls to getNextEvent
        this->m_parser->Parse(&istr, this->m_store, AbcParser::k_AbcToMidi);
        this->m_numTracks = this->m_store->genMidi.ntracks;
        // NB: not all have "tracks", in multitrack files first track is
        // tempo-only. 
        //  - Is Tempo information important to client?
        //  - Should we collapse to NOTE tracks?
        this->m_pendingEvents.resize(this->m_numTracks); 
    }
    else
        std::cerr << "dbAbcFile: " << filename.c_str() << " not found\n";
    return 0;
}

int
dbAbcFile::Close()
{
    if(this->m_parser)
    {
        delete this->m_parser;
        this->m_parser = nullptr;
    }
    if(this->m_store)
    {
        delete this->m_store;
        this->m_store = nullptr;
    }
    return 0;
}

int
dbAbcFile::Rewind()
{
    int r = 0;
    if(this->m_store)
        r = this->m_store->genMidi.rewindPerformance();
    return r;
}

int
dbAbcFile::Read(int track, MidiEvent *evt)
{
    assert(this->m_pendingEvents.size() > track);
    std::deque<MidiEvent> &equeue = this->m_pendingEvents[track];
    if(equeue.size())
    {
        MidiEvent &e = equeue.front();
        equeue.pop_front();
        *evt = e;
        return 1;
    }

    int r = 0;
    if(this->m_store)
        r = this->m_store->genMidi.getNextPerformanceEvents(track, this);
    return r;
}

/* -------------------------------------------------------------------- */
int
dbAbcFile::writeTempo(long temp)
{
    return 0;
}

int
dbAbcFile::writeMetaEvent(long dt, int type, char const *data, int size)
{
    return 0;
}

int
dbAbcFile::writeMidiEvent(long dt, int type, int chan, char const *data, int size)
{
    return 0;
}


