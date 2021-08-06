/*
 * overflow file for AbcStore containing override methods
 * for AbcParser::EventHandler methods.
 */

#include "AbcStore.h"

void
AbcStore::init(int argc, char const*argv[], std::string *filename)
{
    int j;
    int arg, m, n;

    /* look for code checking option */
    if (this->getarg("-c", argc, argv) != -1) 
        this->check = 1;
    else
        this->check = 0;

    /* disable repeat checking because abc2midi does its own workaround
     * attempting to fix various repeat errors.
     */
    this->parser->repcheck = 0;

    /* look for filename-from-tune-titles option */
    this->namelimit = 252;
    this->titlenames = 0;
    if(this->getarg("-t", argc, argv) != -1)
    {
        this->titlenames = 1;
        this->namelimit = 8;
    }
    /* look for verbose option */
    arg = this->getarg("-v", argc, argv);
    if(arg != -1) 
    {
        if (argc > arg)
        {
            n = sscanf(argv[arg],"%d", &m);
            if (n > 0) 
                this->verbose = m; 
            else 
                this->verbose = 1; /* arg != -1 but arg == argc */
        } 
        else 
            this->verbose = 0;
    }
    if(this->getarg("-ver", argc, argv) != -1) 
    {
        printf("%s\n", this->version);
        return;
    }
    /* look for "no forte no piano" option */
    if (this->getarg("-NFNP", argc, argv) != -1)
        this->nofnop = 1;
    else
        this->nofnop = 0;

    if(this->getarg("-NFER",argc, argv) != -1)
        this->ignore_fermata = 1;
    else
        this->ignore_fermata = 0;

    if(this->getarg("-NGRA",argc, argv) != -1)
        this->ignore_gracenotes = 1;
    else
        this->ignore_gracenotes = 0;

    if(this->getarg("-NGUI",argc, argv) != -1)
        this->ignore_guitarchords = 1;
    else
        this->ignore_guitarchords = 0;
  
    if(this->getarg("-NCOM", argc, argv) != -1) 
        this->nocom = 1;
    else
        this->nocom = 0;

    if(this->getarg("-STFW",argc,argv) != -1)
        this->separate_tracks_for_words = 1;
    else
        this->separate_tracks_for_words = 0;

    if(this->getarg("-HARP",argc,argv) != -1)
        this->harpmode = 1;
    else 
        this->harpmode = 0;

    if(this->getarg("-EA",argc,argv) != -1)
        this->easyabcmode = 1;
    else
        this->easyabcmode = 0;

    arg = getarg("-BF",argc,argv);
    if(arg != -1)  
    { 
        if (argc > arg) 
        {
            n = sscanf(argv[arg],"%d",&m);
            if (n > 0) 
                this->genMidi.stressmodel = m;
        } 
        else 
            this->genMidi.stressmodel = 2;
    } 
    else 
        this->genMidi.stressmodel = 0;
    this->genMidi.barflymode = this->genMidi.stressmodel;

    arg = getarg("-TT",argc,argv); /* [SS] 2012-04-01 */
    if (arg != -1) 
    {
        float afreq, semitone_shift;
        n = 0;
        if (argc > arg) 
        {
            n = sscanf(argv[arg],"%f", &afreq);
        }
        if (n < 1) 
        {
            this->error("expecting float between 415.30 and 466.16 after -TT\n");
        } 
        else 
        {
            char msg[100];
            this->retuning = 1;
            semitone_shift = (float) (12.0 * log10(afreq/440.0f)/log10(2.0f));
            snprintf(msg, 100, "afreq = %f semitone_shift = %f\n", 
                    afreq, semitone_shift);
            this->log(msg);
            if(semitone_shift >= 1.001) 
            {
                snprintf(msg, 100, "frequency %f must be less than 466.16\n",
                    afreq);
                this->warning(msg);
                this->retuning = 0;
            }
            else
            if(semitone_shift <= -1.015) 
            {
                printf("frequency %f must be greater than 415.0\n", afreq);
                this->retuning = 0;
            }             
            if(this->retuning) 
            {
                this->bend = (int) (8192.0 * semitone_shift) + 8192;
                if (bend > 16383) bend=16383;
                if (bend < 0) bend = 0;
                printf("bend = %d\n",bend);
            }
        }
    } 
    if(this->getarg("-OCC",argc,argv) != -1) 
        this->parser->oldchordconvention = 1;
    if(this->getarg("-silent",argc,argv) != -1) 
        this->silent = 1;

    /* allocate space for notes, we'll grow it as needed */
    this->maxnotes = 500;
    this->notelist.resize(this->maxnotes);
    /* and for text */
    this->atext.resize(this->maxtexts);
    this->wordlist.clear();

    for (j=0;j<Abc::DECSIZE;j++)  
        this->dummydecorator[j] = 0;

    if((this->getarg("-h", argc, argv) != -1) || (argc < 2)) 
    {
        char msg[100];
        snprintf(msg, 100, "abc2midi_cpp version %s", this->version);
        this->log(msg);
        this->log(
        "Usage : abc2midi <abc file> [reference number] [-c] [-v] "
        "[-o filename]\n"
        "        [-t] [-n <value>] [-CS] [-NFNP] [-NCOM] [-NFER] [-NGRA] [-NGUI] [-HARP]\n"
        "        [reference number] selects a tune\n"
        "        -c  selects checking only\n"
        "        -v  selects verbose option\n"
        "        -ver prints version number and exits\n"
        "        -o <filename>  selects output filename\n"
        "        -t selects filenames derived from tune titles\n"
        "        -n <limit> set limit for length of filename stem\n"
        "        -CS use 2:1 instead of 3:1 for broken rhythms\n"
        "        -quiet suppress some common warnings\n"
        "        -silent suppresses most messages\n"
        "        -Q default tempo (quarter notes/minute)\n"
        "        -NFNP don't process !p! or !f!-like fields\n"
        "        -NCOM suppress comments in output MIDI file\n"
        "        -NFER ignore all fermata markings\n"
        "        -NGRA ignore grace notes\n"
        "        -NGUI ignore guitar chord indications\n"
        "        -STFW separate tracks for words (lyrics)\n"
        "        -HARP ornaments=roll for harpist (same pitch)\n"
        "        -BF Barfly mode: invokes a stress model if possible\n"
        "        -OCC old chord convention (eg. +CE+)\n"
        "        -TT tune to A =  <frequency>\n"
        "        -CSM <filename> load custom stress models from file\n"
        " The default action is to write a MIDI file for each abc tune\n"
        " with the filename <stem>N.mid, where <stem> is the filestem\n"
        " of the abc file and N is the tune reference number. If the -o\n"
        " option is used, only one file is written. This is the tune\n"
        " specified by the reference number or, if no reference number\n"
        " is given, the first tune in the file.\n"
        );
        return;
    } 
    else 
    {
        xmatch = 0;
        if ((argc >= 3) && (isdigit(*argv[2]))) 
        {
            xmatch = this->parser->readnumf(argv[2]);
        };
        *filename = argv[1];
        this->outbase = argv[1];
        std::string::size_type s = this->outbase.find_last_of('.');
        if(s != std::string::npos)
            this->outbase.assign(this->outbase.substr(0, s-1));
    }
  
    /* look for filename stem limit */
    j = this->getarg("-n", argc, argv);
    if (j != -1) 
    {
        if (argc >= j+1) 
        {
            namelimit = 0;
            sscanf(argv[j], "%d", &namelimit);
            if ((namelimit < 3) || (namelimit > 252)) 
            {
                this->error("filename stem limit must be in the range 3 - 252");
                return;
            }
        }
    } 
    else 
        this->warning("No number given, ignoring -n option");

    /* look for default tempo */
    j = getarg("-Q", argc, argv);
    if (j != -1) 
    {
        if (argc >= j+1) 
        {
            sscanf(argv[j], "%d", &default_tempo);
            if(default_tempo < 3) 
            {
                this->error("Q parameter is too small\nEnter -Q 240 not -Q 1/4=240");
                return;
            }
        } 
        else 
            this->warning("No number given, ignoring -Q option");
    }
       
    /* look for user-supplied output filename */
    j = getarg("-o", argc, argv);
    if (j != -1)
    {
        if (argc >= j+1)
        {
            this->outname = argv[j];
            this->userfilename = 1;
            if(this->xmatch == 0) 
            {
                this->xmatch = -1;
            }
            if (this->titlenames == 1) 
            {
                this->warning("-o option overrides -t option");
                this->titlenames = 0;
            }
        } 
        else 
        {
            this->warning("No filename given, ignoring -o option");
        }
    }

    j = this->getarg("-CSM", argc, argv);
    if (j != -1)
    {
        if (argc >= j+1)
        {
            this->csmfilename = argv[j];
            if(this->csmfilename[0] == '-') 
            {
                this->warning("csmfilename confused with options");
                this->csmfilename.clear();
            }
        } 
        else 
            this->warning("Filename required after -CSM option");
    }

    this->ratio_standard = this->getarg("-CS", argc, argv); /* [SS] 2016-01-02 */
    this->quiet  = this->getarg("-quiet", argc, argv);
    this->dotune = 0;
    this->parser->parserOff();
    this->setup_chordnames();

    /* [SS] 2016-01-02 */
    if(this->getarg("-RS",argc,argv) != -1) 
        this->warning("use -CS to get Celtic broken rhythm");
  
    /* XXX: 
    if(this->barflymode) 
        this->init_stresspat();
    */
}

