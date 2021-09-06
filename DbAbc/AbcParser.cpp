#include "AbcParser.h"

#include <cstring> 
#include <sstream>      // std::istringstream
#include <fstream>

#include "AbcPort.h"

AbcParser::AbcParser() :
    abcversion("2.0"),
    lastfieldcmd(' '),
    decorations(".MLRH~Tuv'OPS"), // matches Abc::
    handler(nullptr)
{
    this->modekeyshift = { 0, 5, 5, 5, 6, 0, 1, 2, 3, 4 };
    this->modeminor = { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0 };
    this->inputline.reserve(512);
    this->clear_abbreviations();
}

AbcParser::~AbcParser()
{}

int 
AbcParser::Parse(char const *buf, IAbcParseClient *h, ParseMode m)
{
    std::istringstream strstr;
    strstr.str(buf);
    return this->parse(&strstr, h, m);
}

int 
AbcParser::Parse(std::istream *s, IAbcParseClient *h, ParseMode m)
{
    return this->parse(s, h, m);
}

int
AbcParser::parse(std::istream *istream, IAbcParseClient *h, ParseMode m)
{
    this->handler = h;
    this->parseBegin(m);
    int err = this->parseStream(istream); // re-entrant for includes
    this->parseEnd();
    return err;
}

void
AbcParser::parseBegin(ParseMode m)
{
    this->parseMode = m;
    this->num_voices = 0;
    this->repcheck = (m == k_AbcToMidi) ? 0 : 1; 
    this->has_voice_fields = 0;
    this->nokey = 0; /* K: none was encountered */
    this->nokeysig = 0;
    this->fileline_number = 1;
    this->intune = 1;
    this->temperament = 0;
    this->oldchordconvention = 0;
    this->parserOn();
}

void
AbcParser::parseEnd()
{
    this->parserOff();
    // this->handler = nullptr;
}

void
AbcParser::parserOff()
{
    this->parsing = 1;
    this->slur = 0;
}

void
AbcParser::parserOn()
{
    this->parsing = 1;
    this->slur = 0;
    this->parsing_started = 1;
}

/* called at the start of each tune --- */
void 
AbcParser::reset_parser_status()
{
    AbcMusic::ClefType default_clef;
    master_timesig.Init();
    master_clef.InitStandard("treble");
    has_timesig = 0;
    master_unitlen = -1;
    voicenum = 1;
    has_voice_fields = 0;
    num_voices = 1;
    parserinchord = 0;
    ingrace = 0;
    slur = 0;
    for(int i=0;i<Abc::DECSIZE;i++)
        this->decorators_passback[i] = 0;
    this->voicecode.resize(1);
    this->voicecode[0].Init();
}

// the user's requested stream may have include requests withing
// this implementation currently doesn't support more than one level of 
// nesting.
int
AbcParser::parseStream(std::istream *istream)
{
    int err = 0;
    int reading;
    int streamline;
    std::streampos last_position = 0; /* [SDG] 2020-06-03 */
    std::string line;
    int lastch, done_eol;
    std::istream *currentStream = istream;
    std::ifstream *includeStream = nullptr;

    this->inhead = 0;
    this->inbody = 0;
    this->parserOff();
    reading = 1;
    streamline = 1;
    done_eol = 0;
    lastch = '\0';
    while (reading)
    {
        char t;
        if(currentStream->get(t)) // get returns true on success, false on EOF
        {
            /* recognize  \n  or  \r  or  \r\n  or  \n\r  as end of line */
            /* should work for DOS, unix and Mac files */
            if ((t != '\n') && (t != '\r'))
            {
                line.push_back(t);
                done_eol = 0;
            }
            else
            {
                if((done_eol) && (((t == '\n') && (lastch == '\r')) ||
				   ((t == '\r') && (lastch == '\n'))))
                {
                    done_eol = 0;
                    /* skip this character */
                }
                else
                {
                    /* reached end of line */
                    /* [SS] 2017-12-10 */
                    std::ifstream *inc = this->parse_abc_include(line.c_str());
                    if (inc == nullptr) 
                    { 
                        this->parseline(line.c_str());
                        line.clear();
                        if(includeStream == nullptr)
                        {
                            // only count lines in user's file
                            streamline = streamline + 1;
                            lineno = streamline;
                        }
                        if (parsing)
                            this->handler->linebreak();
                        done_eol = 1;
                    } 
                    else 
                    if(includeStream == nullptr) 
                    {
                        last_position = currentStream->tellg();
                        /*printf("last position = %d\n",last_position);*/
                        includeStream = inc; // closed below
                        currentStream = static_cast<std::istream*>(inc);
                        if (parsing)
                            this->handler->linebreak();
                        done_eol = 1;
                        line.clear();
                    } 
                    else 
                    {
                        this->handler->error("Not allowed to recurse include file");
                        inc->close();
                        delete inc;
                    }
                }
            }
            lastch = t;
        }
        else
        {
            // reached end-of-file, "pop-the-stack" if we're in a nested include
            reading = 0;
            if (line.length())
            {
                this->parseline(line.c_str());
                streamline = streamline + 1;
                lineno = streamline;
                if (this->parsing)
                    this->handler->linebreak();
            };
            if (currentStream == includeStream)
            {  
                /* [SS] 2017-12-10 */
                includeStream->close();
                delete includeStream;
                currentStream = istream;
                // err = fseek(fp,last_position,SEEK_SET);
                /*printf("fseek return err = %d\n",err);*/
                reading = 1;
                includeStream = nullptr;
            }
        }
    }
    this->handler->eof();
    if (parsing_started == 0)
        this->handler->error("No tune processed. Possible missing X: field");
    return err;
}

void
AbcParser::parseline(char const *line)
{
    if(strcmp(line,"%%begintext") == 0)
        ignore_line = 1;
    else
    if(strcmp(line,"%%endtext") == 0)
        ignore_line = 0;
    if(strcmp(line,"%%beginps") == 0)
        ignore_line = 1;
    else
    if(strcmp(line,"%%endps") == 0)
        ignore_line = 0;
    else
    if(parseMode == k_AbcToMidi)
    {
        if(strnicmp(line, "%%temperament", 12) == 0)
            this->handler->temperament(line);
        else
        if(strnicmp(line, "%%MidiOff",9) == 0) 
            ignore_line = 1;
        else
        if(strnicmp(line, "%%MidiOn",8) == 0)
            ignore_line = 0;
        
        if(ignore_line == 1) return;
    }

    if(ignore_line == 1)
    {
        if(parseMode == k_AbcToAbc)
            printf("%s", line); // <<----
        return;
    } 

    this->inputline = line; // <--- copy line
    // linestart: to measure intraline offset (lineposition)
    this->linestart = this->inputline.c_str(); 
    this->lineposition = 0;
    char const *p = this->linestart;
    this->ingrace = 0;
    this->skipspace(&p);
    if(strlen(p) == 0)
    {
        this->handler->blankline();
        this->inhead = 0;
        this->inbody = 0;
        return;
    }
    char c = *p;
    if(c == '\\')
    {
        if(this->parsing)
            this->handler->tex(p);
        return;
    }
    if(c == '%')
    {
        this->parse_precomment(p+1);
        if(!this->parsing)
            this->handler->linebreak();
        return;
    }
    if(strchr("ABCDEFGHIKLMNOPQRSTUVdwsWXZ+", c) != nullptr) // lettercheck
    {
        char const *q = p + 1;
        this->skipspace(&q);
        if(*q == ':')
        {
            if(*(linestart + 1) != ':')
            {
                this->handler->warning("whitespace in field declaration");
            }
            if((*(q + 1) == ':') || (*(q + 1) == '|'))
            {
                this->handler->warning("Potentially ambiguous line - either a :| repeat or a field command -- cannot distinguish.");
                if(this->inbody)
                {
                    /* malformed field command try processing it as a music line */
                    if(*p != 'w') // exception for w: field
                    {
                        if(this->parsing)
                            this->parsemusic(p);
                    }
                    else
                        this->preparse_words((char *) p); // okay since we have a copy
                }
                else
                {
                    if(this->parsing)
                        this->handler->text(p);
                }
            }
            else
                this->parsefield(*p, q+1);
        }
        else
        {
            if(this->inbody)
            {
                if(this->parsing)
                    this->parsemusic(p);
            }
            else
            {
                if(this->parsing)
                    this->handler->text(p);
            };
        }
    } // end lettercheck
    else
    {
        if(this->inbody)
        {
            if(this->parsing)
                this->parsemusic(p);
        }
        else
        {
            if(this->parsing)
                this->handler->text(p);
        }
    }
}

void
AbcParser::parse_precomment(char const *s)
{
    char package[40];
    char const *p;
    int success = sscanf(s, "%%%%abc-version %3s", abcversion);
    if(*s == '%')
    {
        p = s + 1;
        this->readstr(package, &p, 40);
        this->handler->specific(package, p); // used by Store/GenMidi
    }
    else
        this->handler->comment(s);
}

