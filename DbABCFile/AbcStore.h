#ifndef AbcStore_h
#define AbcStore_h

#include "AbcParser.h"
#include "AbcGenMidi.h"
#include "Abc.h"
#include "AbcPort.h"

class AbcStore : public AbcParser::EventHandler
{
public:
    static char const *version; 
    static const int MAXLINE = 500;
    static const int INITTEXTS = 20;
    static const int INITWORDS = 20;
    static const int MAXCHANS = 16;
    static const int MAXMIDICMD = 200;

public:
    AbcStore(AbcParser *);
    virtual ~AbcStore();

    /**
     * @Override intercept parser's eventhandler methods
     */
    void init(int argc, char const *argv[], std::string *filename); 
    void text(char const *);
    void x_reserved(char);
    void split_voice();
    void linebreak();
    void startmusicline();
    void endmusicline(char);
    void comment(char const *);
    void specific(const char *package, const char *s);
    void start_extended_overlay();
    void stop_extended_overlay();
    void field(char k, char const *f);
    void words(char const *p, int continuation);
    void part(char const *s);
    void voice(int n, char const *s, AbcParser::voice_params *vp);
    void length(int l);
    void tempo(int n, int a, int b, int rel, char const *pre, char const *post);
    void timesig(Abc::TimeSigDetails *);
    void octave(int num, int local);
    void info_key(char const *key, char const *value);
    void graceon();
    void graceoff();
    void playonrep(char const *);
    void sluron(int);
    void sluroff(int);
    void tie();
    void broken(int type, int mult);
    void tuple(int n, int q, int r);
    void chord();
    void rest(int decorators[Abc::DECSIZE], int n, int m, int type);
    void mrest(int n, int m, char c);
    void chordon(int chorddecorators[]);
    void chordoff(int, int);
    void note(int decorators[Abc::DECSIZE], AbcMusic::ClefType *clef,
                char accidental, int mult, 
                char note, int xoctave, int n, int m);
    void temperament(char const *line);
    void microtone(int dir, int a, int b);
    void normal_tone();
    void handle_gchord(char const *s);
    void handle_instruction(char const *s);
    void bar(int type, char const *replist);
    void key(int sharps, char const *s, int modeindex, 
            char modmap[7], int modmul[7], fraction modmicro[7],
            int gotkey, int gotclef, char const *clefname, AbcMusic::ClefType *new_clef,
            int octave, int transpose, int gotoctave, int gottranspose,
            int explict);
    void blankline();
    void refno(int n);
    void eof();

private:
    int done_with_barloc; /* [SS] 2019-03-21 */

    /* parsing stage */
    int tuplecount, tfact_num, tfact_denom, tnote_num, tnote_denom;
    int specialtuple;
    int gracenotes;
    int headerpartlabel;
    int dotune, pastheader;
    int hornpipe, last_num, last_denom;
    int timesigset;
    int ratio_a, ratio_b;
    int velocitychange;
    int chordstart;
    int propagate_accidentals;

    int active_pitchbend;

    float temperament_dt[12];
    #define TEMPERDT -1
    #define TEMPEREQ 1
    #define TEMPERLN 2
    #define TEMPERNORMAL 0
    float octave_size; 
    float fifth_size; 
    float sharp_size; 
    float microstep_size; 
    int started_parsing;
    int v1index;
    int ignore_fermata;
    int ignore_gracenotes;
    int ignore_guitarchords;
    int separate_tracks_for_words;

    int bodystarted;
    int harpmode;
    int easyabcmode;
    char rhythmdesignator[32];
    int retuning;
    int bend;
    int comma53;
    int silent;
    int no_more_free_channels;

    void init_p48toc53();
    void convert_to_comma53(char acc, int *midipitch, int* midibend);  
    int p48toc53[50];

    struct voicecontext 
    {
        /* not to be confused with: 
        *  struct voice defined in struct.h and used only by yaps.
        *  nor
        *  voice_context used within the parser.
        */

        // default constructor is for global context.
        voicecontext()
        {
            /* xxx
              this->mtime_num = time_num;
              this->mtime_denom = time_denom;
            */
            this->hasgchords = 0;
            this->haswords = 0;
            this->hasdrums = 0;
            this->hasdrone = 0;
            this->inslur = 0;
            this->ingrace = 0;
            this->inchord = 0;
            this->chordcount = 0;
            this->lastbarloc = -1;
            this->laststart = -1;
            this->lastend = -1;
            this->thisstart = -1;
            this->thisend = -1;
            this->brokenpending = -1;
            this->tosplitno = -1;
            this->fromsplitno = -1;  
            this->last_resync_point = 0;
            this->next = nullptr;
            this->drumchannel = 0;
            this->midichannel = -1;
            this->nbars = 0;
        }

