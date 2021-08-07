#ifndef AbcParser_h
#define AbcParser_h
/* 
 * AbcParser.h - rewrite of parseabc.[h,c] for .cpp
 * Copyright (C) 2021 Dana Batali (GNU licensed - following abcmidi lib)
 * 
 * parseabc.c
 * Copyright (C) 1999 James Allwright
 * e-mail: J.R.Allwright@westminster.ac.uk
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */
#include <fstream>
#include <vector>
#include <cstdio>

#include "Abc.h"
#include "AbcMusic.h"

struct fraction
{
    int num;
    int denom;
};

class AbcParser
{
public:
    // Event handler represents callbacks of the parser client.
    // Client instantiates an AbcParser and installs a handler
    // for these events.
    struct voice_params  // passed to client in voice event
    {
        int gotclef;
        int gotoctave;
        int gottranspose;
        int gotname;
        int gotsname;
        int gotmiddle;
        int gotother;  /* [SS] 2011-04-18 */
        int octave;
        int transpose;
        char clefname[30];
        AbcMusic::ClefType new_clef;
        std::string namestring;
        std::string snamestring;
        std::string middlestring;
        std::string other; /* [SS] 2011-04-18 */
    };

    class EventHandler
    {
    protected:
        AbcParser *parser;

    public:
        EventHandler(AbcParser *p) :
            parser(p) 
        {}
        virtual ~EventHandler() {}

        virtual void init(int argc, char const *argv[], std::string *filename);
        virtual void text(char const *s) {}
        virtual void reserved(char p) {}
        virtual void x_reserved(char p) {}
        virtual void tex(char const *s) {}
        virtual void score_linebreak(char ch) {}
        virtual void linebreak(void) {}
        virtual void startmusicline(void) {}
        virtual void endmusicline(char endchar) {}
        virtual void eof(void) {}
        virtual void comment(char const *s) {}
        virtual void specific(char const *package, char const *s) {}
        virtual void startinline() {}
        virtual void closeinline() {}
        virtual void field(char k, char const *f) {}
        virtual void appendfield(char k, const char *f) {};
        virtual void words(char const *p, int continuation) {}
        virtual void part(char const *s) {}
        virtual void voice(int n, char const *s, struct voice_params *params) {}
        virtual void length(int n) {}
        virtual void default_length(int n) {}
        virtual void blankline() {}
        virtual void refno(int n) {}
        virtual void tempo(int n, int a, int b, int rel, char const *pre, char const *post) {}
        virtual void timesig(Abc::TimeSigDetails *timesig) {}
        virtual void octave(int num, int local) {}
        virtual void info_key(char const *key, char const *value) {}
        virtual void info(char const *s) {}
        virtual void key(int sharps, char const *s, int modeindex, 
                    char modmap[7], int modmul[7], fraction modmicro[7],
                    int gotkey, int gotclef, char const *clefname, AbcMusic::ClefType *new_clef,
                    int octave, int transpose, int gotoctave, int gottranspose,
                    int explict) {}
        virtual void microtone(int dir, int a, int b) {}
        virtual void graceon() {}
        virtual void graceoff() {}
        virtual void rep1() {}
        virtual void rep2() {}
        virtual void playonrep(char const *s) {}
        virtual void tie() {}
        virtual void slur(int t) {}
        virtual void sluron(int t) {}
        virtual void sluroff(int t) {}
        virtual void rest(int decorators[], int n,int m,int type) {}
        virtual void mrest(int n,int m, char c) {}
        virtual void spacing(int n, int m) {}
        virtual void bar(int type, char const *replist) {}
        virtual void space(void) {}
        virtual void lineend(char ch, int n) {}
        virtual void broken(int type, int mult) {}
        virtual void tuple(int n, int q, int r) {}
        virtual void chord() {}
        virtual void chordon(int chorddecorators[]) {}
        virtual void chordoff(int, int) {}
        virtual void instruction(char const *s) {}
        virtual void gchord(char const *s) {}
        virtual void note(int decorators[Abc::DECSIZE], 
                    AbcMusic::ClefType *clef,
                    char accidental, int mult, 
                    char note, int xoctave, int n, int m) {}
        virtual void abbreviation(char symbol, char const *string, char container) {}
        virtual void acciaccatura() {}
        virtual void start_extended_overlay() {}
        virtual void stop_extended_overlay() {}
        virtual void split_voice() {}
        virtual void temperament(char const *line) {}
        virtual void log(char const *msg) { fprintf(stderr, "info: %s\n", msg); }
        virtual void warning(char const *msg) { fprintf(stderr, "warning: %s\n", msg); }
        virtual void error(char const *msg) { fprintf(stderr, "error: %s\n", msg); }
    }; // end EventHandler

public:
    // following public vars accessible by EventHandler cliento

    char const *abcversion;
    char lastfieldcmd;
    std::vector<int> modeminor;
    std::vector<int> modekeyshift;
    std::vector<int> decorations; // STACCATO, TENUTO, ...
    int repcheck; /* AbcStore may enable/disable repeat checking */
    int oldchordconvention;
    int lineposition;
    int lineno;

    /* microtonal support and scale temperament */
    int microtone;
    fraction setmicrotone;	
    int temperament; // value is client specific

    int beatmodel;
    int inchordflag; // set by chordon event

    int decorators_passback[Abc::DECSIZE];
    /* this public array is accessed by AbcStore.c and 
     *   yaps.tree.c and to communicate decorator information
     *   from event_instruction to parsenote.
     */

    AbcParser();
    ~AbcParser();

    enum ParseMode
    {
        k_AbcToMidi,
        k_AbcToAbc,
    };