void
AbcParser::parsemusic(char const *line)
{
    char endchar = ' ';
    int iscomment = 0;
    int starcount;
    int i;
    char playonrep_list[80];
    int decorators[Abc::DECSIZE];

    for(i = 0; i < Abc::DECSIZE; i++)
        decorators[i] = 0;

    this->handler->startmusicline();

    char *comment = (char *) line;
    while((*comment != '\0') && (*comment != '%'))
        comment = comment + 1;
    if(*comment == '%')
    {
        iscomment = 1;
        *comment = '\0';  // <<------------------
        comment = comment + 1;
    };

    char const *p = line;
    this->skipspace(&p);
    while(*p != '\0')
    {
        this->lineposition = p - this->linestart;
        if(*p == '.' && *(p+1) == '(') 
        {  
            /* [SS] 2015-04-28 dotted slur */
            p++;
            this->handler->sluron(1);
            p++; // (hrm)
        }
        if(((*p >= 'a') && (*p <= 'g')) || ((*p >= 'A') && (*p <= 'G')) ||
            (strchr("_^=", *p) != NULL) || (strchr(this->decorations, *p) != NULL))
        {
            this->parsenote(&p);
        }
        else
        {
            char c = *p; /* [SS] 2017-04-19 */
            switch(c)
            {
            case '"':
                {
                    std::string gchord;
                    p = p + 1;
                    while((*p != '"') && (*p != '\0'))
                    {
                        gchord.push_back(*p);
                        p = p + 1;
                    };
                    if(*p == '\0')
                        this->handler->error("Guitar chord name not properly closed");
                    else
                        p = p + 1;
                    this->handler->gchord(gchord.c_str());
                }
                break;
            case '|':
                p = p + 1;
                switch(*p)
                {
                case ':':
                    this->check_and_call_bar(Abc::BAR_REP, "");
                    p = p + 1;
                    break;
                case '|':
                    this->check_and_call_bar(Abc::DOUBLE_BAR, "");
                    p = p + 1;
                    break;
                case ']':
                    this->check_and_call_bar(Abc::THIN_THICK, "");
                    p = p + 1;
                    break;
                default:
                    p = this->getrep(p, playonrep_list);
                    this->check_and_call_bar(Abc::SINGLE_BAR, playonrep_list);
                };
                break;
            case ':':
                p = p + 1;
                switch(*p)
                {
                case ':':
                    this->check_and_call_bar(Abc::DOUBLE_REP, "");
                    p = p + 1;
                    break;
                case '|':
                    p = p + 1;
                    p = this->getrep(p, playonrep_list);
                    this->check_and_call_bar(Abc::REP_BAR, playonrep_list);
                    if(*p == ']')
                        p = p + 1;	/* [SS] 2013-10-31 */
                    break;
                /*  [JM] 2018-02-22 dotted bar line ... this is legal */
                default:
                    /* [SS] 2018-02-08 introducing DOTTED_BAR */
                    this->check_and_call_bar(Abc::DOTTED_BAR,"");
                }
                break;
            case ' ':
            case '\t':
                this->handler->space();
                this->skipspace(&p);
                break;
            case '(':
                p = p + 1;
                {
                    int t=0, q=0, r=0;
                    t = this->readnump(&p);
                    if((t != 0) && (*p == ':'))
                    {
                        p = p + 1;
                        q = this->readnump(&p);
                        if(*p == ':')
                        {
                            p = p + 1;
                            r = this->readnump(&p);
                        };
                    };
                    if(t == 0)
                    {
                        if(*p == '&') 
                        {
                            p = p+1;
                            this->handler->start_extended_overlay(); /* [SS] 2015-03-23 */
                        }
                        else
                            this->handler->sluron(1);
                    }
                    else  /* t != 0 */
                    {
                        this->handler->tuple(t, q, r);
                    };
                }
                break;
            case ')':
                p = p + 1;
                this->handler->sluroff(0);
                break;
            case '{':
                p = p + 1;
                this->handler->graceon();
                this->ingrace = 1;
                break;
            case '}':
                p = p + 1;
                this->handler->graceoff();
                this->ingrace = 0;
                break;
            case '[':
                p = p + 1;
                switch(*p)
                {
                case '|':
                    p = p + 1;
                    this->check_and_call_bar(Abc::THICK_THIN, "");
                    if(*p == ':') 
                    {  
                        this->check_and_call_bar(Abc::BAR_REP, "");
                        p = p + 1;
                    }
                    break;
                default:
                    if(isdigit(*p))
                    {
                        p = this->getrep(p, playonrep_list);
                        this->handler->playonrep(playonrep_list);
                    }
                    else
                    {
                        if(isalpha(*p) && (*(p + 1) == ':'))
                        {
                            // NB: may edit p
                            p = this->parseinlinefield((char *) p);
                        }
                        else
                        {
                            this->lineposition = p - this->linestart;	/* [SS] 2011-07-18 */
                            /* [SS] 2012-03-30 */
                            for(i = 0; i < Abc::DECSIZE; i++)
                                this->chorddecorators[i] = decorators[i] | decorators_passback[i];
                            this->handler->chordon(this->chorddecorators);
                            this->parserinchord = 1;
                        }
                    }
                    break;
                }
                break;
            case ']':
                p = p + 1;
                /*readlen (&chord_n, &chord_m, &p); [SS] 2019-06-06 */
                if(!this->parserinchord && *p == '|') 
                { 
                    /* [SS] 2019-06-06 not THICK_THIN  bar line*/
                    p = p + 1; /* skip over | */
                    this->check_and_call_bar(Abc::THIN_THICK, "");
                }
                else 
                {
                    this->readlen(&chord_n, &chord_m, &p); /* [SS] 2019-06-06 */
                    this->handler->chordoff(chord_n, chord_m);
                    this->parserinchord = 0;
                }
                for(i = 0; i < Abc::DECSIZE; i++)
                {
                    chorddecorators[i] = 0;
                    decorators_passback[i] = 0;	/* [SS] 2012-03-30 */
                }
                break;
            case '$':
                p = p + 1;
                this->handler->score_linebreak('$');  /* [JA] 2020-10-07 */
                break;
            /*  hidden rest  */
            case 'x':
                {
                    int n, m;
                    p = p + 1;
                    this->readlen(&n, &m, &p);
                    /* in order to handle a fermata applied to a rest we must
                    * pass decorators to this->handler->rest.
                    */
                    for(i = 0; i < Abc::DECSIZE; i++)
                    {
                        decorators[i] = this->decorators_passback[i];
                        decorators_passback[i] = 0;
                    }
                    this->handler->rest(decorators, n, m, 1);
                    decorators[Abc::FERMATA] = 0;  /* [SS] 2014-11-17 */
                }
                break;
            case 'z': /*  regular rest */
                {
                    int n, m;
                    p = p + 1;
                    this->readlen(&n, &m, &p);
                    /* in order to handle a fermata applied to a rest we must
                    * pass decorators to this->handler->rest.
                    */
                    for(i = 0; i < Abc::DECSIZE; i++)
                    {
                        decorators[i] = decorators_passback[i];
                        decorators_passback[i] = 0;
                    }
                    this->handler->rest(decorators, n, m, 0);
                    decorators[Abc::FERMATA] = 0;  /* [SS] 2014-11-17 */
                }
                break;
            case 'y':		/* used by Barfly and abcm2ps to put space */
                /* I'm sure I've seen somewhere that /something/ allows a length
                * specifier with y to enlarge the space length. Allow it anyway; it's
                * harmless.
                */
                {
                    int n, m;
                    p = p + 1;
                    this->readlen(&n, &m, &p);
                    this->handler->spacing(n, m);
                }
                break;
            /* full bar rest */
            case 'Z':
            case 'X':		/* [SS] 2012-11-15 */
                {
                    int n, m;
                    p = p + 1;
                    this->readlen(&n, &m, &p);
                    if(m != 1)
                    {
                        this->handler->error("X or Z must be followed by a whole integer");
                    };
                    this->handler->mrest(n, m, c);
                    decorators[Abc::FERMATA] = 0;  /* [SS] 2014-11-17 */
                }
                break;
            case '>':
                {
                    int n = 0;
                    while(*p == '>')
                    {
                        n = n + 1;
                        p = p + 1;
                    };
                    if(n > 3)
                        this->handler->error("Too many >'s");
                    else
                        this->handler->broken(Abc::GT, n);
                }
                break;
            case '<':
                {
                    int n = 0;
                    while(*p == '<')
                    {
                        n = n + 1;
                        p = p + 1;
                    };
                    if(n > 3)
                        this->handler->error("Too many <'s");
                    else
                        this->handler->broken(Abc::LT, n);
                }
                break;
            case 's':
                p = p + 1;
                if(this->slur == 0)
                    this->slur = 1;
                else
                    this->slur = this->slur - 1;
                this->handler->slur(this->slur);
                break;
            case '-':
                p = p + 1;
                this->handler->tie();
                break;
            case '\\':
                p = p + 1;
                if(this->checkend(p))
                {
                    this->handler->lineend('\\', 1);
                    endchar = '\\';
                }
                else
                    this->handler->error("'\\' in middle of line ignored");
                break;
            case '+':
                if(this->oldchordconvention)
                {
                    this->lineposition = p - this->linestart;	/* [SS] 2011-07-18 */
                    this->handler->chord();
                    parserinchord = 1 - parserinchord;
                    if(parserinchord == 0)
                    {
                        for(i = 0; i < Abc::DECSIZE; i++)
                            chorddecorators[i] = 0;
                    }
                    p = p + 1;
                    break;
                }
                /* else fall through into the next case statement */
            case '!':
                {
                    std::string instruction;
                    char endcode = *p;
                    p = p + 1;
                    char const *s = p;
                    while((*p != endcode) && (*p != '\0'))
                    {
                        instruction.push_back(*p);
                        p = p + 1;
                    }
                    if(*p != endcode)
                    {
                        p = s;
                        if(this->checkend(s))
                        {
                            this->handler->lineend('!', 1);
                            endchar = '!';
                        }
                        else
                        {
                            this->handler->error("'!' or '+' in middle of line ignored");
                        };
                    }
                    else
                    {
                        this->handler->instruction(instruction.c_str());
                        p = p + 1;
                    }
                }
                break;
            case '*':
                p = p + 1;
                starcount = 1;
                while(*p == '*')
                {
                    p = p + 1;
                    starcount = starcount + 1;
                };
                if(this->checkend(p))
                {
                    this->handler->lineend('*', starcount);
                    endchar = '*';
                }
                else
                    this->handler->error("*'s in middle of line ignored");
                break;
            case '/':
                p = p + 1;
                if(this->ingrace)
                    this->handler->acciaccatura();
                else
                    this->handler->error("stray / not in grace sequence");
                break;
            case '&':
                p = p + 1;
                if(*p == ')') 
                {
                    p = p + 1;
                    this->handler->stop_extended_overlay(); /* [SS] 2015-03-23 */
                    break;
                }
                else
                    this->handler->split_voice();
                break;
            default:
                {
                    char msg[40];
                    if((*p >= 'A') && (*p <= 'z')) /* [SS] 2016-09-20 */
                    {
                        this->handler->reserved(*p);
                    }
                    else
                    {
                        sprintf(msg, "Unrecognized character: %c", *p);
                        this->handler->error(msg);
                    };
                };
                p = p + 1;
                break;
            } // switch on char
        } // not a note
    } // end while
    this->handler->endmusicline(endchar);
    if(iscomment)
    {
      this->parse_precomment(comment);
    }
}