void
AbcStore::text(char const *s)
{
    if(this->quiet == -1)
    {
        char msg[200];
        snprintf(msg, sizeof(msg), "Ignoring text: %s", s);
        this->warning(msg); /* [JM] 2018-02-22 */
    } 
}

void
AbcStore::x_reserved(char c)
{
    if(this->quiet == -1)
    {
        char msg[200];
        snprintf(msg, sizeof(msg), "Ignoring reserved character %c", c);
        this->warning(msg); /* [JM] 2018-02-22 */
    }
}

// abbreviation ignoredo
// acciaccatura ignored

/* When a split voice is encountered for the first time, we
   must create a new voice and insert the proper delay (rests)
   so that it remains in sync with the starting voice. If
   the split voice already exists, we still sync it to the
   source voice (there may have been intervening bars with
   no splits) by adding rests. 
*/
void 
AbcStore::split_voice()
{
    int program = 0;
    int i;
    if (!this->voicesused) 
        this->insertfeature(Abc::VOICE, 1, 0, 0, this->v1index+1); 
    this->voicesused = 1; /* multivoice file */
    int splitno = v->tosplitno;

    /* a voice split in bar is just like a bar line */
    this->zerobar();

    /* in case we need to change it */
    this->v->lastbarloc = this->notes;  
    int voiceno = v->voiceno;
    int indexno = v->indexno;
    int topvoiceno = v->topvoiceno;
    int topindexno = v->topindexno;
    int octaveshift = v->octaveshift;
    int default_length = v->default_length; 
    int midichannel = v->midichannel;
    int sync_to;
    if (topvoiceno == voiceno) 
        sync_to = this->search_backwards_for_last_bar_line(this->notes-1);
    this->addfeature(Abc::SINGLE_BAR,0,0,0);

    if (splitno == -1) 
    {
        splitno = 32+numsplits++;
        v->tosplitno = splitno;
    }
    /* save basemap and basemul in case it was just changed. We
     * need to send it to the split voice. [SS] 2013-10-30
     */
    int abasemap[7], abasemul[7]; /* active basemap */
    for (i=0;i<7;i++) 
    {
        abasemap[i] = v->basemap[i];
        abasemul[i] = v->basemul[i];
    }

    this->v = this->getvoicecontext(splitno);
    /* propagate the active basemap to the split voice */
    for (i=0;i<7;i++) 
    {
        v->basemap[i] = abasemap[i];
        v->basemul[i] = abasemul[i];
    }
    this->copymap(v);

    /* need to propogate unit length */
    v->default_length = default_length;

    splitdepth++;
    this->addfeature(Abc::VOICE, v->indexno, 0, 0);
    if (v->fromsplitno == -1) 
    {
        v->fromsplitno = voiceno;
        v->topvoiceno = topvoiceno;
        v->topindexno = topindexno;
        v->octaveshift = octaveshift;
        v->default_length = default_length;
    }
    this->dependent_voice[v->indexno] = 1;
    /* when syncing the split voice we want to be sure that
     * we do not include the notes in the last bar in the source
     * voice the notes in the split voice take their place.
     */
    v->midichannel = midichannel; 
    if(this->extended_overlay_running) 
        this->sync_voice(v, extended_overlay_running, 1);
    else
        this->sync_voice (v, sync_to, 1);
}