        voicecontext(
            int voiceNum, int voiceIndex, voicecontext &global, 
            int time_num, int time_denom) :
            voicecontext()
        {
            this->voiceno = voiceNum;
            this->indexno = voiceIndex;
            this->topvoiceno = voiceNum;
            this->topindexno = voiceIndex;
            this->default_length = global.default_length;
            this->active_meter_num = time_num;
            this->active_meter_denom = time_denom;
            /* xxx
              this->mtime_num = time_num;
              this->mtime_denom = time_denom;
            */
            for (int i=0; i<7; i++) 
            {
                this->basemap[i] = global.basemap[i];
                this->basemul[i] = global.basemul[i];
                this->basemic[i].num = global.basemic[i].num; /* [SS] 2014-01-08 */
                this->basemic[i].denom = global.basemic[i].denom;
                for (int j=0;j<10;j++) 
                {
                    this->workmap[i][j] = global.workmap[i][j];
                    this->workmul[i][j] = global.workmul[i][j];
                    this->workmic[i][j].num =   global.workmic[i][j].num; /* [SS] 2014-01-26 */
                    this->workmic[i][j].denom = global.workmic[i][j].denom;
                }
            }
            this->keyset = global.keyset;
            this->octaveshift = global.octaveshift;
        }

        char basemap[7], workmap[7][10];
        int basemul[7], workmul[7][10];
        fraction basemic[7],workmic[7][10];
        int keyset; /* flag to indicate whether key signature set */
        int default_length;
        int active_meter_num; /* [SS] 2012-11-08 */
        int active_meter_denom; /* [SS] 2012-11-08 */
        int voiceno; /* voice number referenced by V: command. To avoid
                      * conflicts with split voices, all split voices
                      * begin from 32. 
                      */
        int indexno; /* voice index number in the feat array. It just
                      * increments by one and depends on the order the
                      * voices are created -- including split voices.
                      */
        int topvoiceno, topindexno; /* links to original voice in the split */
        int hasgchords;
        int haswords;
        int hasdrums;
        int hasdrone;
        int inslur;
        int ingrace;
        int octaveshift;
        int lastbarloc;  /* position of last bar line parsed */
        int tosplitno,fromsplitno; /* links to split voices and source voice*/ 
        int last_resync_point;
        /* chord handling */
        int inchord, chordcount;
        int chord_num, chord_denom;
        /* details of last 2 notes/chords to apply length-modifiers to */
        int laststart, lastend, thisstart, thisend; /* for handling broken rhythms*/
        /* broken rhythm handling */
        int brokentype, brokenmult, brokenpending;
        int broken_stack[7];
        int midichannel; /* [SS] 2015-03-24 */
        voicecontext* next;
        int drumchannel;
        int nbars; /* [SS] 2019-03-18 */
    };

    voicecontext global;
    voicecontext* v;
    voicecontext* head;
    voicecontext* vaddr[64]; /* address of all voices (by v->indexno) */
    /* vaddr is only a convenience for debugging */

    /* structure for expanding a note into a TRILL, ROLL, or ORNAMENT */
    struct notestruct 
    {
        int note;
        int index;
        int notetype;
        int pitch;
        int pitchup;
        int bendup;
        int benddown;
        int pitchdown;
        int default_length;
    };

    notestruct* noteaddr[1000];
    int notesdefined;

    AbcGenMidi::trackstruct trackdescriptor[40];

    int detune_list[12];
    int dependent_voice[64]; /* flag to indicate type of voice */
    int voicecount;
    int numsplits;
    int splitdepth;

    /* storage structure for strings */
    int maxtexts;
    std::vector<std::string> atext;
    int ntexts;

    /* Named guitar chords */
    struct Chord
    {
        std::string name;
        std::vector<int> notes;
    };
    std::vector<Chord> chords;

    /* general purpose storage structure */
    int maxnotes;
    int notes; // index into following...
    struct featureDesc
    {
        featureDesc()
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
    std::vector<featureDesc> notelist;

    int verbose;
    int titlenames;
    int got_titlename;
    int namelimit;
    int xmatch;
    int sf, mi;
    int gchordvoice, wordvoice, drumvoice, dronevoice;
    int ratio_standard; /* flag corresponding to -RS parameter */
    /* when ratio_standard != -1 the ratio for a>b is 3:1 instead of 2:1 */
    int quiet; /* if not -1 many common warnings and error messages */
               /* are suppressed.*/
    int fermata_fixed; /* flag on how to process fermata */
    int apply_fermata_to_chord; 

    /* Part handling */
    AbcGenMidi genMidi;

    int voicesused;

    /* Tempo handling (Q: field) */
    int time_num, time_denom;
    int mtime_num, mtime_denom;
    long current_tempo;
    int tempo_num, tempo_denom;
    int relative_tempo, Qtempo;

    // XXX: extern int division;
    // XXX: extern int div_factor;
    int default_tempo; /* quarter notes per minutes */

    /* for get_tempo_from_name  [SS] 2010-12-07 */
    char const *temponame[19];
    int temporate[19];

    char const * abcm2psoptions[1];
    int number_of_abcm2ps_options;

    /* output file generation */
    int userfilename;
    std::string outname;
    std::string outbase;
    int check;
    int nofnop; /* for suppressing dynamics (ff, pp etc) */
    int nocom;  /* for suppressing comments in MIDI file */
    int ntracks; /* manages GenMidi.trackdesciptor[] */