/* parse abc note and advance character pointer */
void
AbcParser::parsenote(char const **s)
{
    int decorators[Abc::DECSIZE];
    int octave, n, m;

    for(int i=0; i < Abc::DECSIZE; i++)
    {
        decorators[i] = this->decorators_passback[i];
        if (!this->inchordflag)
            this->decorators_passback[i] = 0;
    }
    while(strchr(this->decorations, **s) != nullptr)
    {
        int t = strchr(this->decorations, **s) - this->decorations;
        decorators[t] = 1;
        *s = *s + 1;
    }
    /*check for decorated chord */
    if (**s == '[')
    {
        this->lineposition = *s - this->linestart;
        if(this->parseMode == k_AbcToYaps)
            this->handler->warning("decorations applied to chord");
        for(int i = 0; i < Abc::DECSIZE; i++)
            this->chorddecorators[i] = decorators[i];
        this->handler->chordon(chorddecorators);
        if(this->parseMode == k_AbcToAbc)
        {
            for(int i = 0; i < Abc::DECSIZE; i++)
                decorators[i] = 0;
        }
        this->parserinchord = 1;
        *s = *s + 1;
        this->skipspace(s);
    }
    if(this->parserinchord)
    {
        /* inherit decorators */
        if(this->parseMode != k_AbcToAbc)
        {
            for(int i = 0; i < Abc::DECSIZE; i++)
                decorators[i] = decorators[i] | this->chorddecorators[i];
        }
    }

    /* catch fermata H followed by a rest */

    if (**s == 'z')
    {
        *s = *s + 1;
        this->readlen(&n, &m, s);
        this->handler->rest(decorators, n, m, 0);
        return;
    }

    if (**s == 'x')
    {
        *s = *s + 1;
        this->readlen(&n, &m, s);
        this->handler->rest(decorators, n, m, 1);
        return;
    }

    /* read accidental */
    int mult = 1;
    this->microtone = 0;
    char accidental = ' ';
    char note = ' ';
    switch (**s)
    {
    case '_':
        accidental = **s;
        *s = *s + 1;
        if (**s == '_')
        {
            *s = *s + 1;
            mult = 2;
        }
        this->microtone = this->ismicrotone(s, -1);
        if(this->microtone)
        {
            if (mult == 2)
                mult = 1;
            else
                accidental = ' ';
        }
        break;
    case '^':
        accidental = **s;
        *s = *s + 1;
        if(**s == '^')
        {
            *s = *s + 1;
            mult = 2;
        }
        this->microtone = this->ismicrotone(s, 1);
        if(this->microtone)
        {
            if (mult == 2)
                mult = 1;
            else
                accidental = ' ';
        }
        break;
    case '=':
        accidental = **s;
        *s = *s + 1;
        /* if ((**s == '^') || (**s == '_')) {
            accidental = **s;
            }; */
        if(**s == '^')
        {
            accidental = **s;
            *s = *s + 1;
            this->microtone = this->ismicrotone(s, 1);
            if (this->microtone == 0)
                accidental = '^';
        }
        else 
        if(**s == '_')
        {
            accidental = **s;
            *s = *s + 1;
            this->microtone = this->ismicrotone(s, -1);
            if (this->microtone == 0)
                accidental = '_';
        }
        break;
    default:
        this->microtone = this->ismicrotone(s, 1);
        break;
    }
    if((**s >= 'a') && (**s <= 'g'))
    {
        note = **s;
        octave = 1;
        *s = *s + 1;
        while((**s == '\'') || (**s == ','))
        {
            if(**s == '\'')
            {
                octave = octave + 1;
                *s = *s + 1;
            }
            if(**s == ',')
            {
                char msg[80];
                snprintf(msg, 80, "Bad pitch specifier , after note %c", note);
                this->handler->error(msg);
                octave = octave - 1;
                *s = *s + 1;
            }
        }
    }
    else
    {
        octave = 0;
        if((**s >= 'A') && (**s <= 'G'))
        {
            note = **s + 'a' - 'A';
            *s = *s + 1;
            while((**s == '\'') || (**s == ','))
            {
                if(**s == ',')
                {
                    octave = octave - 1;
                    *s = *s + 1;
                }
                if (**s == '\'')
                {
                    char msg[80];
                    snprintf(msg, 80, "Bad pitch specifier ' after note %c",
                                note + 'A' - 'a');
                    this->handler->error(msg);
                    octave = octave + 1;
                    *s = *s + 1;
                }
            }
        }
    }
    if(note == ' ')
        this->handler->error("Malformed note : expecting a-g or A-G");
    else
    {
        this->readlen(&n, &m, s);
        this->handler->note(decorators, &this->voicecode[voicenum - 1].clef, 
                            accidental, mult, note, octave, n, m);
    }
}