// event_tex inherit
// event_error inherit
// event_warning inherit
// event_info inherit

// event_linebreak
void
AbcStore::linebreak()
{
    this->addfeature(Abc::LINENUM, this->parser->lineno, 0, 0);
}

// event_score_linebreak inherit

/* starting to parse line of abc music */
void 
AbcStore::startmusicline()
{
    this->addfeature(Abc::MUSICLINE, 0, 0, 0);
}

/* finished parsing line of abc music */
void 
AbcStore::endmusicline(char endchar)
{
    this->addfeature(Abc::MUSICSTOP, 0, 0, 0);
}

/* comment found in abc */
void 
AbcStore::comment(char const *s)
{
    if(this->nocom) 
        return;
    if(this->dotune) 
    {
        // XXX: what's the diff?
        if(this->pastheader)
            this->textfeature(Abc::TEXT, s);
        else
            this->textfeature(Abc::TEXT, s);
    }
}

void 
AbcStore::start_extended_overlay()
{
    this->extended_overlay_running = this->notes;
}

void 
AbcStore::stop_extended_overlay()
{
    this->extended_overlay_running = 0;
    if (this->v->fromsplitno != -1 || this->splitdepth >0) 
        this->recurse_back_to_original_voice();
}

/* package-specific command found i.e. %%NAME */
void 
AbcStore::specific(char const *package, char const *s)
{
    if(this->started_parsing == 0) 
    {
        this->specific_in_header(package, s);
        return;
	}

    if(strcmp(package, "MIDIdef") == 0)
    {
        this->parse_mididef(s);
        return;
    }

    if(strcmp(package,"MIDIx") == 0) 
    {
        this->process_midix(s);
        return;
    }

    if (strcmp(package, "MIDI") == 0) 
        this->midi(s); 
    else 
    {
        /* Parse %%abc directive */
        char msg[200], command[40];
        char const *p;
        int done = 0;
        if(strcmp(package, "abc") == 0)
        {
            p = s;
            this->parser->skipspace(&p);
            /* skip '-' character after abc */          
            p++;
            this->parser->readstr(command, &p, 40);
            /* Parse %%abc-copyright */
            if(strcmp(command, "copyright") == 0)
            {            
                int n;
                char *ptr;
                done = 1;            
                this->parser->skipspace(&p);
                /* Copy string to parse.
                * Convert \n, \t, \r, \x and \\ in string to C style character 
                * constant defaults. (Handles carriage return, linefeed and 
                * tab characters and hex codes.)
                */
                int lnth = 0;
                char *ptr = &msg[0];
                while((*p != '\0') && (lnth < 199))
                { 
                    if (*p == '\\')
                    {
                        p++;
                        switch (*p)
                        {
                        case 'n':
                            *ptr = '\n';
                            break;
                        case 'r':  
                            *ptr = '\r';
                            break;
                        case 't':  
                            *ptr = '\t';
                            break;
                        case '\\':
                            *ptr = *p;
                            break; 
                        case 'x':
                        case 'X':
                            p++;
                            sscanf (p, "%x", &n);
                            *ptr = n;
                            while ((*p != '\0') && (isxdigit (*p)))                     
                                p++;
                            p--;   
                            break;
                        default:
                            *ptr = *p;
                            break;
                        }
                    }
                    else
                        *ptr = *p;
                    ptr++;
                    p++;
                    lnth++;
                }
                *ptr = '\0';
                this->textfeature(Abc::COPYRIGHT, msg);
                if (*p != '\0') 
                    this->warning("ABC copyright notice abridged");
            }
            else
            {
                if(snprintf(msg, sizeof(msg), "%%%s%s", package, s) > sizeof(msg)) 
                    this->warning("event_specific: comment too long");
                this->comment(msg);
            }
        }
        else
        if(strcmp(package, "propagate") == 0) 
        {
            p = s;
            p = p + 13;
            /*printf("propagate-accidentals encountered\n");*/
            if(strcmp(p,"not") == 0) 
            {
                this->propagate_accidentals = 0;
            } 
            else 
            if(strcmp(p,"pitch") == 0) 
            {
                this->propagate_accidentals = 2;
            } 
            else 
            if (strcmp(p,"octave") == 0)
            {
                this->propagate_accidentals = 1;
            } 
            else 
            {
                snprintf(msg, sizeof(msg), 
                    "cannot interpret %%%%propagate-accidentals %s\n"
                    "The choices are not, octave or pitch", p);
                this->error(msg);
            }
            done = 1;
        }
        if (done == 0)  
        {
            if(snprintf(msg, sizeof(msg), "%%%s%s", package, s) > sizeof(msg)) 
            {
                this->warning("event_specific: message too long");
            }
            this->comment(msg);
        }
    }
}

