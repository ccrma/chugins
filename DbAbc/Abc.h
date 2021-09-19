#ifndef Abc_h
#define Abc_h

#include <string>
#include <vector>

/* Abc.h - .cpp version of abc.h */
/* Copyright Dana Batali 2021 */
/* Copyright James Allwright 2000 */
/* may be copied under the terms of the GNU public license */
namespace Abc
{
    enum TimeSig
    {
        TIMESIG_NORMAL,
        TIMESIG_FREE_METER,
        TIMESIG_COMMON,
        TIMESIG_CUT,
        TIMESIG_COMPLEX
    };

    struct TimeSigDetails
    {
        void Init()
        {
            this->type = TIMESIG_FREE_METER;
            this->num = 4;
            this->denom = 4;
            this->complex_values[0] = 4;
            this->num_values = 1;
        }
        TimeSig type;
        int num;
        int denom;
        int complex_values[8];
        int num_values;
    };

    /* define types of abc object */
    enum FeatureType
    {
        /* types of bar sign */
        SINGLE_BAR,
        DOUBLE_BAR,
        DOTTED_BAR,
        BAR_REP,
        REP_BAR,
        PLAY_ON_REP,
        REP1,
        REP2,
        /* BAR1 = SINGLE_BAR + REP1 */
        /* REP_BAR2 = REP_BAR + REP2 */
        BAR1,
        REP_BAR2,
        DOUBLE_REP,
        THICK_THIN,
        THIN_THICK,
        /* other things */
        PART,
        TEMPO,
        TIME,
        KEY,
        REST,
        TUPLE,
        /* CHORD replaced by CHORDON and CHORDOFF */
        NOTE,
        NONOTE,
        OLDTIE,
        TEXT,
        SLUR_ON,
        SLUR_OFF,
        TIE,
        CLOSE_TIE,
        TITLE,
        CHANNEL,
        TRANSPOSE,
        RTRANSPOSE,
        GTRANSPOSE,
        GRACEON,
        GRACEOFF,
        SETGRACE,
        SETC,
        SETTRIM,
        EXPAND,
        GCHORD,
        GCHORDON,
        GCHORDOFF,
        VOICE,
        CHORDON,
        CHORDOFF,
        CHORDOFFEX,
        DRUMON,
        DRUMOFF,
        DRONEON,
        DRONEOFF,
        SLUR_TIE,
        TNOTE,
        /* broken rhythm */
        LT,
        GT,
        DYNAMIC,
        LINENUM,
        MUSICLINE,
        MUSICSTOP,
        WORDLINE,
        WORDSTOP,
        INSTRUCTION,
        NOBEAM,
        CHORDNOTE,
        CLEF,
        PRINTLINE,
        NEWPAGE,
        LEFT_TEXT,
        CENTRE_TEXT,
        VSKIP,
        COPYRIGHT,
        COMPOSER,
        ARPEGGIO,
        SPLITVOICE,
        META,
        PEDAL_ON,
        PEDAL_OFF,
        EFFECT,

        NUM_FEATURES
    }; 

    char const *featname(FeatureType f);

    /* note decorations */
    // .MLRH~Tuv'OPS matches AbcParser::decorations
    enum NoteDecorations
    {
        STACCATO = 0, // .
        TENUTO,       // M
        LOUD,         // L
        ROLL,         // R
        FERMATA,      // H
        ORNAMENT,     // ~
        TRILL,        // T
        BOWUP,        // u
        BOWDOWN,      // v
        BREATH,       // '
        CODA,         // O
        UPPERMORDENT, // P
        SEGNO,        // S


        DECSIZE      
    };

    /* general purpose storage structure. A collection of i
     * features cross boundaries between tracks, 
     * ie: it contains notes for all channels.
     */
    struct FeatureDesc
    {
        FeatureDesc()
        {
            this->bentpitch = 0; // 8192 is a good default only for NOTES
            this->decotype = 0;
            this->stressvelocity = -1;
            this->pitchline = -1;
            this->charloc = 0;
        }

        // copy constructor for debugging
        //  (currently never implicitly invoked)
        FeatureDesc(FeatureDesc const &rhs)
        {
            this->feature = rhs.feature;
            this->pitch = rhs.pitch;
            this->num = rhs.num;
            this->denom = rhs.denom;
            this->bentpitch = rhs.bentpitch;
            this->stressvelocity = rhs.stressvelocity;
            this->decotype = rhs.decotype;
            this->pitchline = rhs.pitchline;
            this->charloc = rhs.charloc;
        }

        FeatureType feature;
        int pitch;
        int num;
        int denom;
        int bentpitch; // microtones
        int stressvelocity;
        int decotype; // ROLLS, TRILLS, etc
        int pitchline; // file location when ties are in play
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

        int nfeatures; // currently featurelist.size() >= nfeatures
        std::vector<FeatureDesc> &featurelist;

        std::vector<std::string> &atext;
        std::vector<Chord> &chords;
        std::vector<std::string> &words;
        int verbose;
        int silent;
        int quiet;
        int programbase;
        bool voicesused;
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
        int lineno; // at the end of parsing this subfile.
    };
};

#endif