/* top-level routine handling all lines containing a field */
void
AbcParser::parsefield(char key, char const *f)
{
    std::string sfield(f); // copy f, so we can edit it in-place
    char *field = (char *) sfield.c_str();
    char *comment;
    char *place;
    int iscomment;
    int foundkey;
    if (key == 'X')
    {
        int x;
        char *xplace = field;
        this->skipspace((char const **) &xplace);
        x = this->readnumf(xplace);
        if(inhead)
            this->handler->error("second X: field in header");
        if(inbody)
        {
            /* [JA] 2020-10-14 */
            this->handler->error("Missing blank line before new tune");
        }
        this->handler->refno(x);
        this->ignore_line = 0; /* [SS] 2017-04-12 */
        this->reset_parser_status(); /* [JA] 2020-10-12 */
        inhead = 1;
        inbody = 0;
        parserinchord = 0;
        return;
    }

    if(this->parsing == 0)
        return;

    /*if ((inbody) && (strchr ("EIKLMPQTVdswW", key) == NULL)) [SS] 2014-08-15 */
    if(this->inbody && (strchr("EIKLMPQTVdrswW+", key) == NULL)) /* [SS] 2015-05-11 */
    {
        this->handler->error("Field not allowed in tune body");
    };
    comment = field;
    iscomment = 0;
    while((*comment != '\0') && (*comment != '%'))
    {
        comment = comment + 1;
    };
    if(*comment == '%')
    {
        iscomment = 1;
        *comment = '\0';
        comment = comment + 1;
    };

    place = field;
    this->skipspace((char const **) &place);
    switch (key)
    {
    case 'K':
        if(inhead)
        {
            /* First K: is the end of the header and the start of the body.
            * make sure we set up default for unit length
            * if L: fields was missing in the header.
            */
            this->resolve_unitlen();
        }
        foundkey = this->parsekey(place); /* parsekey called before set_voice_from_master(1) */
        if(inhead) 
        {
            /* set voice parameters using values from header */
            this->set_voice_from_master(1);
        }
        if(inhead || inbody) 
        {
            if (foundkey)
            {
                inbody = 1;
                inhead = 0;
            }
            else
            {
                if (inhead)
                {
                    this->handler->error ("First K: field must specify key signature");
                }
            }
        }
        else
        {
            this->handler->error ("No X: field preceding K:");
        }
        break;
    case 'M':
        {
            Abc::TimeSigDetails timesig;
            snprintf(timesigstring, sizeof(timesigstring), "%s", place); /* [SEG] 2020-06-07 */
            this->readsig((char const **) &place, &timesig);
            if ((*place == 's') || (*place == 'l')) 
            {
                this->handler->error ("s and l in M: field not supported");
            }
            if ((timesig.num == 0) || (timesig.denom == 0)) 
                this->handler->warning ("Invalid time signature ignored");
            else 
            {
                if (inhead) 
                {
                    /* copy timesig into master_timesig */
                    master_timesig = timesig;
                } 
                if (inbody) 
                {
                    /* update timesig in current voice */
                    voice_context &current_voice = voicecode[voicenum - 1];
                    current_voice.timesig = timesig;
                }
                this->handler->timesig (&timesig);
                has_timesig = 1;
            }
        }
	    break;
    case 'L':
        {
            int num, denom;
            this->read_L_unitlen(&num, &denom, (char const **) &place);
            if (num != 1)
                this->handler->error("Default length must be 1/X");
            else
            {
                if (denom > 0)
                {
                    this->handler->length(denom);
                    if(inhead)
                    {
                        master_unitlen = denom;
                    }
                    if (inbody)
                    {
                        voice_context &current_voice = voicecode[voicenum-1];
                        current_voice.unitlen = denom;
                    }
                }
                else
                    this->handler->error("invalid denominator");
            }
        }
        break;
    case 'P':
      this->handler->part(place);
      break;
    case 'I':
      this->handler->info(place);
      break;
    case 'V':
      this->parsevoice(place);
      break;
    case 'Q':
      this->parsetempo(place);
      break;
    case 'U':
        {
            char symbol;
            char container;
            char *expansion;
            this->skipspace((char const **) &place);
            if ((*place >= 'A') && (*place <= 'z'))  /* [SS] 2016-09-20 */
            {
                symbol = *place;
                place = place + 1;
                this->skipspace((char const **) &place);
                if (*place == '=')
                {
                    place = place + 1;
                    this->skipspace((char const **) &place);
                    if(*place == '!')
                    {
                        place = place + 1;
                        container = '!';
                        expansion = place;
                        while ((!iscntrl(*place)) && (*place != '!'))
                            place = place + 1;
                        if (*place != '!')
                            this->handler->error ("No closing ! in U: field");
                        *place = '\0';
                    }
                    else
                    {
                        container = ' ';
                        expansion = place;
                        while (isalnum (*place))
                            place = place + 1;
                        *place = '\0';
                    }
                    if(strlen(expansion) > 0)
                    {
                        this->record_abbreviation(symbol, expansion);
                        this->handler->abbreviation (symbol, expansion, container);
                    }
                    else
                        this->handler->error ("Missing term in U: field");
                }
                else
                    this->handler->error ("Missing '=' U: field ignored");
            }
            else
                this->handler->warning("only 'A' - 'z' supported in the U: field");
        }
        break;
    case 'w':
        this->preparse_words(place);
        break;
    case 'd':
        /* decoration line in abcm2ps */
        this->handler->field(key, place);	/* [SS] 2010-02-23 */
        break;
    case 's':
        this->handler->field(key, place);	/* [SS] 2010-02-23 */
        break;
    case '+':
        if (lastfieldcmd == 'w') 
            this->handler->appendfield(key, place); /*[SS] 2014-08-15 */
        break; /* [SS] 2014-09-07 */
    default:
        this->handler->field(key, place);
    }
    if (iscomment)
        this->parse_precomment(comment);
    if (key == 'w') 
        lastfieldcmd = 'w'; /* [SS] 2014-08-15 */
    else 
        lastfieldcmd = ' ';  /* [SS[ 2014-08-15 */
}

/* parse field within abc line e.g. [K:G] */
char const *
AbcParser::parseinlinefield(char *p)
{
    char *q = p;
    this->handler->startinline();
    while ((*q != ']') && (*q != '\0'))
    {
        q = q + 1;
    }
    if (*q == ']')
    {
        *q = '\0';
        this->parsefield(*p, p + 2);
        q = q + 1;
    }
    else
    {
        this->handler->error ("missing closing ]");
        this->parsefield(*p, p + 2);
    }
    this->handler->closeinline();
    return q;
}

void 
AbcParser::check_and_call_bar(int bar_type, char const *replist)
{
    if(this->repcheck)
        this->check_bar_repeats(bar_type, replist);
    this->handler->bar(bar_type, replist);
}

void 
AbcParser::check_bar_repeats(int bar_type, char const *replist)
{
    voice_context &cv = this->voicecode[this->voicenum];
    switch(bar_type) 
    {
    case Abc::SINGLE_BAR:
        break;
    case Abc::DOUBLE_BAR:
        break;
    case Abc::THIN_THICK:
        break;
    case Abc::THICK_THIN:
        break;
    case Abc::BAR_REP:
        if(cv.expect_repeat) 
        {
            this->handler->warning("Expecting repeat, found |:");
        }
        cv.expect_repeat = 1;
        cv.repeat_count = cv.repeat_count + 1;
      break;
    case Abc::REP_BAR:
        if(!cv.expect_repeat) 
        {
            char error_message[80];
            if(cv.repeat_count == 0)
            {
                snprintf(error_message, 80, "Missing repeat at start ? Unexpected :|%s found", replist);
                this->handler->warning(error_message);
            }
            else
            {
                snprintf(error_message, 80, "Unexpected :|%s found", replist);
                this->handler->warning(error_message);
            }
        }
        cv.expect_repeat = 0;
        cv.repeat_count = cv.repeat_count + 1;
        break;
    case Abc::BAR1:
        if(!cv.expect_repeat) 
        {
            if(cv.repeat_count == 0)
                this->handler->warning("Missing repeat at start ? found |1");
            else
                this->handler->warning("found |1 in non-repeat section");
        }
        break;
    case Abc::REP_BAR2:
        if(cv.expect_repeat)
        {
            if(cv.repeat_count == 0)
                this->handler->warning("Missing repeat at start ? found :|2");
            else
                this->handler->warning("No repeat expected, found :|2");
        };
        cv.expect_repeat = 0;
        cv.repeat_count = cv.repeat_count + 1;
        break;
    case Abc::DOUBLE_REP:
        if(cv.expect_repeat)
        {
            if(cv.repeat_count == 0)
                this->handler->warning("Missing repeat at start ? found ::");
            else
                this->handler->warning("No repeat expected, found ::");
        };
        cv.expect_repeat = 1;
        cv.repeat_count = cv.repeat_count + 1;
        break;
    }
}

/* look for number or list following [ | or :| */
char const *
AbcParser::getrep(char const *p, char *out)
{
    char const *q = p;
    int digits=0, done=0, count=0;
    while(!done)
    {
        if(isdigit(*q))
        {
            out[count] = *q;
            count = count + 1;
            q = q + 1;
            digits = digits + 1;
            /* [SS] 2013-04-21 */
            if(count > 50)
            {
                this->handler->error("malformed repeat");
                break;
            }
        }
        else
        {
            if(((*q == '-') || (*q == ',')) && (digits > 0)
                && (isdigit(*(q + 1))))
            {
                out[count] = *q;
                count = count + 1;
                q = q + 1;
                digits = 0;
                /* [SS] 2013-04-21 */
                if(count > 50)
                {
                    this->handler->error("malformed repeat");
                    break;
                }
            }
            else
            {
                done = 1;
            };
        }
    } // while !done
    out[count] = '\0';
    return q;
}

/* read integer from string without advancing character pointer */
int 
AbcParser::readnumf(char const *num)
{
    int t = 0;
    char const *p = num;
    if(!isdigit(*p))
    {
        this->handler->error("Missing Number");
    }
    /* [JA] 2021-05-25 */
    while(((int) *p >= '0') && ((int) *p <= '9') && (t < (INT_MAX-9)/10))
    {
        t = t * 10 + (int) *p - '0';
        p = p + 1;
    };
    if(t >= (INT_MAX-9)/10) 
    {  
        /* [JA] 2021-05-25 */
        this->handler->error("Number too big");
    }
    return t;
}

/*static*/ void
AbcParser::Skipspace(char const **p)
{
    char c = **p;
    while((c == ' ') || (c == '\t'))
    {
        *p = *p + 1;
        c = **p;
    }
}

/*static*/ void
AbcParser::SplitString(char const *s, char sep, std::vector<std::string> *result)
{
    char const *p = s;
    int fieldcoming = 1;
    while(fieldcoming) 
    {
        char const *start = p;
        char const *end;
        while ((*p != '\0') && (*p != sep)) 
            p = p + 1;
        end = p;
        if(*p == sep) 
            p++;
        else 
            fieldcoming = 0;
        result->push_back(std::string(start, end-start));
    }
}

/*static*/ void
AbcParser::Readstr(char *out, char const **in, int limit)
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

void 
AbcParser::readstr(char *out, char const **in, int limit)
{
    AbcParser::Readstr(out, in, limit);
}

void
AbcParser::skipspace(char const **p)
{
    AbcParser::Skipspace(p);
}

/*static*/ int
AbcParser::Readnump(char const **p)
{
    int t = 0;
    while (((int) **p >= '0') && ((int) **p <= '9') && (t < (INT_MAX-9)/10))
    {
        t = t * 10 + (int) **p - '0';
        *p = *p + 1;
    }
    /* advance over any spurious extra digits [JA] 2021-05-25 */
    while (isdigit(**p)) 
    {
        *p = *p + 1;
    }
    return t;
}

/* read integer from string and advance character pointer */
int
AbcParser::readnump (char const **p)
{
    return Readnump(p);
}

/*static*/ int
AbcParser::Readsnump(char const **p)
{
    if(**p == '-')
    {
        *p = *p + 1;
        AbcParser::Skipspace(p);
        return -AbcParser::Readnump(p);
    }
    else
    {
        return AbcParser::Readnump(p);
    }
}