/* package-specific command found i.e. %%NAME */
/* only %%MIDI commands are actually handled */
void 
AbcStore::specific_in_header(char const *package, char const *s)
{
    if(strcmp(package, "MIDI") == 0) 
    {
        char command[40];
        char const *p = s;
        int done = 0;
        p = s;

        this->parser->skipspace(&p);
        this->parser->readstr(command, &p, 40);
        if (strcmp(command, "C") == 0)
        {
            this->parser->skipspace(&p);
            int val = this->parser->readnump(&p);
            this->default_middle_c = val;
            done = 1;
        }
        else
        if(strcmp(command, "nobarlines") == 0) 
        {
            this->default_retain_accidentals = 0;
            done = 1;
        }
        else
        if(strcmp(command, "barlines") == 0) 
        {
            this->default_retain_accidentals = 2;
            done = 1;
        }
        else
        if(strcmp(command, "fermatafixed") == 0) 
        {
            this->default_fermata_fixed = 1;
            done = 1;
        }
        else
        if(strcmp(command, "fermataproportional") == 0)
        {
            this->default_fermata_fixed = 0;
            done = 1;
        }
        else
        if(strcmp(command, "harpmode") == 0)
        { 
            this->parser->skipspace(&p);
            this->harpmode = this->parser->readnump(&p);
            done = 1;
        }
        if(strcmp(command, "ratio") == 0)
        {
            this->parser->skipspace(&p);
            int b = this->parser->readnump(&p);
            this->parser->skipspace(&p);
            int a = this->parser->readnump(&p);
            if ((a > 0) && (b > 0))
            {
                this->default_ratio_a = a;
                this->default_ratio_b = b;
                if((this->default_ratio_a + this->default_ratio_b) % 2 == 1) 
                {
                    this->default_ratio_a = 2 * a;
                    this->default_ratio_b = 2 * b;
                }
            } 
            else 
                this->error("Invalid ratio");
            done = 1;
        }
        else
        if(strcmp(command, "chordname") == 0)
        {
            char name[20];
            int notes[6];
            this->parser->skipspace(&p);
            int i = 0;
            while ((i<19) && (*p != ' ') && (*p != '\0')) 
            {
                name[i] = *p;
                p = p + 1;
                i = i + 1;
            }
            name[i] = '\0';
            if (*p != ' ') 
            {
                this->error("Bad format for chordname command");
            } 
            else 
            {
                i = 0;
                while ((i<=6) && (*p == ' ')) 
                {
                    this->parser->skipspace(&p);
                    notes[i] = this->parser->readsnump(&p); 
                    i = i + 1;
                }
                this->addchordname(name, i, notes);
            }
            done = 1;
        }
        else
        if(strcmp(command, "deltaloudness") == 0)
        {
            this->parser->skipspace(&p);
            this->velocitychange = this->parser->readnump(&p);
            done = 1;
        }
        else
        if(strcmp(command,"drummap") == 0) 
        {
            this->parser->skipspace(&p);
            this->parse_drummap(&p);
            done = 1;
        }

        if (done == 0) 
        {
            this->warning("cannot handle this MIDI directive in file header");
        }
    }
}

