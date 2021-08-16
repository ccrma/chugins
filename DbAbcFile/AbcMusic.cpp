#include "AbcMusic.h"

struct clef_item
{
  char const *name;
  AbcMusic::BasicClefType basic_clef;
  int staveline;
  int octave_offset;
};

/* entries are name, basic_clef, staveline, octave_offset */
static const clef_item clef_conversion_table[] = 
{
    {"treble", AbcMusic::basic_clef_treble, 2, 0},
    {"alto", AbcMusic::basic_clef_alto, 3, 0},
    {"bass", AbcMusic::basic_clef_bass, 4, 0},
    {"soprano", AbcMusic::basic_clef_alto, 1, 0},
    {"mezzosoprano", AbcMusic::basic_clef_alto, 2, 0},
    {"tenor", AbcMusic::basic_clef_alto, 4, 0},
    {"baritone", AbcMusic::basic_clef_bass, 0, 0},
};

#define NUM_CLEFS (sizeof(clef_conversion_table)/sizeof(clef_item))

/* The following 3 are not really clefs, but abc standard
 * 2.2 allows them. We treat them as treble clef and only
 * allow them after clef= . This ensures that K:none is not
 * interpreted as a clef instead of a key signature.
 *
 * the clef defines how a pitch value maps to a y position
 * on the stave. If there is a clef of "none", then you don't
 * know where to put the notes!
 */
static const clef_item odd_clef_conversion_table[] = 
{
    {"auto", AbcMusic::basic_clef_treble, 2, 0},
    {"perc", AbcMusic::basic_clef_treble, 2, 0},
    {"none", AbcMusic::basic_clef_treble, 2, 0}
};

#define NUM_ODD_CLEFS (sizeof(odd_clef_conversion_table)/sizeof(clef_item))

static int get_clef_octave_offset(char const *clef_ending)
{
    if(strncmp(clef_ending, "+8", 2) == 0)
        return 1;
    else
    if(strncmp(clef_ending, "+15", 2) == 0)
        return 2;
    else
    if(strncmp(clef_ending, "-8", 2) == 0)
        return -1;
    else
    if(strncmp(clef_ending, "-15", 2) == 0)
        return -2;
    else
        return 0;
}

/* These are musical modes. Each specifies a scale of notes starting on a
 * particular tonic note. Effectively the notes are the same as used in a
 * major scale, but the starting note is different. This means that each
 * mode can be notated using the standard key signatures used for major
 * keys. The abc standard specifies that only the first 3 characters of
 * the mode are significant, and further that "minor" can be abbreviated
 * as "m" and "major" omitted altogether. The full names are:
 * Major, Minor, Aeolian, Locrian, Ionian, Dorian, Phyrgian, Lydian and
 * Mixolydian. In addition, we have "exp" short for "explicit" to indicate
 * that arbitrary accidentals can be applied to each stave line in the
 * key signature and "" the empty string to represent "major" where this
 * is inferred as the default value rather than being supplied.
 */
char const *AbcMusic::mode[12] = 
{ 
    "maj", "min", "m",
    "aeo", "loc", "ion", 
    "dor", "phr", "lyd", 
    "mix", "exp", ""
};

/* This is a table for finding the sharps/flats representation of
 * a key signature for a given mode.
 * Suppose we want to find the sharps/flats representation for
 * K:GDor
 * If we know the major mode key K:G is 1 sharp, and have the index of the
 * mode we want within the mode table, we can work out the new key
 * signature as follows:
 *
 * Original major mode key signature +1 (1 sharp)
 * Desired new mode Dorian is at position 6 in table
 * modeshift[6] is -2.
 * new key signature is 1 - 2 = -1 (1 flat)
 * GDor is 1 flat.
 */
int const AbcMusic::modeshift[12] = 
{ 
    0, -3, -3, 
    -3, -5, 0, 
    -2, -4, 1, 
    -1, 0, 0 
};

/* ------------------------------------------------------------------------ */

int
AbcMusic::ClefType::InitStandard(char const *nm)
{
    int i;
    int len;
    for(i = 0; i < NUM_CLEFS; i++) 
    {
        const clef_item *table_row = &clef_conversion_table[i];
        len = strlen(table_row->name);
        if(strncmp(nm, table_row->name, len) == 0) 
        {
            this->basic_clef = table_row->basic_clef;
            this->staveline = table_row->staveline;
            this->named = 1;
            this->octave_offset = get_clef_octave_offset(nm + len);
            return 1;                 /* lookup succeeded */
        }
    }
    return 0; // fail
}

