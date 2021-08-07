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
    if(this->getarg("-c", argc, argv) != -1) 
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
        if(argc > arg)
        {
            n = sscanf(argv[arg],"%d", &m);
            if(n > 0) 
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
    if(this->getarg("-NFNP", argc, argv) != -1)
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
        if(argc > arg) 
        {
            n = sscanf(argv[arg],"%d",&m);
            if(n > 0) 
                this->genMidi.stressmodel = m;
        } 
        else 
            this->genMidi.stressmodel = 2;
    } 
    else 
        this->genMidi.stressmodel = 0;
    this->genMidi.barflymode = this->genMidi.stressmodel;

    arg = getarg("-TT",argc,argv); /* [SS] 2012-04-01 */
    if(arg != -1) 
    {
        float afreq, semitone_shift;
        n = 0;
        if(argc > arg) 
        {
            n = sscanf(argv[arg],"%f", &afreq);
        }
        if(n < 1) 
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
                if(bend > 16383) bend=16383;
                if(bend < 0) bend = 0;
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

    for(j=0;j<Abc::DECSIZE;j++)  
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
        if((argc >= 3) && (isdigit(*argv[2]))) 
        {
            xmatch = this->parser->readnumf(argv[2]);
        }
        *filename = argv[1];
        this->outbase = argv[1];
        std::string::size_type s = this->outbase.find_last_of('.');
        if(s != std::string::npos)
            this->outbase.assign(this->outbase.substr(0, s-1));
    }
  
    /* look for filename stem limit */
    j = this->getarg("-n", argc, argv);
    if(j != -1) 
    {
        if(argc >= j+1) 
        {
            namelimit = 0;
            sscanf(argv[j], "%d", &namelimit);
            if((namelimit < 3) || (namelimit > 252)) 
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
    if(j != -1) 
    {
        if(argc >= j+1) 
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
    if(j != -1)
    {
        if(argc >= j+1)
        {
            this->outname = argv[j];
            this->userfilename = 1;
            if(this->xmatch == 0) 
            {
                this->xmatch = -1;
            }
            if(this->titlenames == 1) 
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
    if(j != -1)
    {
        if(argc >= j+1)
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
    if(!this->voicesused) 
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
    if(topvoiceno == voiceno) 
        sync_to = this->search_backwards_for_last_bar_line(this->notes-1);
    this->addfeature(Abc::SINGLE_BAR,0,0,0);

    if(splitno == -1) 
    {
        splitno = 32+numsplits++;
        v->tosplitno = splitno;
    }
    /* save basemap and basemul in case it was just changed. We
     * need to send it to the split voice. [SS] 2013-10-30
     */
    int abasemap[7], abasemul[7]; /* active basemap */
    for(i=0;i<7;i++) 
    {
        abasemap[i] = v->basemap[i];
        abasemul[i] = v->basemul[i];
    }

    this->v = this->getvoicecontext(splitno);
    /* propagate the active basemap to the split voice */
    for(i=0;i<7;i++) 
    {
        v->basemap[i] = abasemap[i];
        v->basemul[i] = abasemul[i];
    }
    this->copymap(v);

    /* need to propogate unit length */
    v->default_length = default_length;

    splitdepth++;
    this->addfeature(Abc::VOICE, v->indexno, 0, 0);
    if(v->fromsplitno == -1) 
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
    if(this->v->fromsplitno != -1 || this->splitdepth >0) 
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

    if(strcmp(package, "MIDI") == 0) 
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
                    if(*p == '\\')
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
                if(*p != '\0') 
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
            if(strcmp(p,"octave") == 0)
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
        if(done == 0)  
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
        if(strcmp(command, "C") == 0)
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
            if((a > 0) && (b > 0))
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
            if(*p != ' ') 
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

        if(done == 0) 
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
                if(((strncmp(p, "Hornpipe", 8) == 0) ||
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
        if(k == 'T')
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
    if(continuation == 0)
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
            if(((int)*p < 'A') || ((int)*p > 'Z')) 
            {
                if(!this->silent)
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

/* handles a V: field in the abc */
void 
AbcStore::voice(int n, char const *s, AbcParser::voice_params *vp)
{
    if(!this->voicesused && this->bodystarted) 
    {
        this->warning("First V: field occurs past body; will combine this body with this voice.");
            this->bodystarted = 0;
    }
    if(this->pastheader || true)  // XTEN1: enables reading V: in header
    {
        this->voicesused = 1;
        if(this->pastheader)
            this->checkbreak();
        this->v = this->getvoicecontext(n); 
        this->addfeature(Abc::VOICE, v->indexno, 0, 0); 
        this->dependent_voice[this->v->indexno] = 0;
        if(vp->gotclef)
            this->octave(vp->new_clef.octave_offset, 1);
        if(vp->gotoctave) 
            this->octave(vp->octave, 1);
        if(vp->gottranspose)
            this->addfeature(Abc::TRANSPOSE, vp->transpose, 0, 0);
    }
    else 
    {
        this->warning("V: in header ignored");
    }
}

/* handles an L: field in the abc */
void
AbcStore::length(int n)
{
    if(this->pastheader)
        this->v->default_length = n;
    else 
        this->global.default_length = n;
}

/* handles a Q: field e.g. Q: a/b = n  or  Q: Ca/b = n */
/* strings before and after are ignored */
void
AbcStore::tempo(int n, int a, int b, int rel, 
                char const *pre, char const *post)
{
    if(n == 0) 
        n = this->get_tempo_from_name(pre);
    if((n == 0) || ((a!=0) && (b == 0)))
    {
        this->error("malformed Q: field ignored");
    } 
    else 
    {
        if(this->dotune) 
        {
            if(this->pastheader)  // relative change in tempo
            {
                this->tempo_num = a;
                this->tempo_denom = b;
                this->relative_tempo = rel;

                int t_num, t_denom;
                this->tempounits(&t_num, &t_denom);

                int new_tempo = (long) 60*1000000*t_denom/(n*4*t_num);
                /* split up into short ints */
                int tempo_l = new_tempo & 0xffff;
                int tempo_h = new_tempo >> 16;

                float frac = new_tempo/(float)this->current_tempo;
                int new_div = (int) (.5f + AbcGenMidi::DIV*frac);
                this->addfeature(Abc::TEMPO, new_div, tempo_h, tempo_l);
            } 
            else 
            {
                this->Qtempo = n;
                this->tempo_num = a;
                this->tempo_denom = b;
                this->relative_tempo = rel;
            }
        }
    }
}

/* handles an M: field  M:n/m */
void 
AbcStore::timesig(Abc::TimeSigDetails *timesig)
{
    int dochecking;
    if(timesig->type == Abc::TIMESIG_FREE_METER) 
        dochecking = 0;
    else
        dochecking = 1;

    if(this->dotune) 
    {
        if(this->pastheader) 
        {
            this->addfeature(Abc::TIME, dochecking, timesig->num, timesig->denom);
            mtime_num = timesig->num;
            mtime_denom = timesig->denom;
            if(this->v != NULL) 
            {
                v->active_meter_num = timesig->num;
                v->active_meter_denom = timesig->denom;
            }
        } 
        else 
        {
            this->time_num = timesig->num;
            this->time_denom = timesig->denom;
            this->mtime_num = timesig->num;
            this->mtime_denom = timesig->denom;
            this->meter_voice_update(timesig->num, timesig->denom);
            this->timesigset = 1;
            this->barchecking = dochecking;
        }
    }
}

/* used internally by other routines when octave=N is encountered */
/* in I: or K: fields */
void 
AbcStore::octave(int num, int local)
{
    if(this->dotune) 
    {
        if(pastheader || local) 
            this->v->octaveshift = num;
        else 
            this->global.octaveshift = num;
    }
}

void 
AbcStore::info_key(char const* key, char const* value)
{
    if(strcmp(key, "octave")==0) 
    {
        int num = this->parser->readsnumf(value);
        this->octave(num, 0);
    }
    if(strcmp(key, "MIDI") == 0 || strcmp(key, "MIDIx") == 0 )
        this->specific(key, value);
    else 
    if(strcmp(key, "volinc")  == 0 || strcmp(key,"vol") == 0)
    {
        char midicmd[64];
        midicmd[0] = 0;
        strcat(midicmd, key);
        strcat(midicmd, " ");
        strcat(midicmd, value);
        this->specific("MIDI", midicmd);
    }
    /*
    else 
    if(is_abcm2ps_option (key)) 
        return;
    */
    else 
    {
        char errmsg[80];
        snprintf(errmsg, 80, "I: key \' %s\' not recognized", key);
        if(quiet == -1 && silent == 0) 
            this->error(errmsg); /* [SS] 2018-04-01 */
    }
}

/* a { in the abc */
void 
AbcStore::graceon()
{
    if(this->gracenotes) 
    {
        this->error("Nested grace notes not allowed");
    } 
    else 
    {
        if(v->inchord) 
        {
            this->error("Grace notes not allowed in chord");
        } 
        else 
        {
            this->gracenotes = 1;
            this->addfeature(Abc::GRACEON, 0, 0, 0);
            this->v->ingrace = 1;
            this->stack_broken(this->v);
        }
    }
}

/* a } in the abc */
void 
AbcStore::graceoff()
{
    if(!this->gracenotes) 
    {
        this->error("} without matching {");
    } 
    else 
    {
        this->gracenotes = 0;
        this->addfeature(Abc::GRACEOFF, 0, 0, 0);
        this->v->ingrace = 0;
        this->restore_broken(v);
    }
}

/* [X in the abc, where X is a list of numbers */
void
AbcStore::playonrep(char const *s)
{
    int num;
    char seps[2];
    int converted = sscanf(s, "%d%1[,-]", &num, seps);
    if(converted == 0) 
        this->error("corrupted variant ending");
    else 
    {
        if((converted == 1) && (num != 0)) 
            this->addfeature(Abc::PLAY_ON_REP, 0, 0, num);
        else
            this->textfeature(Abc::PLAY_ON_REP, s);
    }
}

/* called when ( is encountered in the abc */
void 
AbcStore::sluron(int t)
{
    if(v->inslur) 
        this->warning("Slur within slur");
    else 
    {
        this->addfeature(Abc::SLUR_ON, 0, 0, 0);
        v->inslur = 1;
    }
}

void 
AbcStore::sluroff(int t)
{
    if(v->inslur) 
    {
        this->addfeature(Abc::SLUR_OFF, 0, 0, 0);
        v->inslur = 0;
    }
}

/* a tie - has been encountered in the abc */
void 
AbcStore::tie()
{
    if(this->gracenotes && this->ignore_gracenotes) 
        return;
    if(this->notelist[this->notes-1].feature == Abc::CHORDOFF ||
       this->notelist[notes-1].feature == Abc::CHORDOFFEX) 
    { 
        /* did a TIE connect with a chord */
        this->patchup_chordtie(chordstart,notes-1);
    }
    else
        this->addfeature(Abc::TIE, 0, 0, 0);
}

/* handles > >> >>> < << <<< in the abc (shorthand for swing) */
void 
AbcStore::broken(int type, int mult)
{
    if(this->v->inchord) 
        this->error("Broken rhythm not allowed in chord");
    else 
    if(this->v->ingrace) 
        this->error("Broken rhythm not allowed in grace notes");
    else 
    {
        if((this->hornpipe) && (this->notelist[notes-1].feature == Abc::GT)) 
        {
            /* remove any superfluous hornpiping */
            this->notes = this->notes - 1;
        }
        /* addfeature(type, mult, 0, 0); */
        this->v->brokentype = type;
        this->v->brokenmult = mult;
        this->v->brokenpending = 0;
    }
}

/* handles triplets (3 and general tuplets (n:q:r in the abc */
void
AbcStore::tuple(int n, int q, int r)
{
    if(this->tuplecount > 0) 
    {
        this->error("nested tuples");
        return;
    }
    if(r == 0) 
    {
        this->specialtuple = 0;
        this->tuplecount = n;
    } 
    else 
    {
        this->specialtuple = 1;
        this->tuplecount = r;
    }
    if(q != 0) 
    {
        this->tfact_num = q;
        this->tfact_denom = n;
    } 
    else 
    {
        if((n < 2) || (n > 9)) 
        {
            this->error("Only tuples (2 - (9 allowed");
            this->tfact_num = 1;
            this->tfact_denom = 1;
            this->tuplecount = 0;
        } 
        else 
        {
            /* deduce tfact_num using standard abc rules */
            if((n == 2) || (n == 4) || (n == 8)) 
                this->tfact_num = 3;
            if((n == 3) || (n == 6)) 
                this->tfact_num = 2;
            if((n == 5) || (n == 7) || (n == 9)) 
            {
                if((time_num % 3) == 0) 
                    this->tfact_num = 3;
                else 
                    this->tfact_num = 2;
            }
            this->tfact_denom = n;
        }
    }
    this->tnote_num = 0;
    this->tnote_denom = 0;
}

/* a + or [ or ]  has been encountered in the abc */
void 
AbcStore::chord()
{
    if(this->v->inchord) 
        this->chordoff(1,1);
    else 
        this->chordon(this->dummydecorator);
}

/* rest of n/m in the abc */
void 
AbcStore::rest(int decorators[Abc::DECSIZE],
                int n, int m, int type)
{
    int num = n; 
    int denom = m;

    if(decorators[Abc::FERMATA] && !this->ignore_fermata) 
    {
        if(fermata_fixed) 
            this->addfract(&num, &denom, 1, 1);
        else 
            num = num*2;
    }
    if(this->v == nullptr) 
    {
        this->error("Internal error : no voice allocated");
        return;
    }
    if(this->v->inchord) 
        this->v->chordcount++;

    if(this->tuplecount > 0) 
    {
        num = num * this->tfact_num;
        denom = denom * this->tfact_denom;
        if(this->tnote_num == 0) 
        {
            this->tnote_num = num;
            this->tnote_denom = denom;
        } 
        else 
        {
            if(this->tnote_num * denom != num * this->tnote_denom) 
            {
                if(!this->specialtuple && this->quiet == -1) 
                    this->warning("Different length notes in tuple");
            }
        }
        if((!this->gracenotes) && (!this->v->inchord)) 
        {
            this->tuplecount--;
        }
    }
    if(this->v->chordcount == 1) 
    {
        this->v->chord_num = num*4;
        this->v->chord_denom = denom*(this->v->default_length);
    }
    if((!this->v->ingrace) && ((!this->v->inchord)||(this->v->chordcount==1))) 
    {
        this->genMidi.addunits(num, denom*(v->default_length));
    }
    this->last_num = 3; /* hornpiping (>) cannot follow rest */
    this->addfeature(Abc::REST, 0, num*4, denom*(v->default_length));
    if(!this->v->inchord)
        this->marknote();
}

/* multiple bar rest of n/m in the abc */
/* we check for m == 1 in the parser */
void 
AbcStore::mrest(int n, int m, char c)
{
    int decorators[Abc::DECSIZE];
    decorators[Abc::FERMATA] = 0;
    /* it is not legal to pass a fermata to a multirest */

    for(int i=0; i<n; i++) 
    {
        this->rest(decorators, 
                    this->genMidi.mtime_num*(v->default_length), 
                    this->genMidi.mtime_denom, 0);
        if(i != n-1) 
            this->bar(Abc::SINGLE_BAR, "");
    }
}

/* handles a chord start [ in the abc */
/* the array chorddecorators is needed in toabc.c and yapstree.c */
/* and is used here to handle fermatas.                          */
void 
AbcStore::chordon(int chorddecorators[])
{
    this->parser->inchordflag = 1;
    this->apply_fermata_to_chord = chorddecorators[Abc::FERMATA];
    if(v->inchord) 
        this->error("Attempt to nest chords");
    else 
    {
        this->chordstart = this->notes;
        if(this->easyabcmode) 
        {
            this->addfeature(Abc::META,0, this->parser->lineno,
                this->parser->lineposition);
        }
        this->addfeature(Abc::CHORDON, 0, 0, 0);
        v->inchord = 1;
        v->chordcount = 0;
        v->chord_num = 0;
        v->chord_denom = 1;
        this->marknotestart();
    }
}

/* handles a chord close ] in the abc */
void 
AbcStore::chordoff(int chord_n, int chord_m)
{
    int c_n, c_m;
    this->parser->inchordflag = 0;
    featureDesc &fd = this->notelist[this->chordstart];
    if(chord_m == 1 && chord_n == 1) 
    {
        c_n = fd.num;
        c_m = fd.denom;
    }
    else 
    {
        c_n = chord_n;
        c_m = chord_m; 
    }

    if(!v->inchord) 
        this->error("Chord already finished");
    else 
    {
        if(this->tuplecount > 0) 
        {
            c_n = c_n * tfact_num;
            c_m = c_m * tfact_denom;
            if(tnote_num == 0) 
            {
                tnote_num   = c_n;
                tnote_denom = c_m;
            } 
            else 
            {
                /*   if(tnote_num * c_m != c_n * tnote_denom) {
                        if(!specialtuple) {
                            event_warning("Different length notes in tuple for chord");
                        }
                    }
                    [SS] 2013-04-24 
                */
            }
            if((!this->gracenotes) && (!this->v->inchord)) 
                this->tuplecount--;
        }

        if(chord_m == 1 && chord_n == 1) /* chord length not set outside [] */
            this->addfeature(Abc::CHORDOFF, 0, v->chord_num, v->chord_denom); 
        else
        {
            this->addfeature(Abc::CHORDOFFEX, 0, c_n*4, c_m*v->default_length);
            this->fix_enclosed_note_lengths(chordstart, notes-1);
        }
        v->inchord = 0;
        v->chordcount = 0;
        this->marknoteend();
        if(this->tuplecount > 0) 
            --this->tuplecount;
    }
}

/* handles a note in the abc */
void 
AbcStore::note(int decorators[Abc::DECSIZE], 
    AbcMusic::ClefType *clef, char accidental, int mult, 
    char note, int xoctave, int n, int m)
{
    featureDesc &fd = this->notelist[notes];
    fd.decotype = 0;
    if(this->voicesused == 0) 
        this->bodystarted = 1;
    if(v == nullptr) 
    {
        this->error("Internal error - no voice allocated");
        return;
    }
    if(this->gracenotes && this->ignore_gracenotes) 
        return;
    
    int octave;
    if(v->octaveshift == 0) 
        octave = xoctave + clef->octave_offset;
    else 
        octave = xoctave + v->octaveshift;

    int num = n;
    int denom = m;
    if(v->inchord) 
        v->chordcount = v->chordcount + 1;
    if(this->tuplecount > 0) 
    {
        num = num * this->tfact_num;
        denom = denom * this->tfact_denom;
        if(this->tnote_num == 0)
        {
            this->tnote_num = num;
            this->tnote_denom = denom;
        } 
        else
        {
            if(tnote_num * denom != num * tnote_denom)
            {
                if(!this->specialtuple && this->quiet == -1)
                    this->warning("Different length notes in tuple");
            }
        }
        if((!this->gracenotes) && (!v->inchord))
            this->tuplecount--;
    }
    if((!v->ingrace) && (!v->inchord)) 
        this->hornp(num, denom*(v->default_length));
    else 
        this->last_num = 3; /* hornpiping (>) cannot follow chord or grace notes */

    if((!v->ingrace) && ((!v->inchord)||(v->chordcount==1))) 
        this->genMidi.addunits(num, denom*(v->default_length));

    /* linear temperament support */
    int pitch, dummy, pitch_noacc;
    if(v->drumchannel) 
        pitch = this->barepitch(note, accidental, mult, octave);

    pitch = this->pitchof_b(note, accidental, mult, octave, propagate_accidentals,&active_pitchbend);
    pitch_noacc = this->pitchof_b(note, 0, 0, octave, 0, &dummy);
    if(decorators[Abc::FERMATA] && !this->ignore_fermata) 
    {
        if(this->fermata_fixed) 
            this->addfract(&num, &denom, 1, 1);
        else 
            num = num*2;
    }
    if(v->chordcount == 1) 
    {
        v->chord_num = num*4;
        v->chord_denom = denom*(v->default_length);
    }
    if(decorators[Abc::ROLL] || 
       decorators[Abc::ORNAMENT] || 
       decorators[Abc::TRILL])
    {
        if(v->inchord) 
        {
            this->error("Rolls and trills not supported in chords");
            fd.pitchline = pitch_noacc; /* [SS] 2013-03-26 */
            fd.bentpitch = active_pitchbend; /* [SS] 2013-03-26 */
            this->addfeature(Abc::NOTE, pitch, num*4, denom*2*(v->default_length)); /* [SS] */
        } 
        else 
        {
            if(this->easyabcmode) /* [SS] 2011-07-18 */ 
            {
                this->addfeature(Abc::META, 0, 
                    this->parser->lineno, this->parser->lineposition);
            }
            if(decorators[Abc::TRILL]) 
            {
                fd.decotype = this->notesdefined;
                /*dotrill(note, octave, num, denom, pitch);*/
                this->dotrill_setup(note, octave, num, denom, pitch);
                this->addfeature(Abc::NOTE, pitch, num*4, denom*(v->default_length));
            }
            else 
            if(decorators[Abc::ORNAMENT]) 
                this->doornament(note, octave, num, denom, pitch);
            else 
            { 
                fd.decotype = notesdefined; /* [SS] 2012-06-29 */
                /*doroll(note, octave, num, denom, pitch);*/
                this->doroll_setup(note, octave, num, denom, pitch);
                fd.bentpitch = active_pitchbend;
                this->addfeature(Abc::NOTE, pitch, num*4, denom*(v->default_length));
            }
            this->marknote(); /* [SS] 2019-10-13 */
        } /* end of else block for not in chord */
    } /* end of if block for ROLL,ORNAMENT,TRILL */
    else 
    {
        if(decorators[Abc::STACCATO] || decorators[Abc::BREATH]) 
        {
            if(v->inchord) 
            {
                if(v->chordcount == 1) 
                    this->addfeature(Abc::REST, pitch, num*4, denom*(v->default_length));
                fd.pitchline = pitch_noacc;
                fd.bentpitch = active_pitchbend;
                this->addfeature(Abc::NOTE, pitch, num*4, denom*2*(v->default_length));
            } 
            else 
            {
                fd.pitchline = pitch_noacc;
                if(this->easyabcmode) /* [SS] 2011-07-18 */ 
                {
                    this->addfeature(Abc::META, 0, this->parser->lineno, 
                        this->parser->lineposition);
                }
                fd.bentpitch = active_pitchbend;
                this->addfeature(Abc::NOTE, pitch, num*4, denom*2*(v->default_length));
                this->marknotestart();
                this->addfeature(Abc::REST, pitch, num*4, denom*2*(v->default_length));
                this->marknoteend();
            }
        }
        else 
        {
            fd.pitchline = pitch_noacc;
            if(this->easyabcmode && !v->inchord)
            {
                this->addfeature(Abc::META, 0, this->parser->lineno,
                    this->parser->lineposition);
            }
            fd.bentpitch = active_pitchbend;
            this->addfeature(Abc::NOTE, pitch, num*4, denom*(v->default_length));
            if(!v->inchord)
                this->marknote();
            if((v->inslur) && (!v->ingrace)) 
                this->addfeature(Abc::SLUR_TIE, 0, 0, 0);
        }
    }
    /*printf(": notes = %d v->thisend = %d\n",notes,v->thisend); [SS] 2019-10-13*/
}

void 
AbcStore::temperament(char const* line)
{
    char const *p = line+13;
    float dt[12];
    int i = sscanf(p, "%f %f %f %f %f %f %f %f %f %f %f %f",
        &dt[0], &dt[1], &dt[2], &dt[3],& dt[4], &dt[5],
        &dt[6], &dt[7], &dt[8], &dt[9], &dt[10], &dt[11]);
    if(i!=12) 
    {
        this->error("%%temperament expects 12 numbers");
        return;
    }
    for(i=0;i<12;i++) 
    {
        if(fabs(dt[i]) > 200.0) 
        {
            this->error("%%temperament detune cannot be greater than +- 200 cents");
            return;
        }
    }
    for(i=0;i<12;i++) 
        this->temperament_dt[i] = dt[i];
    this->parser->temperament = TEMPERDT;
}

/* pitchwheel range +/- 2 semitones according to General MIDI specification */
void 
AbcStore::microtone(int dir, int a, int b)
{
    int bend;
    /* resolution of 14bit -- order of bytes is inverted for pitchbend */
    this->parser->setmicrotone.num = dir*a;
    this->parser->setmicrotone.denom = b;
    if(a == 0) 
    {
        bend = 8192;
        this->parser->microtone = 0;
        this->parser->setmicrotone.num = 0;
        this->parser->setmicrotone.denom = 0;
        return;
    }
    else 
    {
        bend = dir*((int)(4096.0*a/b + 0.5))+8192; /* [HL] 2015-05-14 */
        bend = bend<0?0:(bend>16383?16383:bend);
    }
    this->active_pitchbend = bend;
    this->parser->microtone = 1;
}

void 
AbcStore::normal_tone()
{ 
    /* event_specific("MIDI", "pitchbend 0 64"); [SS] 2006-09-30 */
    this->parser->microtone = 0;
}

/* handler for the guitar chords */
void 
AbcStore::handle_gchord(char const *s)
{
    if(ignore_guitarchords == 1) return; /* [SS] 2019-12-09 */
    if((*s >= '0') && (*s <= '5')) 
    {
        /* a 1, 2, 3, 4 or 5 has been found in a guitar chord field */
        // this->finger(s); // event_finger -no-op
        return;
    }
    char const *p = s;
    if(*p == '(') 
        p = p+1; /* ignore leading paren */

    int note, bassnote;
    if((*p >= 'A') && (*p <= 'G')) 
    {
        note = *p - 'A' + 'a';
        bassnote = 0;
        p = p + 1;
    } 
    else 
    {
        if((*p >= 'a') && (*p <= 'g')) 
        {
            note = *p;
            bassnote = 1;
            p = p + 1;
        } 
        else 
        {
            if(strchr("_^<>@" , (int)*p) == NULL) 
            {
                if(!this->silent) 
                    this->error("Guitar chord does not start with A-G or a-g");
            }
            return;
        }
    }
    char accidental;
    p = this->get_accidental(p, &accidental);
    int basepitch = this->pitchof(note, accidental, 1, 0, 0) - this->middle_c;
    char name[9];
    int i = 0;
    while ((i<9) && 
      (*p != ' ') && (*p != '\0') && (*p != '(') && (*p != '/') && (*p != ')')) 
    {
        /* also ignore closing parentheses ')'  [SS] 2005-03-19*/
        name[i] = *p;
        i = i+1;
        p = p + 1;
    }
    int inversion = -1;
    if(*p == '/') 
    {
        p = p + 1;
        if((*p >= 'A') && (*p <= 'G')) 
        {
            note = (int)*p - 'A' + 'a';
            p = p + 1;
            p = this->get_accidental(p, &accidental);
            inversion = this->pitchof(note, accidental, 1, 0, 0) - middle_c;
        } 
        else 
        if((*p >= 'a') && (*p <= 'g')) 
        {
            note = (int)*p;
            p = p + 1;
            p = this->get_accidental(p, &accidental);
            inversion = this->pitchof(note, accidental, 1, 0, 0) - middle_c;
        } 
        else 
        if(!this->silent) 
            this->error(" / must be followed by A-G or a-g in gchord");
    }
    name[i] = '\0';
    int chordno = this->getchordnumber(name);
    if(chordno == 0) 
    {
        char msg[200];
        /* [SS] 2015-07-08 */
        snprintf(msg, 200, 
            "Unrecognized chord name \"%s\"\n(Hint: use %%MIDI chordname to define it. eg %%MIDI chordname sus4 0 4 7).", 
            name);
        this->error(msg);
        chordno = 1; /* defaults to major */
    } 
    else 
    {
        /* only record voice as having chords if we recognize chord type */
        v->hasgchords = 1;
        gchordvoice = v->indexno;
    }
    if(bassnote) 
    {
        chordno = -1;
    }
    this->addfeature(Abc::GCHORD, basepitch, inversion, chordno);
}