int
AbcParser::readsnump(char const **p)
{
    return AbcParser::Readsnump(p);
}

/* reads signed integer from string without advancing character pointer */
int
AbcParser::readsnumf(char const *s)
{
    char const *p = s;
    if (*p == '-')
    {
        p = p + 1;
        this->skipspace(&p);
        return -this->readnumf(p);
    }
    else
      return this->readnumf(p);
}

/* read length part of a note and advance character pointer */
void 
AbcParser::readlen(int *a, int *b, char const **p)
{
    *a = this->readnump (p);
    if (*a == 0) 
        *a = 1;
    *b = 1;
    if (**p == '/') 
    {
        *p = *p + 1;
        *b = this->readnump (p);
        if (*b == 0) 
        {
            *b = 2;
            /* [JA] 2021-05-19 prevent infinite loop */
            /* limit the number of '/'s we support */
            while ((**p == '/') && (*b < 1024)) 
            {
                *b = *b * 2;
                *p = *p + 1;
            }
            if (*b >= 1024) 
            {
                this->handler->warning ("Exceeded maximum note denominator");
            }
        }
    }
    *b = this->check_power_of_two(*b);
}

/* part of parsekey, extracts word from input line */
/* besides the space, the symbols _, ^, and = are used */
/* as separators in order to handle key signature modifiers. */
const char *
AbcParser::readword(char word[30], char const *s)
{
    char const *p = s;
    int i = 0;
    while ((*p != '\0') && (*p != ' ') && (*p != '\t') && 
        ((i == 0) || ((*p != '='))))
    {
        if (i >1 && *p == '^') break; /* allow for double sharps and flats */
        if (i >1 && *p == '_') break;
        if (i < 29)
        {
            word[i] = *p;
            i = i + 1;
        };
        p = p + 1;
    }
    word[i] = '\0';
    return p;
}

int
AbcParser::check_power_of_two(int denom)
{
    int t;
    char error_message[80];
    t = denom; 
    while (t > 1) 
    {
        if (t % 2 != 0) 
        {
            snprintf(error_message, 80, "%d b is not a power of 2", denom);
            this->handler->error(error_message);
            return -1;
        } 
        else 
        {
            t = t / 2;
        }
    }
    return denom;
}

/* returns 1 if we are at the end of the line 0 otherwise */
/* used when we encounter '\' '*' or other special line end characters */
int
AbcParser::checkend(char const *s)
{
    char const *p = s;
    int atend;
    this->skipspace(&p);
    if (*p == '\0')
        atend = 1;
    else
        atend = 0;
    return atend;
}

/* function to resolve unit note length when the
 * L: field is missing in the header [JA] 2020-12-10
 *
 * From the abc standard 2.2:
 * If there is no L: field defined, a unit note length is set by default,
 * based on the meter field M:. This default is calculated by computing
 * the meter as a decimal: if it is less than 0.75 the default unit note
 * length is a sixteenth note; if it is 0.75 or greater, it is an eighth
 * note. For example, 2/4 = 0.5, so, the default unit note length is a
 * sixteenth note, while for 4/4 = 1.0, or 6/8 = 0.75, or 3/4= 0.75,
 * it is an eighth note. For M:C (4/4), M:C| (2/2) and M:none (free meter),
 * the default unit note length is 1/8.
 */
void 
AbcParser::resolve_unitlen()
{
    if (master_unitlen == -1)
    {
        if (has_timesig == 0)
        {
            this->handler->default_length(8);
            master_unitlen = 8;
        }
        else
        {
            if (((4 * master_timesig.num)/master_timesig.denom) >= 3)
            {
                this->handler->default_length(8);
                master_unitlen = 8;
            }
            else
            {
                this->handler->default_length(16);
                master_unitlen = 16;
            }
        }
    }
}

/* parse contents of K: field */
/* this works by picking up a strings and trying to parse them */
/* returns 1 if valid key signature found, 0 otherwise */
/* This is unfortunately a complicated function because it does alot.
 * It prepares the data:
 * sf = number of sharps or flats in key signature
 * modeindex:= major 0, minor 1, ... dorian, etc.
 * modmap: eg "^= _^   " corresponds to A# B Cb D#    
 * modmul: 2 signals double sharp or flat
 * modmicrotone: eg {2/4 0/0 0/0 2/0 0/0 -3/0 0/0} for ^2/4A ^2D _3F
 * clefstr: treble, bass, treble+8, ... 
 * octave: eg. 1,2,-1 ... 
 * transpose: 1,2 for handling certain clefs 
 * and various flags like explict, gotoctave, gottranspose, gotclef, gotkey
 * which are all sent to abc2midi (via store.c), yaps (via yapstree), abc2abc
 * via (toabc.c), using the function call this->handler->key().
 *
 * The variables sf, modeindex, modmul, and modmicrotone control which notes
 * are sharpened or flatten in a musical measure.
 * The variable clefstr selects one of the clefs, (treble, bass, ...)
 * The variable octave allows you to shift everything up and down by an octave
 * The variable transpose allows you to automatically transpose every note.
 *
 * All of this information is extracted from the string str from the
 * K: command.
 */
int
AbcParser::parsekey(char *str)
{
    char *s;
    char word[30];
    int parsed = 0;  /* [SDG] 2020-06-03 */
    int gotclef, gotkey, gotoctave, gottranspose;
    int explict;			/* [SS] 2010-05-08 */
    int modnotes;			/* [SS] 2010-07-29 */
    int foundmode;
    int transpose, octave;
    char clefstr[30];
    char modestr[30];
    char msg[80];
    char *moveon;
    int sf = -1, minor = -1;
    char modmap[7];
    int modmul[7];
    AbcMusic::fraction modmicrotone[7];
    int i, j;
    int cgotoctave, coctave;
    char *key = "FCGDAEB";
    int modeindex;
    AbcMusic::ClefType newclef; // auto-inits

    clefstr[0] = (char) 0;
    modestr[0] = (char) 0;
    s = str;
    transpose = 0;
    gottranspose = 0;
    octave = 0;
    gotkey = 0;
    gotoctave = 0;
    gotclef = 0;
    cgotoctave = 0;
    coctave = 0;
    modeindex = 0;
    explict = 0;
    modnotes = 0;
    nokey = nokeysig; /* [SS] 2016-03-03 */
    for (i = 0; i < 7; i++)
    {
        modmap[i] = ' ';
        modmul[i] = 1;
        modmicrotone[i].num = 0;	/* [SS] 2014-01-06 */
        modmicrotone[i].denom = 0;
    };
    word[0] = 0; /* in case of empty string [SDG] 2020-06-04 */
    while(*s != '\0')
    {
        parsed = this->parseclef((char const **) &s, word, 
                    &gotclef, clefstr, &newclef, &cgotoctave, &coctave);
        if(parsed) 
        {  
            /* [JA] 2021-05-21 changed (gotclef) to (parsed) */
            /* make clef an attribute of current voice */
            if (inhead)
                master_clef = newclef;
            if (inbody)
                voicecode[voicenum-1] .clef = newclef;
        }
        /* parseclef also scans the s string using readword(), placing */
        /* the next token  into the char array word[].                   */
        if(!parsed)
            parsed = this->parsetranspose((char const **) &s, word,     
                        &gottranspose, &transpose);

        if(!parsed)
            parsed = this->parseoctave ((char const **) &s, word, 
                        &gotoctave, &octave);

        if((parsed == 0) && (stricmp(word, "Hp") == 0))
        {
            sf = 2;
            minor = 0;
            gotkey = 1;
            parsed = 1;
        }

        if((parsed == 0) && (stricmp(word, "none") == 0))
        {
            gotkey = 1;
            parsed = 1;
            nokey = 1;
            minor = 0;
            sf = 0;
        }

        if(stricmp(word, "exp") == 0)
        {
            explict = 1;
            parsed = 1;
        }

        /* if K: not 'none', 'Hp' or 'exp' then look for key signature
         * like Cmaj Amin Ddor ...                                   
         * The key signature is expressed by sf which indicates the
         * number of sharps (if positive) or flats (if negative)
         */
        if((parsed == 0) && ((word[0] >= 'A') && (word[0] <= 'G')))
        {
            gotkey = 1;
            parsed = 1;
            /* parse key itself */
            sf = strchr(key, word[0]) - key - 1;
            j = 1;
            /* deal with sharp/flat */
            if (word[1] == '#')
            {
                sf += 7;
                j = 2;
            }
            else
            {
                if (word[1] == 'b')
                {
                    sf -= 7;
                    j = 2;
                }
            }
            minor = 0;
            foundmode = 0;
            if(strlen(word) == j)
            {
                /* look at next word for mode */
                this->skipspace((char const **) &s);
                moveon = (char *) this->readword(modestr, s);
                this->lcase(modestr);
                for (i = 0; i < 10; i++)
                {
                    if (strncmp(modestr, AbcMusic::mode[i], 3) == 0)
                    {
                        foundmode = 1;
                        sf = sf + AbcMusic::modeshift[i];
                        minor = this->modeminor[i];
                        modeindex = i;
                    }
                }
                if(foundmode)
                {
                    s = moveon;
                }
            }
            else
            {
                strcpy(modestr, &word[j]);
                this->lcase(modestr);
                for (i = 0; i < 10; i++)
                {
                    if(strncmp(modestr, AbcMusic::mode[i], 3) == 0)
                    {
                        foundmode = 1;
                        sf = sf + AbcMusic::modeshift[i];
                        minor = this->modeminor[i];
                        modeindex = i;
                    }
                }
                if(!foundmode)
                {
                    snprintf(msg, 80, "Unknown mode '%s'", &word[j]);
                    this->handler->error(msg);
                    modeindex = 0;
                }
            }
        }

        if(gotkey)
        {
            if (sf > 7)
            {
                this->handler->warning("Unusual key representation");
                sf = sf - 12;
            }
            if (sf < -7)
            {
                this->handler->warning("Unusual key representation");
                sf = sf + 12;
            }
        }

        /* look for key signature modifiers
         * For example K:D _B
         * which will include a Bb in the D major key signature
         *
         */
        if((word[0] == '^') || (word[0] == '_') || (word[0] == '='))
        {
            modnotes = 1;
            if ((strlen (word) == 2) && (word[1] >= 'a') && (word[1] <= 'g'))
            {
                j = (int) word[1] - 'a';
                modmap[j] = word[0];
                modmul[j] = 1;
                parsed = 1;
            }
            else
            {			
                /*double sharp or double flat */
                if ((strlen (word) == 3) && (word[0] != '=')
                    && (word[0] == word[1]) && (word[2] >= 'a')
                    && (word[2] <= 'g'))
                {
                    j = (int) word[2] - 'a';
                    modmap[j] = word[0];
                    modmul[j] = 2;
                    parsed = 1;
                }
            }
        }

        /* if (explict)  for compatibility with abcm2ps 2010-05-08  2010-05-20 */
        if((word[0] == '^') || (word[0] == '_') || (word[0] == '='))
        {
            modnotes = 1;
            if((strlen(word) == 2) && (word[1] >= 'A') && (word[1] <= 'G'))
            {
                j = (int) word[1] - 'A';
                modmap[j] = word[0];
                modmul[j] = 1;
                parsed = 1;
            }
            else 
            if((strlen (word) == 3) && (word[0] != '=') && (word[0] == word[1])
                && (word[2] >= 'A') && (word[2] <= 'G'))
            {
                /*double sharp or double flat */
                j = (int) word[2] - 'A';
                modmap[j] = word[0];
                modmul[j] = 2;
                parsed = 1;
            }
        }

        /* microtone? */
        /* shortcuts such as ^/4G instead of ^1/4G not allowed here */
        /* parsed =0; [SS] 2020-09-30 */
        this->process_microtones(&parsed,  word, modmap, modmul, modmicrotone);
    } // end while
    
    if((parsed == 0) && (strlen (word) > 0))
    {
        sprintf(msg, "Ignoring string '%s' in K: field", word);
        this->handler->warning(msg);
    }
    if(cgotoctave)
    {
        gotoctave = 1;
        octave = coctave;
    }
    if(modnotes & !gotkey)
    {
        /* explicit key signature */
        sf = 0;
        /*gotkey = 1; [SS] 2010-07-29 */
        explict = 1;		/* [SS] 2010-07-29 */
    }
    this->handler->key(sf, str, modeindex, modmap, modmul, modmicrotone, gotkey,
	     gotclef, clefstr, &newclef, octave, transpose, gotoctave, gottranspose,
	     explict);
    return gotkey;
}