void
AbcStore::field(char k, char const *f)
{
    if(this->dotune) 
    {
        switch (k) 
        {
        case 'T':
            this->textfeature(Abc::TITLE, f);
            if(this->titlenames && (!this->got_titlename)) 
                this->extract_filename(f);
            break;
        case 'C':
            this->textfeature(Abc::COMPOSER, f);
            break;
        case 'R':
            {
                char const* p; 
                p = f;
                snprintf(this->rhythmdesignator, sizeof(this->rhythmdesignator),
                        "%s", f); 
                this->parser->skipspace(&p);
                if (((strncmp(p, "Hornpipe", 8) == 0) ||
                    (strncmp(p, "hornpipe", 8) == 0)) &&
                    this->genMidi.barflymode ==0) 
                {
                    this->hornpipe = 1;
                    this->ratio_a = 2;
                    this->ratio_b = 4;
                }
            }
            break;
        default:
            {
                char buff[258];
                if(strlen(f) < 256) 
                {
                    sprintf(buff, "%c:%s", k, f);
                    this->textfeature(Abc::TEXT, buff);
                }
            }
            break;
        }
    } 
    else 
    {
        if (k == 'T')
            this->warning("T: outside tune body - possible missing X:");
    }
}

/* handles a w: field in the abc 
 * cpp version has "no limit" on the number of words.
 */
