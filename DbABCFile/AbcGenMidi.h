#ifndef AbcGenMidi_h
#define AbcGenMidi_h

#include "Abc.h"
#include "AbcMidiFile.h"
#include "AbcQueue.h"
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

class AbcGenMidi : public AbcMidiFile::IMidiWriter
{
public:
    static const int MAXCHORDNAMES = 80;
    static const int MAXPARTS = 100;
    static const int MAXCHANS = 16;
    static const int DIV = 480;
    static const int MAXTRACKS = 40;
    static const int MAXLAYERS = 3;

public:
    AbcGenMidi();
    ~AbcGenMidi();

    /* general purpose storage structure. A collection of i
     * features cross boundaries between tracks, 
     * ie: it contains notes for all channels.
     */
    struct FeatureDesc
    {
        FeatureDesc()
        {
            this->bentpitch = 0;
            this->decotype = 0;
        }

        Abc::FeatureType feature;
        int pitch;
        int num;
        int denom;
        int bentpitch; // microtones
        int stressvelocity;
        int pitchline; // file location
        int decotype; // ROLLS, TRILLS, etc
        int charloc; // character position in abctune
    };

    struct Chord // used by AbcGenMidi
    {
        std::string name;
        std::vector<int> notes;
    };

    struct InitState
    {
        InitState(
            int nfeat, 
            std::vector<FeatureDesc> &fdlist,
            std::vector<std::string> &atxt,
            std::vector<Chord> &gchords,
            std::vector<std::string> &wrds
        ) :
            nfeatures(nfeat),
            featurelist(fdlist),
            atext(atxt),
            chords(gchords),
            words(wrds)
        {}

        int nfeatures;
        std::vector<FeatureDesc> &featurelist;
        std::vector<std::string> &atext;
        std::vector<Chord> &chords;
        std::vector<std::string> &words;
        int verbose;
        int silent;
        int quiet;
        int programbase;
        int voicesused;
        int ntracks;
        int tempo;
        int time_num;
        int time_denom;
        int mtime_num;
        int mtime_denom;
        int keySharps;
        int keyMinor;
        int karaoke;
        int wcount;
        int retuning;
        int bend;
        int *dependent_voice; /* flag to indicate type of voice (len 64) */
        int barchecking;
    };

    int writefile(char const *filepath, InitState const *init);

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
    struct Track 
    {
        Tracktype tracktype;
        int voicenum;
        int midichannel; 
    };

    Track trackdescriptor[MAXTRACKS];
    int ntracks;

    /* directly updated by AbcParser during operation.
     * XXX: these could be migrated to parser and exposed
     * via InitState (to better separate final state from
     * intermediate state).
     */
    std::string partspec;
    int parts; 
    int part_start[26];

    int channel_in_use[MAXCHANS + 3]; 
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
    void Init();

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
    // primary callback from AbcMidiFile
    int writetrack(int xtrack) override; 

private:
    void parse_drummap(char const **);

    int findvoice(int initplace, int voice, int xtrack);
    void fillvoice(int partno, int xtrack, int voice);

    int findchannel();
    int no_more_free_channels;

    int findpart(int place);
    int partbreak(int xtrack, int voice, int place);

    void write_syllable(int);
    void checksyllables();
    int findwline(int startline);
    int getword(int *place, int w);
    void text_data(char const *s);

    void checkbar(int pass);
    void softcheckbar(int pass);
    int inlist(int place, int passno);

    void noteon(int, FeatureDesc &);
    void noteon_data(int pitch,  int bentpitch, int chan, int vel);
    void midi_noteon(long delta_time, int pitch, int chan, int vel, int pitchbend);
    void midi_noteoff(long delta_time, int pitch, int chan);
    void midi_re_tune(int channel);
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
    InitState const *initState;

private:
    struct notetype 
    {
        int base;
        int chan;
        int vel;
    };
    struct dronestruct
    {
        int chan;  /* MIDI channel assigned to drone */
        int event; /* stores time in MIDI pulses when last drone event occurred*/
        int prog;  /* MIDI program (instrument) to use for drone */
        int pitch1;/* MIDI pitch of first drone tone */
        int vel1;  /* MIDI velocity (loudness) of first drone tone */
        int pitch2;/* MIDI pitch of second drone tone */
        int vel2;  /* MIDI velocity (loudnress) of second drone tone */
    }; 

    /* WriteContext represents evolving state throughout the creation of a 
     * midi file.  At the point that all AbcGenMidi members are private
     * WriteContext can be disappered.
     */
    class WriteContext : public AbcQueue::Client
    {
    public:
        WriteContext(AbcGenMidi *g) :
            queue(this)
        {
            this->genMidi = g;
            for(int j=0; j<256; j++)
                this->drum_map[j] = j;
            this->initState = nullptr;
        }

        void beginWriting(FILE *fp, InitState const *initState);

        void error(char const *msg);
        void warning(char const *msg);
        void log(char const *msg);

        void initTrack(int xtrack);
        void set_meter(int n, int m);
        void setbeat();
        void addunits(int a, int b);

