#ifndef AbcMusic_h
#define AbcMusic_h

#include <string>

namespace AbcMusic
{
    enum NoteLetter 
    {
        note_c = 0,
        note_d = 1,
        note_e = 2,
        note_f = 3,
        note_g = 4,
        note_a = 5,
        note_b = 6
    };

    #define MODE_DEFAULT_MAJOR 11
    #define MODE_EXP 10

    /* There are only 3 different types of drawn clef. The other clefs are
    * obtained by drawing one of the basic clefs, but sitting on a different
    * stave line. For convenience, the abc standard 2.2 numbers these lines
    * 1 to 5, with the bottom line being 1 and the top line being 5.
    */
    enum BasicClefType 
    {
        basic_clef_treble,
        basic_clef_bass,
        basic_clef_alto,
        basic_clef_undefined, /* for when we didn't find a clef */
        basic_clef_auto, /* drawing program has free choice of clef */
        basic_clef_perc, /* percussion */
        basic_clef_none  /* from abc standard 2.2 what does this mean ? */
    };

    struct ClefType 
    {
        // return 1 on success, 0 on fail
        // choices: treble, alto, bass, soprano, mezzosoprano, tenor, baritone
        ClefType()
        {
            this->InitStandard("treble");
        }
        int InitStandard(char const *name); 
        int InitExtended(const char *s);

        BasicClefType basic_clef;
        int staveline;
        int octave_offset;
        int named;
    };

    static const char *mode[12];
    static const int modeshift[12];

    /* note operations */
    static NoteLetter note_index(char note_ch);
    static int semitone_value_for_note(NoteLetter note);
    static int semitone_shift_for_acc(char acc);
    static void note_for_semitone(int semitones, NoteLetter *note, char *accidental);

    /* clef operations */
    static void init_new_clef(ClefType *new_clef);
    static void copy_clef(ClefType *target_clef, ClefType *source_clef);
    static int get_standard_clef (const char *name, ClefType *new_clef);
    static int get_extended_clef_details (const char *name, ClefType *new_clef);
    static int get_clef_name (ClefType *new_clef, char *name);

    static void reduceFraction(int *num, int *denom);
    static float compute_fifth_size (float octave_size, int ndiv);

};

#endif