    /* bar length checking */
    // XXX: extern int bar_num, bar_denom;
    int barchecking;

    /* generating MIDI output */
    int middle_c;
    int gfact_num, gfact_denom, gfact_method;  /* for handling grace notes */
    int programbase = 0;

    /* karaoke handling */
    int karaoke, wcount;
    std::vector<std::string> wordlist; // no max

    // extern int decorators_passback[DECSIZE]; 
    /* a kludge for passing
     * information from the event_handle_instruction to parsenote 
     * in parseabc.c 
     * */

    // extern int inchordflag;
    /* for reseting decorators_passback in parseabc.c */

    /* time signature after header processed */
    int header_time_num, header_time_denom;

    int dummydecorator[Abc::DECSIZE]; /* used in event_chord */
    // extern char* featname[];

    std::string csmfilename;  
    char midicmdname[MAXMIDICMD][32];
    std::string midicmd[MAXMIDICMD];
    int nmidicmd = 0;

    int extended_overlay_running;
    int next_sync_to;

    int default_middle_c;
    int default_retain_accidentals; /* [SS] 2015-08-18 */
    int default_fermata_fixed;
    int default_ratio_a;
    int default_ratio_b;

    void startfile();
    voicecontext* newvoice(int n);
    voicecontext* getvoicecontext(int splitno);
    void clearvoicecontexts();

    int getchordnumber(char const *s);

    void addfract(int *xnum, int *xdenom, int a, int b);
    void zerobar();
    void addfeature(Abc::FeatureType f,int p,int n,int d);
    void replacefeature(Abc::FeatureType f, int p, int n, int d, int loc);
    void insertfeature(Abc::FeatureType f, int p, int n, int d, int loc);
    void textfeature(Abc::FeatureType type, char const *s);
    void interchange_features(int loc1, int loc2);
    void removefeatures(int locfrom, int locto);
    void removefeature(int loc);

    int extendNotelist(int maxnotes);

    // extern long writetrack();
    void init_drum_map();
    void fix_enclosed_note_lengths(int from, int end);
    int patchup_chordtie(int chordstart,int chordend);
    void copymap(struct voicecontext* v);
    void init_stresspat();
    void beat_modifier(int);

    void dumpfeat (int from, int to); /* defined in genmidi.c */
    char *concatenatestring(char * s1,char * s2); /* defined in parseabc.c */
    void read_custom_stress_file (char *filename); /* defined in stresspat.c */

    /* --- */
    int getarg(char const *option, int argc, char const *argv[]);

private:
    void setup_chordnames();
    void addchordname(char const *s, int len, int notes[]);

    void meter_voice_update(int n, int m);
    void dump_voicecontexts(); // debugging
    int locate_voice(int start, int indexno);
    void sync_voice(voicecontext *vv, int sync_to, int ignorecurrentbar);
    void complete_all_split_voices();

    void dump_trackdescriptor();
    void setup_trackstructure();

    void dump_notestruct();
    void free_notestructs();

    void recurse_back_to_original_voice();
    int search_backwards_for_last_bar_line(int from);
    void setmap(int sf, char map[7], int mult[7]);
    void copymap(voicecontext*);
    void altermap(voicecontext *v, char modmap[7], int modmul[7], 
        fraction modmic[7]);
 
    void event_tempo(int n, int a, int b, int rel,
                char const *pre, char const *post);
    int get_tempo_from_name (char const *s);
    void tempounits(int *t_num, int *t_denom);

    void specific_in_header(char const *, char const *); // via event_specific
    void parse_drummap(char const **s);
    void parse_mididef(char const *s);
    void process_midix(char const *s);
    void midi(char const *s);

    void extract_filename(char const *s);
    void checkbreak();
    int flattenPartSpec(char const *spec, std::string *partnm);
    void lenmul(int n, int a, int b);
    void stack_broken(voicecontext *); // used for eg gracenotes
    void restore_broken(voicecontext *);
    void brokenadjust();

    void marknote();
    void marknotestart();
    void marknoteend();

    void hornp(int num, int denom);
    int barepitch(char note, char accidental, int mult, int octave);
    int pitchof_b(char note, char accidental, int mult, int octave, 
                int propagate_accs, int *pitchbend);
    int pitchof(char note, char accidental, int mult, int octave, 
                int propagate_accs);

    void doroll(char note, int octave, int n, int m, int pitch);
    void doroll_setup(char note, int octave, int n, int m, int pitch);
    void doroll_output(int i);

    void dotrill_setup(char note, int octave, int n, int m, int pitch);
    void dotrill_output(int i);

    void doornament(char note, int octave, int n, int m, int pitch);
    void makecut(int mainpitch, int shortpitch, int mainbend, int shortbend, 
            int n, int m);
    void makeharproll(int pitch, int bend, int n, int m);
    void makeharproll3(int pitch, int bend, int n, int m);

    char const *get_accidental(char const *place, char *accidental);

}; // end class

#endif