void 
AbcParser::set_voice_from_master(int voice_num)
{
    voice_context &v = voicecode[voice_num - 1];
    v.timesig = master_timesig;
    v.clef = master_clef;
    v.unitlen = master_unitlen;
}

void 
AbcParser::readsig(char const **sig, Abc::TimeSigDetails *timesig)
{
    if ((strncmp (*sig, "none", 4) == 0) ||
        (strncmp (*sig, "None", 4) == 0)) 
    {
        timesig->num = 4;
        timesig->denom = 4;
        timesig->type = Abc::TIMESIG_FREE_METER;
        return;
    }
    /* [SS] 2012-08-08  cut time (C| or c|) is 2/2 not 4/4 */
    if (((**sig == 'C') || (**sig == 'c')) &&
        (*(*sig + 1) == '|')) 
    {
        timesig->num = 2;
        timesig->denom = 2;
        timesig->type = Abc::TIMESIG_CUT;
        return;
    }
    if ((**sig == 'C') || (**sig == 'c'))
    {
        timesig->num = 4;
        timesig->denom = 4;
        timesig->type = Abc::TIMESIG_COMMON;
        return;
    }
    int valid_num = this->read_complex_has_timesig(sig, timesig);
    if(!valid_num) 
    {
        /* An error message will have been generated by read_complex_has_timesig */
        timesig->num = 4;
        timesig->denom = 4;
        timesig->type = Abc::TIMESIG_FREE_METER;
        return;
    }

    if ((int)**sig != '/') 
    {
        this->handler->warning("No / found, assuming denominator of 1");
        timesig->denom = 1;
    } 
    else 
    {
        *sig = *sig + 1;
        this->skipspace(sig);
        if (!isdigit(**sig)) {
            this->handler->warning("Number not found for M: denominator");
        }
        timesig->denom = this->readnump (sig);
    }
    if ((timesig->num == 0) || (timesig->denom == 0)) 
        this->handler->error("Expecting fraction in form A/B");
    else 
        timesig->denom = this->check_power_of_two(timesig->denom);
}

void
AbcParser::read_L_unitlen(int *num, int *denom, char const **place)
{
    if(!isdigit(**place))
        this->handler->warning("No digit at the start of L: field");

    *num = this->readnump(place);
    if(*num == 0)
        *num = 1;
    if((int)**place != '/') 
    {
        this->handler->error("Missing / ");
        *denom = 1;
    } 
    else 
    {
        *place = *place + 1;
        this->skipspace(place);
        *denom = this->readnump(place);
    }
    if((*num == 0) || (*denom == 0)) 
        this->handler->error("Expecting fraction in form A/B");
    else 
        *denom = this->check_power_of_two(*denom);
}

void
AbcParser::parsevoice(char const *s)
{
    int num;			/* voice number */
    IAbcParseClient::voice_params vparams;
    char word[64]; /* 2017-10-11 */
    int parsed;
    int coctave = 0, cgotoctave = 0;
    int is_new = 0;

    vparams.transpose = 0;
    vparams.gottranspose = 0;
    vparams.octave = 0;
    vparams.gotoctave = 0;
    vparams.gotclef = 0;
    vparams.gotname = 0;
    vparams.gotsname = 0;
    vparams.gotmiddle = 0;
    vparams.gotother = 0;		/* [SS] 2011-04-18 */
    vparams.other[0] = '\0';	/* [SS] 2011-04-18 */

    this->skipspace(&s);
    num = 0;
    if(isdigit(*s)) 
        num = this->readnump(&s);

    num = this->interpret_voice_label(s, num, &is_new);
    if(is_new)
    {
        /* declaring voice for the first time.
        * initialize with values set in the tune header */
        this->set_voice_from_master(num);
    }
    has_voice_fields = 1;
    this->skiptospace(&s);
    voicenum = num;
    this->skipspace(&s);
    while (*s != '\0') 
    {
        parsed = this->parseclef(&s, word, &vparams.gotclef, 
                    vparams.clefname, &vparams.new_clef, &cgotoctave, &coctave);
        if (vparams.gotclef) 
        {
            /* make clef an attribute of current voice */
            copy_clef (&voicecode[num - 1].clef, &vparams.new_clef);
        }
        if (!parsed)
            parsed = this->parsetranspose(&s, word, &vparams.gottranspose,
                        &vparams.transpose);
        if (!parsed)
            parsed = this->parseoctave(&s, word, &vparams.gotoctave, &vparams.octave);
        if (!parsed)
            parsed = this->parsename(&s, word, &vparams.gotname, &vparams.namestring);
        if (!parsed)
            parsed = this->parsesname(&s, word, &vparams.gotsname, &vparams.snamestring);
        if (!parsed)
            parsed = this->parsemiddle(&s, word, &vparams.gotmiddle, &vparams.middlestring);
        if (!parsed)
            parsed = this->parseother(&s, word, &vparams.gotother,  &vparams.other);	/* [SS] 2011-04-18 */
    }
    /* [SS] 2015-05-13 allow octave= to change the clef= octave setting */
    /* cgotoctave may be set to 1 by a clef=. vparams.gotoctave is set  */
    /* by octave= */

    if (cgotoctave && vparams.gotoctave == 0)
    {
        vparams.gotoctave = 1;
        vparams.octave = coctave;
    }
    this->handler->voice(num, s, &vparams);

    /*
    if (gottranspose) printf("transpose = %d\n", vparams.transpose);
    if (gotoctave) printf("octave= %d\n", vparams.octave);
    if (gotclef) printf("clef= %s\n", vparams.clefstr);
    if (gotname) printf("parsevoice: name= %s\n", vparams.namestring);
    if(gotmiddle) printf("parsevoice: middle= %s\n", vparams.middlestring);
    */
}

