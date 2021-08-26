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

    struct fraction
    {
        fraction()
        {
            num = 0;
            denom = 0;
        }
        int num;
        int denom;
    };

    extern const char *mode[12];
    extern const int modeshift[12];

    /* note operations */
    NoteLetter note_index(char note_ch);
    int semitone_value_for_note(NoteLetter note);
    int semitone_shift_for_acc(char acc);
    void note_for_semitone(int semitones, NoteLetter *note, char *accidental);

    /* clef operations */
    void init_new_clef(ClefType *new_clef);
    void copy_clef(ClefType *target_clef, ClefType const *source_clef);
    int get_standard_clef (const char *name, ClefType *new_clef);
    int get_extended_clef_details (const char *name, ClefType *new_clef);
    int get_clef_name (ClefType *new_clef, char *name);

    void reduceFraction(int *num, int *denom);
    void addFraction(int *xnum, int *xdenom, int a, int b);
    int gtFraction(int anum, int adenom, int bnum, int bdenom);

    float compute_fifth_size (float octave_size, int ndiv);
};

#endif