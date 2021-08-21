#include "dbAbcFile.h"
#include "AbcStore.h"

#include <cstring>
#include <iostream>
#include <cassert>

dbAbcFile::dbAbcFile() :
    m_parser(nullptr),
    m_store(nullptr),
    m_activeTrack(-1)
{}

dbAbcFile::~dbAbcFile()
{
    this->Close();
}

/*
// Now locate the track offsets and lengths. If not using time
    // code, we can initialize the "tick time" using a default tempo of
    // 120 beats per minute. We will then check for tempo meta-events
    // afterward.
    unsigned int i;
    for ( i=0; i<nTracks_; i++ ) {
        if ( !file_.read( chunkType, 4 ) ) goto error;
        if ( strncmp( chunkType, "MTrk", 4 ) ) goto error;
        if ( !file_.read( buffer, 4 ) ) goto error;
#ifdef __LITTLE_ENDIAN__
        swap32((unsigned char *)&buffer);
#endif
        length = (SINT32 *) &buffer;
        trackLengths_.push_back( *length );
        trackOffsets_.push_back( (long) file_.tellg() );
        trackPointers_.push_back( (long) file_.tellg() );
        trackStatus_.push_back( 0 );
        file_.seekg( *length, std::ios_base::cur );
        if ( usingTimeCode_ ) tickSeconds_.push_back( (double) (1.0 / tickrate) );
        else tickSeconds_.push_back( (double) (0.5 / tickrate) );
 */

int
dbAbcFile::Open(std::string const &fp)
{
    this->Close();
    this->m_parser = new AbcParser();
    this->m_store = new AbcStore(this->m_parser);
    char const *argv[] = {"dbAbcFile", fp.c_str(), "-perform", "-v", "6"};

    std::string filename;
    const int argc = 3; // set to 2 to output file, 3 for normal, 5 for verbose
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
        // tempo-only. There, a tempo-map can trigger tempo changes. 
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

    // printf("Read track %d begin\n", track);

    int r = 0;
    this->m_activeTrack = track;
    if(this->clearPending(track, evt))
        r = 1;
    else
    {
        r = 0;
        // getNextPerfEvents returns active.  It doesn't necessarily
        // mean that it produced any Midi in the process. Alternatively
        // it may have produced multiple midi events.  We wish to ensure
        // that until inactive we deliver caller a non-zero result which
        // only occurs as a side-effect of clearPending.
        while(this->m_store->genMidi.getNextPerformanceEvents(track, this))
        {
            if(this->m_activePending)
                break;
        }
        r = this->clearPending(track, evt);
        this->m_activeTrack = -1;
    }

    // printf("Read track %d end\n", track);
    return r;
}

int
dbAbcFile::clearPending(int track, MidiEvent *evt)
{
    std::deque<MidiEvent> &equeue = this->m_pendingEvents[track];
    if(equeue.size())
    {
        MidiEvent &e = equeue.front();
        equeue.pop_front();
        *evt = e;
        this->m_activePending = equeue.size();
        return 1;
    }
    else
        this->m_activePending = 0;
    return 0;
}

/* -------------------------------------------------------------------- */
/*
 * ch 0 of multichan file:
    writeMeta: 0 1 (10) ()  - annotation
    writeTempo: 200000
    writeMeta: 0 89 (2) (keysig: sharps, minor)
    writeMeta: 0 88 (4) (meter: size:4)
*/
int
dbAbcFile::writeTempo(long tempo)
{
    assert(this->m_activeTrack != -1);
    // printf("writeTempo: %ld\n", tempo);
    return 0;
}

int
dbAbcFile::writeMetaEvent(long dt, int type, char const *data, int size)
{
    assert(this->m_activeTrack != -1);
    std::deque<MidiEvent> &equeue = this->m_pendingEvents[this->m_activeTrack];
    MidiEvent mevt(dt, type, data, size);
    equeue.push_back(mevt);
    this->m_activePending = equeue.size();
    return 0;
}

int
dbAbcFile::writeMidiEvent(long dt, int type, int chan, char const *data, int size)
{
    assert(this->m_activeTrack != -1);
    std::deque<MidiEvent> &equeue = this->m_pendingEvents[this->m_activeTrack];
    MidiEvent mevt(dt, type, data, size);
    equeue.push_back(mevt);
    this->m_activePending = equeue.size();
    return 0;
}