int 
AbcParser::parseclef(char const **s, char *word, 
    int *gotclef, char *clefstr,
    AbcMusic::ClefType *newclef, int *gotoctave, int *octave)
{
    int successful;
    this->skipspace(s);
    *s = this->readword(word, *s);
    successful = 0;
    if(stricmp(word, "clef") == 0)
    {
        this->skipspace(s);
        if (**s != '=')
        {
            this->handler->error ("clef must be followed by '='");
        }
        else
        {
            *s = *s + 1;
            this->skipspace (s);
            *s = this->readword(clefstr, *s);
            if(this->isclef(clefstr, newclef, gotoctave, octave, 1))
            {
                *gotclef = 1;
            }
        };
        successful = 1;
    }
    else 
    if(this->isclef(word, newclef, gotoctave, octave, 0))
    {
        *gotclef = 1;
        strcpy(clefstr, word);
        successful = 1;
    };
    return successful;
}

/* parses string transpose= number */
int
AbcParser::parsetranspose(char const **s, char word[30], 
    int *gottranspose, int *transpose)
{
    if(stricmp(word, "transpose") != 0)
        return 0;
    this->skipspace(s);
    if (**s != '=')
    {
        this->handler->error ("transpose must be followed by '='");
    }
    else
    {
        *s = *s + 1;
        this->skipspace(s);
        *transpose = this->readsnump(s);
        *gottranspose = 1;
    }
    return 1;
}

/* parses string octave= number */
int
AbcParser::parseoctave (char const **s, char word[], int *gotoctave, int *octave)
{
    if(stricmp(word, "octave") != 0)
        return 0;
    this->skipspace(s);
    if(**s != '=')
    {
        this->handler->error("octave must be followed by '='");
    }
    else
    {
        *s = *s + 1;
        this->skipspace(s);
        *octave = this->readsnump (s);
        *gotoctave = 1;
    }
    return 1;
}

void
AbcParser::print_voicecodes()
{
    if(this->num_voices == 0)
        return;
    char msg[80];
    this->handler->log("voice mapping:");
    for(int i = 0; i < this->num_voices; i++)
    {
        snprintf(msg, 80, "%d %s", 
            i+1, this->voicecode[i].label);
        this->handler->log(msg);
    }
}

/* We expect a numeric value indicating the voice number.
 * The assumption is that these will ocuur in the order in which voices
 * appear, so that we have V:1, V:2, ... V:N if there are N voices.
 * The abc standard 2.2 allows strings instead of these numbers to
 * represent voices.
 * This function should be called with either
 * an empty string and a valid num or
 * a valid string and num set to 0.
 *
 * If num is non-zero, we check that it is valid and return it.
 * If the number is one above the number of voices currently in use,
 * we allocate a new voice.
 *
 * If num is zero and the string is not empty, we check if string
 * has been assigned to one of the existing voices. If not, we
 * allocate a new voice and assign the string to it.
 *
 * If we exceed MAX_VOICES voices, we report an error.
 *
 * we return a voice number in the range 1 - num_voices
*/
int 
AbcParser::interpret_voice_label(char const *s, int num, int *is_new)
{
    int i;
    char code[30];
    char msg[80];   /* [PHDM] 2012-11-22 */
    char const *c = this->readword(code, s);
    if(num > 0)
    {
        if(num > this->num_voices + 1)
        {
            char error_message[80];
            snprintf(error_message, 80, "V:%d out of sequence, treating as V:%d",
                num, num_voices);
            this->handler->warning(error_message);
            num = num_voices + 1;
        }
        /* declaring a new voice */
        if (num == this->num_voices + 1)
        {
            *is_new = 1;
            this->num_voices++;
            voice_context vc;
            vc.label[0] = '\0';
            this->voicecode.push_back(vc); // <--------------------------
        } 
        else 
        {
            /* we are using a previously declared voice */
            *is_new = 0;
        }
        return num;
    }
    /* [PHDM] 2012-11-22 */
    if (*c != '\0' && *c != ' ' && *c != ']') 
    {
        sprintf(msg, "invalid character `%c' in Voice ID", *c);
        this->handler->error(msg);
    }
    /* [PHDM] 2012-11-22 */

    if(code[0] == '\0')
    {
        this->handler->warning("Empty V: field, treating as V:1");
        return 1;
    }

    /* Has supplied label been used for one of the existing voices ? */
    if(this->has_voice_fields)
    {
        for (i = 0; i < this->voicecode.size(); i++) 
        {
            if(strcmp(code, this->voicecode[i].label) == 0)
            {
                return i + 1;
            }
        }
    }

    /* if we have got here, we have a new voice */

    /* First V: field is a special case. We are already on voice 1,
    * so we don't increment the number of voices, but we will set
    * status->has_voice_fields on returning from this function.
    */
    if (has_voice_fields)
    {
        *is_new = 1;
        this->num_voices++;
        voice_context vc;
        vc.label[0] = '\0';
        this->voicecode.push_back(vc); // <--------------------------
    } 
    else 
    {
        *is_new = 0; /* we have already started as voice 1 */
    }
    strncpy(this->voicecode[num_voices - 1].label, code, 31);
    return num_voices;
}

/* read the numerator of a time signature in M: field
 *
 * abc standard 2.2 allows M:(a + b + c + ...)/d
 * This indicates how note lenths within a bar are to be grouped.
 * abc standard also allows a+b+c/d to mean the same thing but this 
 * goes against the convention that division takes precendence
 * over addition i.e. a+b+c/d normally means a + b + (c/d).
 */
int 
AbcParser::read_complex_has_timesig(char const **place, Abc::TimeSigDetails *timesig)
{
    int n;
    int total;
    int count;
    int has_bracket = 0;

    if (**place == '(') 
    {
        *place = *place + 1;
        has_bracket = 1;
        this->skipspace(place);
    }
    count = 0;
    total = 0;
    this->skipspace(place);
    while ((**place != '\0') && (isdigit(**place)))
    {
        n = this->readnump(place);
        timesig->complex_values[count] = n;
        total = total + n;
        count = count + 1;
        if(count > 8) 
        {
            this->handler->error("Too many parts to complex time (maximum 8)");
            return 0;
        }
        this->skipspace(place);
        if (**place == '+') 
        {
            *place = *place + 1;
            this->skipspace(place);
        }
    }
    if (**place == ')') 
    {
        *place = *place + 1; /* advance over ')' */
        this->skipspace(place);
        if (!has_bracket) 
        {
            this->handler->warning("Missing ( in complex time");
        }
        has_bracket = 0;
    }
    if (has_bracket) 
    {
        this->handler->warning("Missing ) in complex time");
    }
    /* we have reached the end of the numerator */
    timesig->num_values = count;
    timesig->num = total;
    if (timesig->num_values == 1)
        timesig->type = Abc::TIMESIG_NORMAL;
    else 
        timesig->type = Abc::TIMESIG_COMPLEX;
    return 1;
}

/* convert word to lower case */
void
AbcParser::lcase(char *s)
{
    char *p= s;
    while (*p != '\0')
    {
        if (isupper(*p))
            *p = *p - 'A' + 'a';
        p = p + 1;
    }
}

void 
AbcParser::process_microtones(int *parsed,  char word[30],
    char modmap[], int modmul[], AbcMusic::fraction modmicrotone[])
{
    int a, b;/* for microtones [SS] 2014-01-06 */
    char c;
    int j;

    /* shortcuts such as ^/4G instead of ^1/4G not allowed here */
    int success = sscanf(&word[1], "%d/%d%c", &a, &b, &c);
    if (success == 3) /* [SS] 2016-04-10 */
    {
        *parsed = 1;
        j = (int) c - 'A';
        if (j > 7)
          j = (int) c - 'a';
        if (j > 7 || j < 0) 
        {
            this->handler->error("Not a valid microtone");
            return;
        }
        if (word[0] == '_') a = -a;
        /* printf("%s fraction microtone  %d/%d for %c\n",word,a,b,c); */
    } 
    else 
    {
	    success = sscanf(&word[1], "%d%c", &a, &c); /* [SS] 2020-06-25 */
        if (success == 2)
	    {
            b = 0;
            /* printf("%s integer microtone %d%c\n",word,a,c); */
            if (temperament != 1) 
            { /* [SS] 2020-06-25 2020-07-05 */
		        this->handler->warning("do not use integer microtone without calling %%MIDI temperamentequal");
            }
            *parsed = 1;
	    }
    }
    /* if (parsed ==1)  [SS] 2020-09-30 */
    if(success > 0) 
    {
        j = (int) c - 'A';
        if (j > 7)
            j = (int) c - 'a';
        if (j > 7 || j < 0) 
        {
            this->handler->error("Not a valid microtone");
            return;
        }
        if (word[0] == '_') a = -a;
        modmap[j] = word[0];
	    modmicrotone[j].num = a;
	    modmicrotone[j].denom = b;
	    /* printf("%c microtone = %d/%d\n",modmap[j],modmicrotone[j].num,modmicrotone[j].denom); */
    }
} /* finished ^ = _ */

void
AbcParser::clear_abbreviations()
{
    this->abbreviation.clear();
    this->abbreviation.resize(58);
}

/* update record of abbreviations when a U: field is encountered */
void
AbcParser::record_abbreviation(char symbol, char const *str)
{
    /* if ((symbol < 'H') || (symbol > 'Z')) [SS] 2016-09-20 */
    if ((symbol < 'A') || (symbol > 'z'))
        return;

    int index = symbol - 'A';
    if(index < this->abbreviation.size())
        this->abbreviation[index] = str;
    else
        this->handler->error("Invalid abreviation symbol");
}

