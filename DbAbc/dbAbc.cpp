#include "dbAbc.h"
#include "AbcStore.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <cassert>

dbAbc::dbAbc(unsigned int sampleRate) :
    m_parser(nullptr),
    m_store(nullptr),
    m_activeTrack(-1),
    m_sampleRate(sampleRate),
    m_ignoreTempoUpdates(false)
{
}

dbAbc::~dbAbc()
{
    this->Close();
}

int
dbAbc::Open(std::string const &fp)
{
    this->Close();
    this->m_parser = new AbcParser();
    this->m_store = new AbcStore(this->m_parser);

    std::vector<char const *> largv;
    largv.push_back("dbAbc"); // argv[0]

    bool openFile = true;
    std::string tmp;
    if(fp.find("X:", 0) == 0)
    {
        if(fp.find("K:", 2) == std::string::npos)
        {
            this->m_store->error("abc requires a K: field");
            return -1;
        }
        openFile = false;
        size_t s = fp.find("T:");
        if(s != std::string::npos)
        {
            size_t end = fp.find("\n", s);
            tmp = fp.substr(s, end-s-1);
            largv.push_back(tmp.c_str()); // argv[1]
        }
        else
            largv.push_back("abcstring.abc"); // argv[1]
    }
    else
        largv.push_back(fp.c_str()); // argv[1]
    largv.push_back("-perform");
    for(int i=0;i<this->m_argv.size();i++)
    {
        largv.push_back(this->m_argv[i].c_str());
    }
    #if 0 // debugging
    largv.push_back("-v");
    largv.push_back("6");
    #endif

    std::string filename;
    this->m_store->Init(largv.size(), largv.data(), &filename); // 
    std::ifstream fstr;
    std::istringstream sstr;
    std::istream *istr;
    if(openFile)
    {
        fstr.open(fp.c_str());
        istr = &fstr;
    }
    else
    {
        sstr.str(fp);
        istr = &sstr;
    }

    if(istr->good())
    {
        // because we've requested "_perform_", the abc parse result 
        // is left "hanging" within AbcStore+AbcGenMidi. Now we're read
        // for calls to getNextEvent
        this->m_parser->Parse(istr, this->m_store, AbcParser::k_AbcToMidi);
        this->m_numTracks = this->m_store->genMidi.ntracks;
        // NB: not all have "tracks", in multitrack files first track is
        // tempo-only. There, a tempo-map can trigger tempo changes. 
        //  - Is Tempo information important to client?
        //  - Should we collapse to NOTE tracks?
        this->m_pendingEvents.resize(this->m_numTracks); 

        // default tempo, client should "perform" channel 0 if they
        // want tempo changes. tempo changes (including init) are 
        // delivered through the midi handler and can occur as the 
        // performance progresses.
        this->m_activeTrack = 0;
        this->writeTempo(600000); // 100 bpm
        this->m_activeTrack = -1;
    }
    else
        std::cerr << "dbAbc: " << filename.c_str() << " not found\n";
    return 0;
}

int
dbAbc::Close()
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
dbAbc::Rewind()
{
    int r = 0;
    if(this->m_store)
    {
        this->m_activeTrack = 0;
        r = this->m_store->genMidi.rewindPerformance();
    }
    return r;
}

int
dbAbc::Read(int track, MidiEvent *evt)
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
dbAbc::clearPending(int track, MidiEvent *evt)
{
    std::deque<MidiEvent> &equeue = this->m_pendingEvents[track];
    if(equeue.size())
    {
        MidiEvent &e = equeue.front();
        *evt = e;
        equeue.pop_front(); // pop after copy-out
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
    abc hardcodes a division of 480 in AbcMidiTrackCtx::DIV
     this is the number of ticks per quarter note
    writeMeta: 0 1 (10) ()  - annotation
    writeTempo: 200000
    writeMeta: 0 89 (2) (keysig: sharps, minor)
    writeMeta: 0 88 (4) (meter: size:4)
*/
int
dbAbc::writeTempo(long tempo)
{
    assert(this->m_activeTrack != -1);
    if(this->m_ignoreTempoUpdates)
        return 0;

    this->m_tempo = tempo;
    this->m_tickSeconds = (double) (0.000001 * tempo / AbcMidiTrackCtx::DIV);
    this->m_bpm = 60000000.0f / tempo;
    this->m_samplesPerTick = this->m_tickSeconds * this->m_sampleRate;

    #if 0
    printf("writeTempo Track %d: %ld bpm: %g\n", 
            this->m_activeTrack, tempo, this->m_bpm);
    #endif
    return 0;
}

void 
dbAbc::SetBPM(float bpm)
{
    if(bpm > 0.f)
    {
        this->m_bpm = bpm;
        this->m_tempo = 60000000.0f / bpm;
        this->m_tickSeconds = (double) (0.000001 * this->m_tempo / AbcMidiTrackCtx::DIV);
        this->m_samplesPerTick = this->m_tickSeconds * this->m_sampleRate;
        this->m_ignoreTempoUpdates = true;

        #if 0
        printf("SetBPM %g %ld\n", bpm, this->m_tempo);
        #endif
    }
    else
        this->m_ignoreTempoUpdates = false;
}

float
dbAbc::GetBPM()
{
    return this->m_bpm;
}

int
dbAbc::writeMetaEvent(long dt, int type, unsigned char const *data, int size)
{
    assert(this->m_activeTrack != -1);
    std::deque<MidiEvent> &equeue = this->m_pendingEvents[this->m_activeTrack];
    MidiEvent mevt(this->convertDur(dt), MidiEvent::meta_event, type, data, size);
    equeue.push_back(mevt);
    this->m_activePending = equeue.size();
    return 0;
}

int
dbAbc::writeMidiEvent(long dt, int type, int chan, unsigned char const *data, int size)
{
    assert(this->m_activeTrack != -1);
    std::deque<MidiEvent> &equeue = this->m_pendingEvents[this->m_activeTrack];
    MidiEvent mevt(this->convertDur(dt), type, data, size);
    equeue.push_back(mevt);
    this->m_activePending = equeue.size();
    return 0;
}