void 
AbcStore::words(char const *p, int continuation)
{
    this->karaoke = 1;
    if(this->v == nullptr) 
    {
        this->warning("missplaced w: field. w: field ignored");
        return;
    }
    this->v->haswords = 1;
    this->wordvoice = v->indexno;
    this->wordlist.push_back(std::string(p));
    this->addfeature(Abc::WORDLINE, this->wcount, 0, 0);
    if (continuation == 0)
        this->addfeature(Abc::WORDSTOP, 0, 0, 0);
    this->wcount++;
}

// parts have different meanings:
//  in header: define a pattern of parts to play
//  in body: assign a partid to a sequence of notes
void
AbcStore::part(char const *s)
{
    if(this->dotune) 
    {
        char const *p = s;
        this->parser->skipspace(&p);
        if(this->pastheader && this->genMidi.parts <= 0) 
        {
            // no value in toiling over part names when no
            // partspec provided in header.
            return; 
        }
        if(!this->pastheader && this->headerpartlabel == 0) 
        {
            this->genMidi.partspec.clear();
            this->genMidi.parts = this->flattenPartSpec(p, &this->genMidi.partspec);
            if(this->genMidi.parts == 1) 
            {
                /* abiguous case - might be a label not a specificaton 
                 */
                this->headerpartlabel = 1;
            }
        }
        else
        {
            if (((int)*p < 'A') || ((int)*p > 'Z')) 
            {
                if (!this->silent)
                    this->warning("Part must be one of A-Z");
                return;
            }

            int index = (int)*p - (int)'A';
            if((this->headerpartlabel == 1) && 
               (this->genMidi.partspec[0] == *p)) 
            {
                /* P: field in header is a spec, not a field */
                this->headerpartlabel = 0;
                /* comment-out speculative part label, introduced below */
                int featindex = this->genMidi.part_start[index];
                this->notelist[featindex].feature = Abc::NONOTE;
            } 
            else 
            {
                if(this->genMidi.part_start[index] != -1) 
                    this->error("Part defined more than once");
            }
            this->genMidi.part_start[index] = this->notes;
            this->addfeature(Abc::PART, (int)*p, 0, 0);
            this->checkbreak();
            this->v = this->getvoicecontext(1);
        } 
    }
}