/* return string which symbol abbreviates */
char const *
AbcParser::lookup_abbreviation(char symbol)
{
    /* if ((symbol < 'H') || (symbol > 'Z'))  [SS] 2016-09-25 */
    if((symbol < 'A') || (symbol > 'z'))
        return nullptr;
    else
    {
        char const *s = this->abbreviation[symbol - 'A'].c_str();
        return *s == '\0' ? nullptr : s;
    }
}

int 
AbcParser::parsename(char const **s, char word[], int *gotname,
    std::string *namestring)
{
    int i = 0;
    if(stricmp(word, "name") != 0)
        return 0;

    namestring->clear();
    this->skipspace(s);
    if (**s != '=')
        this->handler->error("name must be followed by '='");
    else
    {
        *s = *s + 1;
        this->skipspace(s);
        if (**s == '"')		/* string enclosed in double quotes */
        {
            namestring->push_back((char) **s);
            *s = *s + 1;
            i++;
            while (i < 256 && **s != '"' && **s != '\0')
            {
                namestring->push_back((char) **s);
                *s = *s + 1;
                i++;
            }
            namestring->push_back((char) **s);	/* copy double quotes */
            i++;
        }
        else			/* string not enclosed in double quotes */
        {
            while (i < 256 && **s != ' ' && **s != '\0')
            {
                namestring->push_back((char) **s);
                *s = *s + 1;
                i++;
            }
        }
        *gotname = 1;
    }
    return 1;
}

/* parses string name= "string" in V: command 
 * for compatability of abc2abc with abcm2ps
 */
int
AbcParser::parsesname(char const **s, char word[], int *gotname, 
    std::string *namestring)
{
    int i = 0;
    if (stricmp(word, "sname") != 0)
        return 0;
    this->skipspace(s);
    if (**s != '=')
    {
        this->handler->error("name must be followed by '='");
    }
    else
    {
        namestring->clear();
        *s = *s + 1;
        this->skipspace(s);
        /* string enclosed in double quotes */
        if (**s == '"')		
        {
            namestring->push_back((char) **s);
            *s = *s + 1;
            i++;
            while (i < 256 && **s != '"' && **s != '\0')
            {
                namestring->push_back((char) **s);
                *s = *s + 1;
                i++;
            }
            namestring->push_back((char) **s);	/* copy double quotes */
        }
        else			/* string not enclosed in double quotes */
        {
            while(i < 256 && **s != ' ' && **s != '\0')
            {
                namestring->push_back((char) **s);
                *s = *s + 1;
                i++;
            }
        }
        *gotname = 1;
    }
    return 1;
}

/* parse string middle=X in V: command
 * for abcm2ps compatibility
 */
int
AbcParser::parsemiddle(char const **s, char word[], int *gotmiddle,
     std::string *middlestring)
{
    int i = 0;
    if(stricmp(word, "middle") != 0)
        return 0;
    this->skipspace(s);
    if (**s != '=')
    {
        this->handler->error("middle must be followed by '='");
    }
    else
    {
        *s = *s + 1;
        this->skipspace(s);
        middlestring->clear();
        /* we really ought to check the we have a proper note name; for now, 
         * just copy non-space characters */
        while (i < 256 && **s != ' ' && **s != '\0')
        {
            middlestring->push_back((char) **s);
            *s = *s + 1;
            ++i;
        }
        *gotmiddle = 1;
    }
    return 1;
}

/* parses any left overs in V: command (eg. stafflines=1) */
int
AbcParser::parseother(char const **s, char word[], int *gotother,
     std::string *other)
{
    if (word[0] != '\0')
    {
        other->append(word);
        if(**s == '=')
        {			/* [SS] 2011-04-19 */
            *s = this->readword(word, *s);
            other->append(word);
        }
        other->append(" ");/* in case other codes follow */
        *gotother = 1;
        return 1;
    }
    return 0;
}

/* part of K: parsing - looks for a clef in K: field                 */
/* format is K:string where string is treble, bass, baritone, tenor, */
/* alto, mezzo, soprano or K:clef=arbitrary                          */
/* revised by James Allwright  [JA] 2020-10-18 */
int 
AbcParser::isclef(char const *s, AbcMusic::ClefType *new_clef,
    int *gotoctave, int *octave, int expect_clef)
{
    new_clef->octave_offset = 0;
    int gotclef = new_clef->InitStandard(s);
    if (!gotclef && expect_clef) 
    {
        /* do we have a clef in letter format ? e.g. C1, F3, G3 */
        gotclef = new_clef->InitExtended(s);
        if (new_clef->basic_clef == AbcMusic::basic_clef_none) 
        {
            this->handler->warning("Found clef=none, but a clef is required. Ignoring");
            gotclef = 0;
        }
    }
    if (expect_clef && !gotclef) 
    {
        char error_message[80];
        snprintf (error_message, 80, "clef %s not recognized", s);
        this->handler->warning(error_message);
    } 
    return gotclef;
}

int
AbcParser::ismicrotone(char const **p, int dir)
{
    int a, b;
    char const *chp = *p;

    this->read_microtone_value(&a, &b, p);
    /* readlen_nocheck advances past microtone indication if present */
    if (chp != *p) /* [HL] 2020-06-20 */
    {
        /* printf("event_microtone a = %d b = %d\n",a,b); */
        this->handler->microtone (dir, a, b);
        return 1;
    }
    this->setmicrotone.num = 0;
    this->setmicrotone.denom = 0;
    return 0;
}

/* read length part of a note and advance character pointer */
void
AbcParser::read_microtone_value (int *a, int *b, char const **p)
{
    *a = this->readnump(p);
    if(*a == 0)
        *a = 1;
    *b = 1;
    if (**p == '/')
    {
        *p = *p + 1;
        *b = this->readnump(p);
        if (*b == 0)
        {
            *b = 2;
            while(**p == '/')
            {
                *b = *b * 2;
                *p = *p + 1;
            }
        }
    } 
    else 
    {
        /* To signal that the microtone value is not a fraction */
        *b = 0; 
    }
    int t = *b;
    while(t > 1)
    {
        if(t % 2 != 0)
        {
            /*event_warning("divisor not a power of 2"); */
            t = 1;
        }
        else
            t = t / 2;
    }
}

std::ifstream *
AbcParser::parse_abc_include(char const *s)
{
    char includefilename[256];
    std::ifstream *istr = nullptr;
    int success = sscanf(s, "%%%%abc-include %79s", 
                        includefilename);
    if (success == 1) 
    {
        /* printf("opening include file %s\n",includefilename); */
        istr = new std::ifstream(includefilename, 
                    std::ifstream::in);
        if(istr == nullptr)
        {
            char msg[300];
            snprintf(msg, 300, 
                "Failed to open include file %s", 
                includefilename);
            this->handler->error(msg);
        }
    }
    return istr; 
}

/* takes a line of lyrics (w: field) and strips off */
/* any continuation character */
void
AbcParser::preparse_words(char *s)
{
    /* printf("Parsing %s\n", s); */
    /* strip off any trailing spaces */
    int l = strlen(s) - 1;
    while((l >= 0) && (*(s + l) == ' '))
    {
        *(s + l) = '\0';
        l = l - 1;
    }
    int continuation;
    if(*(s + l) != '\\')
        continuation = 0;
    else
    {
        /* [SS] 2014-08-14 */
        this->handler->warning("\\n continuation no longer supported in w: line");
        continuation = 1;
        /* remove continuation character */
        *(s + l) = '\0'; // <----------------
        l = l - 1;
        while ((l >= 0) && (*(s + l) == ' '))
        {
            *(s + l) = '\0'; // <----------------
            l = l - 1;
        }
    }
    this->handler->words(s, continuation);
}

/* parse tempo descriptor i.e. Q: field */
void
AbcParser::parsetempo(char *place)
{

    int relative = 0;
    char *p = place;
    char *pre_string = nullptr;
    if (*p == '"')
    {
        p = p + 1;
        pre_string = p;
        while((*p != '"') && (*p != '\0'))
            p = p + 1;
        if(*p == '\0')
            this->handler->error("Missing closing double quote");
        else
        {
            *p = '\0'; // <-------------
            p = p + 1;
            place = p;
        }
    }
    while((*p != '\0') && (*p != '='))
        p = p + 1;
    
    int a, b;
    if(*p == '=')
    {
        p = place;
        this->skipspace((char const **) &p);
        if(((*p >= 'A') && (*p <= 'G')) || ((*p >= 'a') && (*p <= 'g')))
        {
            relative = 1;
            p = p + 1;
        }
        this->readlen(&a, &b, (char const **) &p);
        this->skipspace((char const **) &p);
        if(*p != '=')
            this->handler->error("Expecting = in tempo");
        p = p + 1;
    }
    else
    {
        a = 1;
        /*a = 0;  [SS] 2013-01-27 */
        b = 4;
        p = place;
    }
    this->skipspace((char const **) &p);
    int n = this->readnump((char const **) &p);
    char *post_string = nullptr;
    if (*p == '"')
    {
        p = p + 1;
        post_string = p;
        while((*p != '"') && (*p != '\0'))
            p = p + 1;

        if(*p == '\0')
            this->handler->error("Missing closing double quote");
        else
        {
            *p = '\0'; // <-----
            p = p + 1;
        }
    }
    this->handler->tempo(n, a, b, relative, pre_string, post_string);
}