        void addtoQ(int num, int denom, int pitch, int chan, 
                int effect, int d);
        void delay(int num, int denom, int c);

        void set_gchords(char const *);
        void set_drums(char const *);

        void zerobar();
        void resetBar();
        void addBarUnits(int num, int denom);  // adds a/b to count of units in bar
        void getBarUnits(int *num, int *denom);

        void write_keysig(int keySharps, int keyMinor);
        void write_meter(int t_num, int t_denom);

        void starttrack(int xtrack);
        void karaokestarttrack(int xtrack);

        void saveRepeatState(int voice, int state[6]);
        void restoreRepeatState(int *voice, int state[6]);

        void dogchords(int ch);
        void dodrums(int ch);
        void save_note(int num, int denom, int pitch, int pitchbend,
            int chan, int vel);


        /* interface with AbcQueue -------------------------------------- */
        void progress_sequence(int chan) override;
        void midi_noteoff(long delta_time, int pitch, int chan) override;
        void midi_event(long delta_time, int evt, int chan, 
                        char data[], int len) override;
        void midi_event_with_delay(long delta_time, 
                        int evt, int chan, 
                        char data[], int len) override;
                        
        void getEffectsState(long **delta_time, 
            int *bendstate, int *bendvelocity, int *bendacceleration) override;
        void getEffectsState(long **delta_time, 
            int *bendstate, int *bendnvals, int **benddata) override;
        void getEffectsState(long **delta_time, 
            int *bendstate, int *bendnvals, int **benddata,
            int *nlayers, int **controlnvals,  int **controldefaults,
            int **controldata) override;

        /* ------------------------------------------------------------ */
        FILE *fp;
        AbcGenMidi *genMidi;
        AbcMidiFile mfile;
        AbcQueue queue;
        InitState const *initState;

        long introlen, lastlen;
        // we can track abcline & position through repeats via featurelist
        int lineno; 
        int lineposition;
        int barchecking;

        /* time signatures --- */
        long tempo; // aka AbcStore::current_tempo
        int time_num, time_denom; // time-sig for tune
        int header_time_num, header_time_denom; // after header processed.
        int mtime_num, mtime_denom; // current time sig when writing
        int err_num, err_denom;

        /* bar length checking --- */
        int b_num, b_denom; // set_meter
        int bar_num, bar_denom;
        int barno, barsize;

        int div_factor;
        int division = DIV;
        long delta_time; // since last MIDI event
        long delta_time_track0;
        long tracklen, tracklen1;
        long barloc[1024];

        int beat;
        int loudnote, mednote, softnote;
        int beataccents;
        int velocity_increment;
        char beatstring[100];
        int nbeats;
        int channel, program;
        int channel_in_use[MAXCHANS];
        int current_pitchbend[MAXCHANS];
        int current_program[MAXCHANS];
        int transpose;
        int global_transpose; // init to 0

        int single_velocity_inc;
        int single_velocity;

        // karaoke handling...
        int kspace;
        char const *wordlineptr;
        int thismline, thiswline, windex, thiswfeature;
        int wordlineplace;
        int nowordline;
        int waitforbar;
        int wlineno, syllcount;
        int lyricsyllables, musicsyllables;

        /* the following are booleans to select features in current track */
        int wordson, noteson, gchordson, temposon, drumson, droneon;
        int hyphenstate;

        /* generating accompaniment --*/
        int gchords, g_started;
        int basepitch, inversion, chordnum;
        int gchordnotes[6], gchordnotes_size;

        notetype gchord, fun;
        int g_num, g_denom;
        int g_next;
        char gchord_seq[40];
        int gchord_len[40];
        int g_ptr;
        int gchordbars;

        int tracknumber; 
        dronestruct drone; 

        int notecount;   /* number of notes in a chord [ABC..] */
        int notedelay;   /* time interval in MIDI ticks between */
                         /*  start of notes in chord */
        int chordattack;
        int staticnotedelay;  /* introduced to handle !arpeggio! */
        int staticchordattack;
        int totalnotedelay; /* total time delay introduced */
        int nchordchannels; /* valid while writinga track -- */
        int chordchannels[10]; // for handling in voice chords

        int trim; /* to add a silent gap to note */
        int trim_num;
        int trim_denom;

        int expand; /* overlap note past next note */
        int expand_num;
        int expand_denom;

        /* Generating drum track */
        int drumbars;
        char drum_seq[40];
        int drum_len[40];
        int drum_velocity[40], drum_program[40];
        int drum_ptr, drum_on;
        int drum_num, drum_denom;
        int drum_map[256];

        int gchord_error; 
        int bendvelocity = 100;
        int bendacceleration = 300;
        int bendstate = 8192; /* also linked with queues.c */
        int benddata[256];
        int bendnvals;
        int bendtype;

        int controldata[MAXLAYERS][256];
        int controlnvals[MAXLAYERS];
        int controldefaults[128]; 
        int nlayers;
        int controlcombo;

        int onemorenote;

        long partlen[26];
        int partrepno;
        int part_count[26];
        int partno, partlabel;

    } wctx;
};

#endif