    int Parse(char const *buf, EventHandler *, ParseMode m);
    int Parse(std::ifstream *stream, EventHandler *, ParseMode m);

public:
    int readnumf(char const *num);
    int readnump(char const **num);
    void parserOff();
    void skipspace(char const **p) /* skip space and tab */
    {
        char c = **p;
        while((c == ' ') || (c == '\t'))
        {
            *p = *p + 1;
            c = **p;
        }
    }
    /* copy across alpha string -- */
    void readstr(char *out, char const **in, int limit)
    {
        int i = 0;
        while((isalpha(**in)) && (i < limit - 1))
        {
            out[i] = **in;
            i = i + 1;
            *in = *in + 1;
        }
        out[i] = '\0';
    }

    /* Used by parse_mididef() in AbcStore */
    /* Just like readstr but also allows anything except white space */
    int readaln(char *out, char const **in, int limit)
    {
        int i = 0;
        while((!isspace(**in)) && (**in) != 0 && (i < limit - 1))
        {
            out[i] = **in;
            i = i + 1;
            *in = *in + 1;
        };
        out[i] = '\0';
        return i;
    }

    int readsnump(char const **p);
    int readsnumf(char const *p);

private:
    int parse(std::istream *, EventHandler *h, ParseMode m);
    void parseBegin(ParseMode m);
    void parseEnd();
    void parserOn();
    void reset_parser_status(); // on each tune

    int parseStream(std::istream *); // re-entrant for include
    void parseline(char const *line);
    void skiptospace(char const **p)
    {
        char c = **p;
        while((c != ' ') && (c != '\t') && c != '\0')
        {
            *p = *p + 1;
            c = **p;
        }
    }
    void lcase(char *s);
    void readlen(int *a, int *b, char const **p);
    const char *readword(char word[30], char const *s);
    void readsig(char const **sig, Abc::TimeSigDetails *d);
    void read_L_unitlen(int *num, int *denom, char const **place);
    int read_complex_has_timesig(char const **place, Abc::TimeSigDetails *timesig);
    void record_abbreviation(char symbol, char const *str);
    char const *lookup_abbreviation(char symbol);
    void clear_abbreviations();
    void process_microtones(int *parsed, char word[30],
        char modmap[], int modmul[], fraction modmicrotone[7]);
    int check_power_of_two(int denom);


    std::ifstream *parse_abc_include(char const *s);
    void parse_precomment(char const *s);
    void preparse_words(char const *s);
    void parsefield(char c, char const *s);
    char *parseinlinefield(char *s); // may overrwrite input
    void parsemusic(char const *s);
    void parsenote(char const **s);
    void parsevoice(char const *s);
    void parsetempo(char const *s);
    int parsename (char const **s, char word[], int *gotname,
        std::string *namestring);
    int parsesname (char const **s, char word[], int *gotname, 
        std::string *snamestring);
    int parsemiddle(char const **s, char word[], int *gotmiddle,
        std::string *middlestring);
    int parseother(char const **s, char word[], int *gotother,
        std::string *otherstring);
    int parsetranspose (char const **s, char word[30], 
        int *gottranspose, int *transpose);
    int parseoctave(char const **s, char word[30], int *gotoctave, int *octave);
    int parsekey(char *s);
    void check_and_call_bar(int bar_type, char const *replist);
    void check_bar_repeats(int bar_type, char const *replist);
    char *getrep(char *p, char *out); /* look for number or list following [ | or :| */
    int checkend(char const *s);
    void resolve_unitlen(); // for each tune
    void set_voice_from_master(int voice_num);
    int interpret_voice_label(char const *s, int num, int *is_new);

    int parseclef(char const **, char word[30], int *got, 
        char *clefstr, AbcMusic::ClefType *c, 
        int *gotoctave, int *octave);
    int isclef(char const *s, AbcMusic::ClefType *new_clef,
        int *gotoctave, int *octave, int expect_clef);

private:
    struct voice_context 
    {
        voice_context() // for arrays
        {
            this->Init();
        }

        void Init()
        {
            this->label[0] = '\0';
            this->expect_repeat = 0;
            this->repeat_count = 0;
            this->clef.InitStandard("treble");
            this->timesig.Init();
        }

        char label[31];
        int expect_repeat;
        int repeat_count;
        Abc::TimeSigDetails timesig;
        AbcMusic::ClefType clef;
        int unitlen; /* unitlen value currently active in voice */
    };

private:
    EventHandler *handler;
    ParseMode parseMode;
    std::vector<std::istream *> istreamStack;

    int parsing_started;
    int parsing, slur;
    int ignore_line;
    int inhead, inbody;
    int parserinchord;
    int ingrace;
    int chorddecorators[Abc::DECSIZE];
    char const *decorations;
    std::vector<std::string> abbreviation; // SIZE_ABBREVIATIONS
    
    int num_voices;  /* [JA] 2020-10-12 */
    std::vector<voice_context> voicecode;
    Abc::TimeSigDetails master_timesig;  /* [JA] 2020-12-10 */
    AbcMusic::ClefType master_clef;
    int has_timesig;
    int master_unitlen; /* L: field value is 1/unitlen */
    int voicenum; /* current voice number */
    int has_voice_fields;
    
    std::string inputline;
    char const*linestart;
    char timesigstring[16];		/* [SS] 2011-08-19 links with stresspat.c */
    
    int nokey;
    int nokeysig;               /* links with toabc.c [SS] 2016-03-03 */
    int chord_n, chord_m;		/* for event_chordoff */
    int fileline_number;
    int intune;
};

#endif