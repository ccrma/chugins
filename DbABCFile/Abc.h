#ifndef Abc_h
#define Abc_h

/* Abc.h - .cpp version of abc.h */
/* Copyright Dana Batali 2021 */
/* Copyright James Allwright 2000 */
/* may be copied under the terms of the GNU public license */
namespace Abc
{
    enum ProgramName
    {
        ABC2MIDI,
        ABC2ABC,
        YAPS,
        ABCMATCH
    };

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
    enum NoteDecorations
    {
        STACCATO = 0,
        TENUTO,
        LOUD,
        ROLL,
        FERMATA,
        ORNAMENT,
        TRILL,
        BOWUP,
        BOWDOWN,
        BREATH,
        CODA,
        UPPERMORDENT,
        SEGNO,
        DECSIZE
    };
};

#endif