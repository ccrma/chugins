#ifndef AbcMidiTrackCtx_h
#define AbcMidiTrackCtx_h

#include "Abc.h"
#include "AbcQueue.h"

/* TrackContext represents evolving state throughout the creation of a 
 * midi file. At the point that all AbcGenMidi members are private
 * WriteContext can be disappered.
 */
class AbcMidiTrackCtx : public AbcQueue::Client
{
public:
    static const int MAXCHANS = 16; // midi channels, this is part of the spec
    static const int MAXTRACKS = 64; // voices in file
    static const int MAXPARTS = 100; // via P:
    static const int MAXCHORDNAMES = 80;
    static const int MAXLAYERS = 3;
    static const int DIV = 480; // timing

public:
    AbcMidiTrackCtx(class AbcGenMidi *g) :
        queue(this)
    {
        this->mtime_num = 4;
        this->mtime_denom = 4;
        this->time_num = 4; 
        this->time_denom = 4; 
        this->genMidi = g;
        for(int j=0; j<256; j++)
            this->drum_map[j] = j;
        this->initState = nullptr;
        this->featureIndexBegin = -1;
        this->tracknumber = -1;
        this->partno = -1;
        this->gchordnotes_size = 0;
    }

    AbcMidiTrackCtx(AbcMidiTrackCtx const &rhs) :
        AbcMidiTrackCtx(rhs.genMidi)
    {
    }

    void beginWriting(int xtrack,
        Abc::InitState const *initState, class IMidiWriter *);

    void error(char const *msg) override; 
    void warning(char const *msg); 
    void log(char const *msg); 

    void initTrack(int xtrack, 
        int featureIndexBegin=0, int featureIndexEnd=0,
        IMidiWriter *mw=nullptr);
    void rewind();

    void set_meter(int n, int m);
    void setbeat();

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

    void saveRepeatState(int voice);
    void restoreRepeatState(int *voice);

    void dogchords(int ch);
    void dodrums(int ch);
    void save_note(int num, int denom, int pitch, int pitchbend,
        int chan, int vel);


    /* interface with AbcQueue -------------------------------------- */
    int getid() override { return this->tracknumber; }
    void progress_sequence(int chan) override;
    void midi_noteoff(long delta_time, int pitch, int chan) override;
    void midi_event(long delta_time, int evt, int chan, 
                    unsigned char data[], int len) override;
    void midi_event_with_delay(long delta_time, 
                    int evt, int chan, 
                    unsigned char data[], int len) override;
                    
    void getEffectsState(long **delta_time, 
        int *bendstate, int *bendvelocity, int *bendacceleration) override;
    void getEffectsState(long **delta_time, 
        int *bendstate, int *bendnvals, int **benddata) override;
    void getEffectsState(long **delta_time, 
        int *bendstate, int *bendnvals, int **benddata,
        int *layerIndex, int **controlnvals,  int **controldefaults,
        int **controldata) override;

    /* ------------------------------------------------------------ */
    AbcQueue queue;
    AbcGenMidi *genMidi;
    Abc::InitState const *initState;
    class IMidiWriter *midi;

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
    int featureIndexCurrent, featureIndexBegin, featureIndexEnd;

    int beat;
    int loudnote, mednote, softnote;
    int beataccents;
    int velocity_increment;
    char beatstring[100];
    int nbeats;
    int channel, program;
    int current_pitchbend[MAXCHANS];
    int current_program[MAXCHANS];
    int transpose;
    int global_transpose; // init to 0

    int single_velocity_inc;
    int single_velocity;

    // track state
    int timekey;
    int trackvoice;
    int texton;
    int in_varend;
    int pass, maxpass;
    int graceflag;
    int effecton;
    int inchord;
    int note_num, note_denom;
    int tnote_num, tnote_denom;
    int expect_repeat;
    int slurring, was_slurring;

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
    int gchord_index;
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
    int controldefaults[128];  // values associated with each of 128 CCs
    int layerIndex;
    int controlcombo;

    int onemorenote;

    long partlen[26];
    int partrepno;
    int part_count[26];
    int partno, partlabel;

    int state[6];

private: // XXX: migrate more state to private for better encapsulation
    void karaokestarttrack(int xtrack);

};

#endif