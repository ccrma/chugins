#ifndef AbcGenMidi_h
#define AbcGenMidi_h

#include "Abc.h"
#include "AbcMidiFile.h"
#include "AbcMidiTrackCtx.h"
#include <string>
#include <vector>

/* 
 * AbcGenMidi can be coupled with InitState from AbcStore
 * to produce a midi file conversion from an input abc file.
 * Since the original programs (abc2midi) were an accretion 
 * of manyyears, the task of refactoring the code to support
 * multiple [AbcParser + AbcStore + AbcGenMidi} contexts
 * was a bit of brain surgery.
 * 
 * One porting strategy was to "hide" all our externs that 
 * were accessed by AbcStore and AbcParser. Moreover the
 * mechanical task of qualifying all non-stack references
 * by their enclosing context helped with the surgery.
 * Once complete and solid, it might be argued that removing
 * the extra qualifiers will make the code more readable.
 * 
 * In short, this is the justification for the WriteContext
 * and InitState classes as well as the MidiWrite interface.
 * The WriteContext can be eliminated in favor of private
 * member variables of AbcGenMidi, once all parse-time
 * updates have been removed.
 */

class AbcGenMidi : public AbcMidiFile::IFileHelper
{

public:
    AbcGenMidi();
    ~AbcGenMidi();

public:
    enum Tracktype
    {
        NOTES, 
        WORDS, 
        NOTEWORDS, 
        GCHORDS, 
        DRUMS, 
        DRONE
    };

    // a voicecontext can produce multiple midi tracks
    // since words, drums, etc may be implied within a voicecontext
    struct Track 
    {
        Tracktype tracktype;
        int voicenum;
        int midichannel; 
        int featureIndexBegin;
        int featureIndexEnd;
        // current index managed by AbcMidiTrackCtx
    };

public:
    Track trackdescriptor[AbcMidiTrackCtx::MAXTRACKS];
    int ntracks;

    /* directly updated by AbcParser during operation.
     * XXX: these could be migrated to parser and exposed
     * via InitState (to better separate final state from
     * intermediate state).
     */
    std::string partspec;
    std::string logPrefix;
    int parts; 
    int part_start[26];

    // we overload (via modulo) MIDI channels when numTracks > MAXCHANS
    int channel_in_use[AbcMidiTrackCtx::MAXTRACKS]; 
    int additive;

    /* for handling stress models */
    int nseg, segnum, segden;
    int ngain[32];
    float maxdur, fdur[32], fdursum[32];
    int stressmodel;
    int stress_pattern_loaded;
    int barflymode;

    int beatmodel; /* flag selecting standard or Phil's model */

public:
    void Init(bool forPerformance, char const *logPrefix); // false means write file

    // public methods invokedb y AbcStore during abcfile parsing...
    // TODO: inprove distinction between abcparse-time and midigen-time state.

    int parse_stress_params(char *s);
    void readstressfile(char const *filepath);
    void calculate_stress_parameters(int time_num, int time_denom);

    void set_meter(int n, int m);
    void zerobar();
    void addBarUnits(int num, int denom);  // adds a/b to count of units in bar
    void getBarUnits(int *num, int *denom);
    void set_gchords(char const *);
    void set_drums(char const *);
    void drum_map(int midipitch, int mapto);

public:
    // callbacks from AbcMidiFile
    long writetrack(int xtrack) override; 
    void midierror(char const *) override;

    int findvoice(int initplace, int voice, int xtrack);
    int findvoiceEnd(int start, int voice, int xtrack);

    int findchannel();

public: // called from AbcMidiTrackCtx (XXX: refactor)
    void midi_re_tune(int channel);
    void text_data(char const *s);
    void noteon_data(int pitch, int pitchbend, int chan, int vel);
    void midi_noteoff(long delta_time, int pitch, int chan);

    //  Depending on AbcStore::argv we either write a file (abc2midi)
    //  or 'perform' the post-parse featurelist.  These calls are
    //  made by AbcStoreDriver.cpp
    int writefile(char const *filepath, Abc::InitState const *init);

    // When peforming (and after parsing/storing is complete) the
    // client may invoke these routines to trigger event generation
    // through the MidiWriter interface.
    //
    // getNextPerformanceEvents: ret: 1 active, 0 inactive
    int beginPerformance(Abc::InitState const *init);
    int getNextPerformanceEvents(int track, class IMidiWriter *); 
    int rewindPerformance();

private:
    void assignVoiceBounds();
    int processFeature(int j, int xtrack, MidiEvent *mevt=nullptr);

    void parse_drummap(char const **);
    void fillvoice(int partno, int xtrack, int voice);

    int findpart(int place);
    int partbreak(int xtrack, int voice, int place);

    void write_syllable(int);
    void checksyllables();
    int findwline(int startline);
    int getword(int *place, int w);

    void checkbar(int pass);
    void softcheckbar(int pass);
    int inlist(int place, int passno);

    void noteon(int, Abc::FeatureDesc &);
    void midi_noteon(long delta_time, int pitch, int pitchbed, int chan, int vel);
    void note_beat(int n, int *vel);

    void stress_factors(int n, int *vel);
    int set_velocity_for_one_note();
    int apply_velocity_increment_for_one_note(int vel);
    void articulated_stress_factors(int n,  int *vel);

    void configure_gchord();
    int makechordchannels(int);

    void write_program(int, int);

    void start_drone();
    void stop_drone();

    char const *select_channel(int *, char const *);
    void expand_array(int shortarray[], int size, int longarray[], int factor);
    void dodeferred(char const *s, int noteson);
    void pedal_on();
    void pedal_off();

private:
    Abc::InitState const *initState;
    bool performing;

private:
    IMidiWriter *midi;
    AbcMidiTrackCtx *wctx; // for write-file where we write tracks sequentially
    std::vector<AbcMidiTrackCtx> trackPool; // >1 when performing, 1 when writing a file
};

#endif