/* look for a clef using C, F or G and a number 1 - 5  or
 * one of the specials (none, perc, auto)
 */
//int get_extended_clef_details(char *name, basic_cleftype_t * basic_clef,
//                      int *staveline, int *octave_offset)
int 
AbcMusic::ClefType::InitExtended(char const *name)
{
    int i;
    int len;
    int num;
    int items_read;

    for(i = 0; i < NUM_ODD_CLEFS; i++) 
    {
        const clef_item *table_row = &odd_clef_conversion_table[i];
        len = strlen(table_row->name);
        if(strncmp(name, table_row->name, len) == 0) 
        {
            this->basic_clef = table_row->basic_clef;
            this->staveline = table_row->staveline;
            this->octave_offset = table_row->octave_offset;
            this->named = 1;
            this->octave_offset = get_clef_octave_offset(name + len);
            return 1;                 /* lookup succeeded */
        }
    }
    this->octave_offset = 0;
    /* try [C/F/G][1-5] format */
    switch(name[0]) 
    {
    case 'C':
        this->basic_clef = basic_clef_alto;
        break;
    case 'F':
        this->basic_clef = basic_clef_bass;
        break;
    case 'G':
        this->basic_clef = basic_clef_treble;
        break;
    default:
        return 0;                 /* not recognized */
    }
    items_read = sscanf(&name[1], "%d", &num);
    if((items_read == 1) && (num >= 1) && (num <= 5)) 
    {
        /* we have a valid clef specification */
        this->staveline = num;
        this->named = 0;
        this->octave_offset = get_clef_octave_offset(name + 2);
        return 1;
    }
    return 0;
}

void AbcMusic::copy_clef(ClefType *target_clef, 
    ClefType const *source_clef)
{
    target_clef->basic_clef = source_clef->basic_clef;
    target_clef->staveline = source_clef->staveline;
    target_clef->octave_offset = source_clef->octave_offset;
    target_clef->named = source_clef->named;
}

/* --------------------------------------------------------------*/
/* elimate common factors in fraction a/b */
void
AbcMusic::reduceFraction(int *a, int *b)
{
    int sign;
    int t, n, m;
    if (*a < 0) 
    {
        sign = -1;
        *a = -*a;
    } 
    else 
    {
        sign = 1;
    }
    /* find HCF using Euclid's algorithm */
    if (*a > *b) 
    {
        n = *a;
        m = *b;
    } 
    else 
    {
        n = *b;
        m = *a;
    }
    while (m != 0) 
    {
        t = n % m;
        n = m;
        m = t;
    }
    *a = (*a/n)*sign;
    *b = *b/n;
}

/* add a/b to the count of units in the bar */
/*static*/ void
AbcMusic::addFraction(int *xnum, int *xdenom, int a, int b)
{
    *xnum = (*xnum)*b + a*(*xdenom);
    *xdenom = (*xdenom) * b;
    if (*xnum < 0 && *xdenom < 0) 
    {
        *xnum = -*xnum; 
        *xdenom = -*xdenom;
    }
    reduceFraction(xnum, xdenom);
}

/* compare two fractions  anum/adenom > bnum/bdenom */
/* returns   (a > b)                       */
int
AbcMusic::gtFraction(int anum, int adenom, int bnum,int bdenom)
{
    if((anum*bdenom) > (bnum*adenom)) 
        return 1;
    else
        return 0;
}



/* rather than compute the fifth_size in cents directly
 * we subtract the octave from the third harmonic.
 * We do it this way instead of the proportion 3/2 because the
 * octave also can be tempered. When the octave is tempered, the factor 2
 * in 3/2 might cause problems: mix of pure and tempered octaves!).
 */
float
AbcMusic::compute_fifth_size( 
    float octave_size /* in cents */, int ndiv)
{
    /* h3 (1901.955) is the 3rd harmonic (fifth+octave) represented in cents */
    double h3 = 1200.0 * log (3.0)/log(2.0);  /* 1200 * log2(3.0) */
    double x = h3 - octave_size;           /* fifth reduced by the tempered octave */
    int n = (int) (0.5 + x * ndiv / octave_size);     /* fifth in integer steps */
    float w = n * octave_size / ndiv;                   /* fifth quantized according to temperament */
    return w; /* in cents */
}