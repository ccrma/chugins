#include "AbcStore.h"
#include "AbcPort.h"
#include <cmath> // log10

char const *AbcStore::version = "4.59 June 27 2021 abc2midi";

static const char *sOneOctave = "cdefgab";
static int sScale[7] = {0, 2, 4, 5, 7, 9, 11}; // diatonic 'major' mode

AbcStore::AbcStore(AbcParser *p) :
    IAbcParseClient(p),
    temperament_dt {0.0,0.0,0.0,0.0, 0.0,0.0,0.0,0.0, 0.0,0.0,0.0,0.0},
    temponame {
        "larghissimo" , "adagissimo", "lentissimo",
        "largo", "adagio", "lento", "larghetto", "adagietto", "andante",
        "andantino", "moderato", "allegretto", "allegro", "vivace",
        "vivo", "presto", "allegrissimo", "vivacissimo", "prestissimo"
    },
    temporate {
        40, 44, 48,
        56, 59, 62, 66, 76, 88,
        96, 104, 112, 120, 168,
        180, 192, 208, 220, 240
    },
    abcm2psoptions {"setbarnb"}
{
    this->done_with_barloc = 0;
    this->velocitychange = 15;
    this->chordstart = 0;
    this->propagate_accidentals = 2;
    this->octave_size = 12.0f;
    this->fifth_size = 7.0f; /* default to 12-edo */
    this->sharp_size = 1.0f;
    this->microstep_size = 1.0f;
    this->started_parsing = 0;
    this->v1index = -1;
    this->ignore_fermata = 0;
    this->ignore_gracenotes = 0;
    this->ignore_guitarchords = 0;
    this->separate_tracks_for_words = 0;

    this->bodystarted = 0;
    this->harpmode = 0;
    this->easyabcmode = 1;
    this->retuning = 0;
    this->bend =  8192;
    this->comma53 = 0;
    this->silent = 0;

    this->notesdefined = 0;
    this->voicecount = 0;
    this->voicesused = 0;

    this->numsplits = 0;
    this->splitdepth = 0;

    this->maxtexts = INITTEXTS;
    this->ntexts = 0;

    this->verbose = 0;
    this->titlenames = 0;
    this->ratio_standard = 0;
    this->quiet = -1;
    this->fermata_fixed = 0;
    this->apply_fermata_to_chord = 0;

    this->default_tempo = 120;

    this->number_of_abcm2ps_options = 1;

    this->userfilename = 0; // int/bool
    this->outname.clear();
    this->outbase.clear();

    this->programbase = 0;

    this->csmfilename.clear();

    this->extended_overlay_running = 0;
    this->default_middle_c = 60;
    this->default_retain_accidentals = 2;
    this->default_fermata_fixed = 0;
    this->default_ratio_a = 2; // for swing
    this->default_ratio_b = 6;
}

AbcStore::~AbcStore()
{}

void
AbcStore::Init(int argc, char const*argv[], std::string *filename)
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
                if(this->bend > 16383) this->bend = 16383;
                if(this->bend < 0) this->bend = 0;
                printf("-TT: bend = %d\n", this->bend);
            }
        }
    } 
    if(this->getarg("-OCC",argc,argv) != -1) 
        this->parser->oldchordconvention = 1;
    if(this->getarg("-silent",argc,argv) != -1) 
        this->silent = 1;

    /* allocate space for notes, we'll grow it as needed */
    this->maxFeatures = 500;
    this->featurelist.resize(this->maxFeatures);
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
            this->outbase.assign(this->outbase.substr(0, s));
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
        else 
            this->warning("No number given, ignoring -n option");
    } 

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

/* look for argument 'option' in command line */
int
AbcStore::getarg(char const *option, int argc, char const *argv[])
{
    int place = -1;
    for(int j = 0; j < argc; j++)
    {
        if(strcmp(option, argv[j]) == 0)
            place = j + 1;
    }
    return place;
}

/* ----------------------------------------------------------------- */

AbcStore::voicecontext *
AbcStore::newvoice(int voiceIndex)
{
    this->voicecount++; // initialized to 0
    voicecontext *vc = new voicecontext(this->voicecount, voiceIndex,
                        this->global, this->time_num, this->time_denom);
    this->vaddr[this->voicecount] = vc;
    return vc;
}

void
AbcStore::clearvoicecontexts()
{
    voicecontext *p = this->head;
    voicecontext *q;
    while(p != nullptr) 
    {
        q = p->next;
        delete p;
        p = q;
    }
    this->head = nullptr;
}

AbcStore::voicecontext *
AbcStore::getvoicecontext(int voiceNo)
{
    struct voicecontext *p = this->head;
    struct voicecontext *q = nullptr;
    int i, j;
    while((p != nullptr) && (p->voiceno != voiceNo)) 
    {
        q = p;
        p = p->next;
    }
    if(p == nullptr) 
    {
        p = this->newvoice(voiceNo);
        if(q != nullptr) 
            q->next = p;
    }
    if(this->head == nullptr) 
        this->head = p;

    /* check that key signature mapping is set if global
     * key signature set.                             */
    if(p->keyset == 0 && global.keyset)
    {
        p->keyset = 1;
        for(i=0; i<7; i++) 
        {
            p->basemap[i] = global.basemap[i];
            p->basemul[i] = global.basemul[i];
            for(j=0;j<10;j++) 
            {
                p->workmap[i][j] = global.workmap[i][j];
                p->workmul[i][j] = global.workmul[i][j];
            }
        }
    }
    mtime_num = p->active_meter_num; /* [SS] 2012-11-08 */
    mtime_denom = p->active_meter_denom; /* [SS] 2012-11-08 */
    return p;
}

/* ----------------------------------------------------------------- */

void 
AbcStore::setup_chordnames()
{
    static int list_Maj[3] = {0, 4, 7};
    static int list_m[3] = {0, 3, 7};
    static int list_7[4] = {0, 4, 7, 10};
    static int list_m7[4] = {0, 3, 7, 10};
    static int list_m7b5[4] = {0, 3, 6, 10};
    static int list_maj7[4] = {0, 4, 7, 11};
    static int list_M7[4] = {0, 4, 7, 11};
    static int list_6[4] = {0, 4, 7, 9};
    static int list_m6[4] = {0, 3, 7, 9};
    static int list_aug[3] = {0, 4, 8};
    static int list_plus[3] = {0, 4, 8};
    static int list_aug7[4] = {0, 4, 8, 10};
    static int list_dim[3] = {0, 3, 6};
    static int list_dim7[4] = {0, 3, 6, 9};
    /* static int list_9[5] = {0, 4, 7, 10, 2}; [SS] 2017-10-18 */
    static int list_9[5] = {0, 4, 7, 10, 14};
    /* static int list_m9[5] = {0, 3, 7, 10, 2}; */
    static int list_m9[5] = {0, 3, 7, 10, 14};
    /* static int list_maj9[5] = {0, 4, 7, 11, 2}; */
    static int list_maj9[5] = {0, 4, 7, 11, 14};
    /* static int list_M9[5] = {0, 4, 7, 11, 2}; */
    static int list_M9[5] = {0, 4, 7, 11, 14};
    /* static int list_11[6] = {0, 4, 7, 10, 2, 5}; */
    static int list_11[6] = {0, 4, 7, 10, 14, 17};
    static int list_dim9[5] = {0, 3, 6, 9, 13}; /* [SS] 2016-02-08 */
    static int list_sus[3] = {0, 5, 7};
    static int list_sus4[3] = {0, 4, 7}; /* [SS] 2015-07-08 */
    static int list_sus9[3] = {0, 2, 7};
    static int list_7sus4[4] = {0, 5, 7, 10};
    static int list_7sus9[4] = {0, 2, 7, 10};
    static int list_5[2] = {0, 7};
    
    this->addchordname("", 3, list_Maj);
    this->addchordname("m", 3, list_m);
    this->addchordname("7", 4, list_7);
    this->addchordname("m7", 4, list_m7);
    this->addchordname("m7b5", 4, list_m7b5);
    this->addchordname("maj7", 4, list_maj7);
    this->addchordname("M7", 4, list_M7);
    this->addchordname("6", 4, list_6);
    this->addchordname("m6", 4, list_m6);
    this->addchordname("aug", 3, list_aug);
    this->addchordname("+", 3, list_plus);
    this->addchordname("aug7", 4, list_aug7);
    this->addchordname("dim", 3, list_dim);
    this->addchordname("dim7", 4, list_dim7);
    this->addchordname("9", 5, list_9);
    this->addchordname("m9", 5, list_m9);
    this->addchordname("maj9", 5, list_maj9);
    this->addchordname("M9", 5, list_M9);
    this->addchordname("11", 6, list_11);
    this->addchordname("dim9", 5, list_dim9);
    this->addchordname("sus", 3, list_sus);
    this->addchordname("sus4", 3, list_sus4); /* [SS] 2015-07-08 */
    this->addchordname("sus9", 3, list_sus9);
    this->addchordname("sus2", 3, list_sus9); /* [SS] 2015-07-08 */
    this->addchordname("7sus2", 4, list_7sus9); /* [SS] 2015-07-08 */
    this->addchordname("7sus4", 4, list_7sus4);
    this->addchordname("7sus9", 4, list_7sus9);
    this->addchordname("5", 2, list_5);
}

void 
AbcStore::addchordname(char const *s, int len, int notes[])
{
    if(strlen(s) > 7) 
    {
        this->error("Chord name cannot exceed 7 characters");
        return;
    }
    if(len > 10) 
    {
        this->error("Named chord cannot have more than 10 notes");
        return;
    }
    int done = 0;
    //  check for chord-update
    for(AbcGenMidi::Chord &c : this->chords)
    {
        if(c.name == s)
        {
            c.notes.clear();
            for(int i=0;i<len;i++)
                c.notes.push_back(notes[i]);
            done;
            break;
        }
    }
    if(!done)
    {
        AbcGenMidi::Chord c; // <-- new chord
        c.name = s;
        for(int i=0;i<len;i++)
            c.notes.push_back(notes[i]);
        this->chords.push_back(c);
    }
}

int
AbcStore::getchordnumber(char const *s)
{
    for(int i=0;i<this->chords.size();i++)
    {
        if(this->chords[i].name == s)
            return i+1; // <<---- number, not index
    }
    return 0;
}

/* for all defined voices update p->active_meter_num and
 * p->active_meter_denom
 */
void
AbcStore::meter_voice_update(int n, int m)
{
    voicecontext *p = this->head;
    while(p != nullptr)
    {
        p->active_meter_num = n;
        p->active_meter_denom = n;
        p = p->next;
    }
}

/* called while debugging */
void 
AbcStore::dump_voicecontexts() 
{
    voicecontext *p;
    voicecontext *q;

    p = this->head;
    printf("dump_voicecontexts()\n");
    while(p != NULL) 
    {
        printf("num %d index %d gchords %d words %d drums %d drone %d tosplit %d fromsplit %d\n",
            p->voiceno,p->indexno,p->hasgchords,p->haswords,p->hasdrums,p->hasdrone,p->tosplitno,p->fromsplitno);
        q = p->next;
        p = q;
    }
}


/* support functions for split voices --------------- */
/* This function finds the beginning of voice:indexno
 * in the feature array. It starts looking from j = start
 */
int
AbcStore::locate_voice(int start, int indexno)
{
    int j = start;
    while(j < this->nextFeature) 
    {
        if(this->featurelist[j].feature == Abc::VOICE && 
            this->featurelist[j].pitch == indexno) 
        {
            return j;
        }
        j++;
    }
    return j;
}

/* The function scans the contents of the feature[] array between,
   the last resync point and sync_to (or end of the feature array)
   depending on the ignorecurrentbar flag. It counts the time
   of all notes and rests in the topvoice voice belonging to
   voice vv and adds rests each time it encounters a bar line,
   to synchronize this voice with its topvoice. It also copies
   all repeat signs and dynamic markings in the topvoice and
   inserts them in the current voice. 

   This function is called by event_split_voice, complete_all_split_voices
   and event_bar (if split voices are present).

   The last_resync_point keeps track of what has been scanned,
   so we only scan the topvoice once.
*/
void 
AbcStore::sync_voice(voicecontext *vv, int sync_to, int ignorecurrentbar)
{
    char command[40];
    int maxnotes, begin = 0;
    char message[128]; /* [SS] 2019-06-20 */
    int insidechord = 0;
    char const *p;
    int j = vv->last_resync_point;
    int snum=0, sdenom=1;
    int voiceno = vv->topvoiceno;
    int indexno = vv->topindexno;

    /* to bypass double bar or single bar at beginning */
    /* we have already synced to last_resync_point so we continue from here. */
    if(voiceno != 1 || j > 2)
        j = this->locate_voice(j, indexno);
    else 
        j++; /* bypass bar */
    if(ignorecurrentbar) 
        maxnotes = sync_to;
    else 
        maxnotes = this->nextFeature-1; /*ignore last voice change */
    /*printf("syncing voice %d to %d from %d to %d \n",vv->indexno,indexno,j,maxnotes);*/
    while(j<=maxnotes) 
    {
        /*  dumpfeat(j,j); */
        FeatureDesc &fd = this->featurelist[j];
        switch(fd.feature) 
        {
        case Abc::VOICE:
            if(fd.pitch != indexno) 
                j = this->locate_voice(j, indexno);
            break;  /* [SDG] 2020-06-03 */
        case Abc::CHORDON:
            insidechord = 1;
            break;
        case Abc::CHORDOFF:
        case Abc::CHORDOFFEX:
            insidechord = 0;
            break;
        case Abc::SINGLE_BAR:
        case Abc::DOUBLE_BAR:
        case Abc::BAR_REP:
        case Abc::REP_BAR:
        case Abc::DOUBLE_REP:
            if(snum>0) 
            {  /* bypass REST if no notes processed */
                this->addfeature(Abc::REST, 0, snum, sdenom);
                /*printf("  added %d/%d to voice %d\n",snum,sdenom,vv->indexno);*/
                snum = 0;
                sdenom =1;
            } 
            this->addfeature(fd.feature, 0, 0, fd.denom); /* copy feature */
            break;
        case Abc::PLAY_ON_REP:
            {
                FeatureDesc &lastfd = this->featurelist[j-1];
                if(lastfd.feature == Abc::SINGLE_BAR || 
                    lastfd.feature == Abc::REP_BAR ||
                    lastfd.feature == Abc::VOICE) 
                {
                    this->addfeature(fd.feature, 0, 0, fd.denom);
                }
                else 
                {
                    sprintf(message,"expecting SINGLE_BAR or REP_BAR preceding"
                        " PLAY_ON_REP instead found %s at %d\n",
                        Abc::featname(lastfd.feature),  j-1);
                    this->error(message);
                }
            }
            break;
        case Abc::DYNAMIC:
            p = this->atext[fd.pitch].c_str();
            this->parser->skipspace(&p);
            this->parser->readstr(command, &p, 40);
            if(strcmp(command, "program") == 0) 
                this->textfeature(Abc::DYNAMIC, this->atext[fd.pitch].c_str());
            break;
        case Abc::CHANNEL:
            /* copy feature */
            this->addfeature(fd.feature, fd.pitch, 0, 0); 
            break;
        case Abc::TIME:
            /* copy feature */
            this->addfeature(fd.feature, fd.pitch, fd.num, fd.denom); 
            break;
        case Abc::SETTRIM:
            /* copy feature */
            this->addfeature(fd.feature, fd.pitch, fd.num, fd.denom); 
            break;
        case Abc::GRACEON:
            this->gracenotes = 1;
            break;
        case Abc::GRACEOFF:
            this->gracenotes = 0;
            break;
        case Abc::NOTE:
        case Abc::TNOTE:
        case Abc::REST:
            /*if(insidechord < 2) addfract(&snum,&sdenom,num[j],denom[j]);*/
            if(insidechord < 2 && !this->gracenotes) 
                AbcMusic::addFraction(&snum, &sdenom, fd.num, fd.denom); 
            if(insidechord) 
                insidechord++;
            begin = 1;
            break;
        default:
            break;
        }
        j++;
    }
    /* There are no more indexno notes between maxnotes and notes-1,
     *   so set sync point at the end.
     */
    vv->last_resync_point = this->nextFeature-1;
}

void
AbcStore::complete_all_split_voices()
{
    this->v = this->head;
    while(v != NULL) 
    {
        int splitno = v->tosplitno;
        if(splitno > -1) 
        {
            int voiceno = v->voiceno;
            int indexno = v->indexno;
            voicecontext *p = getvoicecontext(splitno);
            this->addfeature(Abc::VOICE, p->indexno, 0, 0);
            this->sync_voice(p, 0, 0);
            /* complete fraction of bar */

            int bar_num, bar_denom;
            this->genMidi.getBarUnits(&bar_num, &bar_denom);
            if(bar_num >0) 
            {
                this->addfeature(Abc::REST, 0, 
                    4*bar_num,  bar_denom); // XXX: 4?
            }
        }
        v = v->next;
    }
}

void
AbcStore::textfeature(Abc::FeatureType type, char const *s)
{
    this->atext[this->ntexts].assign(s);
    this->addfeature(type, this->ntexts, 0, 0);
    this->ntexts++;
    if(this->ntexts >= this->maxtexts)
    {
        // textextend:
        this->maxtexts *= 2;
        // NB: features must hold onto our index, not our value (or ptr)
        this->atext.resize(this->maxtexts); 
    }
}

/* place feature in internal table */
void
AbcStore::addfeature(Abc::FeatureType f, int p, int n, int d)
{
    FeatureDesc &fd = this->featurelist[this->nextFeature];
    fd.feature = f;
    fd.pitch = p;
    fd.num = n;
    fd.denom = d;
    fd.charloc = this->parser->lineposition;
    if((f == Abc::NOTE) || (f == Abc::REST) || (f == Abc::CHORDOFF)) 
    {
        AbcMusic::reduceFraction(&fd.num, &fd.denom);
    }
    this->nextFeature = this->nextFeature + 1;
    if(this->nextFeature >= this->maxFeatures) 
        this->extendFeaturelist();
}

void 
AbcStore::replacefeature(Abc::FeatureType f, int p, int n, int d, int loc)
{
    FeatureDesc &fd = this->featurelist[loc];
    fd.feature = f;
    fd.pitch = p;
    fd.num = n;
    fd.denom = d;
}

void 
AbcStore::insertfeature(Abc::FeatureType f, int p, int n, int d, int loc)
{
    int i;
    this->nextFeature++;
    if(this->nextFeature >= this->maxFeatures) 
        this->extendFeaturelist();

    // make space for one entry at loc
    for(i=this->nextFeature;i>loc;i--) 
        this->featurelist[i] = this->featurelist[i-1]; // struct copy

    FeatureDesc &fd = this->featurelist[loc];
    fd.feature = f;
    fd.pitch = p;
    fd.num = n;
    fd.denom = d;
    fd.pitchline = 0;
    fd.charloc = this->parser->lineposition;
    fd.bentpitch = 0;
    fd.decotype = 0;
}

/* The function switches the contents of the features in loc1 and loc2 */
void 
AbcStore::interchange_features(int loc1, int loc2)
{
    FeatureDesc tmp = this->featurelist[loc2];
    this->featurelist[loc2] = this->featurelist[loc1];
    this->featurelist[loc1] = tmp;
}

void
AbcStore::removefeatures(int locfrom, int locto)
{
    int i;
    int offset = locto - locfrom + 1;
    for(i=locfrom;i<this->nextFeature;i++)
        this->featurelist[i] = this->featurelist[i+offset];
    this->nextFeature -= offset;
}

/* remove one feature from featurelist, shift all remaining features down one. */
void 
AbcStore::removefeature(int loc)
{
    for(int i=loc;i<this->nextFeature;i++)
        this->featurelist[i] = this->featurelist[i+1];
    this->nextFeature--;
}

/* increase the number of abc elements the program can cope with */
void 
AbcStore::extendFeaturelist()
{
    if(this->verbose > 2) 
        this->warning("Extending note capacity");
    int newlimit = this->maxFeatures*2;
    this->featurelist.resize(newlimit); // memcpy magic happens herein
    this->maxFeatures = newlimit;
}

int
AbcStore::search_backwards_for_last_bar_line(int from)
{
    int found=0, j;
    j = from;
    while(!found && j>0)
    {
        FeatureDesc &fd = this->featurelist[j];
        Abc::FeatureType f = fd.feature;
        if(f == Abc::SINGLE_BAR || 
            f ==  Abc::DOUBLE_BAR ||
            f ==  Abc::BAR_REP    || 
            f ==  Abc::REP_BAR    ||
            f ==  Abc::PLAY_ON_REP ||
            f ==  Abc::DOUBLE_REP)  
        {
            found = 1; 
            break;
        }
        j--;
    }
    return j;
}

void 
AbcStore::recurse_back_to_original_voice()
{
    int previous_voice = v->fromsplitno;
    while(previous_voice >-1 && splitdepth > 0) 
    {
        this->v = this->getvoicecontext(previous_voice);
        previous_voice = v->fromsplitno;
        this->splitdepth--;
    }
    this->addfeature(Abc::VOICE, this->v->indexno, 0, 0);
    this->copymap(this->v);
}

/* work out accidentals to be applied to each note */
/* sf: number of sharps in key signature -7 to +7 */
void 
AbcStore::setmap(int sf, char map[7], int mult[7])
{
    int j;

    for(j=0; j<7; j++) 
    {
        map[j] = '=';
        mult[j] = 1;
    }
    if(sf >= 1) map['f'-'a'] = '^';
    if(sf >= 2) map['c'-'a'] = '^';
    if(sf >= 3) map['g'-'a'] = '^';
    if(sf >= 4) map['d'-'a'] = '^';
    if(sf >= 5) map['a'-'a'] = '^';
    if(sf >= 6) map['e'-'a'] = '^';
    if(sf >= 7) map['b'-'a'] = '^';
    if(sf <= -1) map['b'-'a'] = '_';
    if(sf <= -2) map['e'-'a'] = '_';
    if(sf <= -3) map['a'-'a'] = '_';
    if(sf <= -4) map['d'-'a'] = '_';
    if(sf <= -5) map['g'-'a'] = '_';
    if(sf <= -6) map['c'-'a'] = '_';
    if(sf <= -7) map['f'-'a'] = '_';
}

/* sets up working map at the start of each bar */
void 
AbcStore::copymap(voicecontext *v)
{
    int i,j;
    for(i=0; i<7; i++) 
    {
        for(j=0;j<10;j++) 
        {
            v->workmap[i][j] = v->basemap[i];
            v->workmul[i][j] = v->basemul[i];
            v->workmic[i][j].num = v->basemic[i].num;
            v->workmic[i][j].denom = v->basemic[i].denom;
        }
    }
}

/* apply modifiers to a set of accidentals */
void 
AbcStore::altermap(voicecontext *v,
    char modmap[7], int modmul[7], AbcMusic::fraction modmic[7])
{
    for(int i=0; i<7; i++) 
    {
        if(modmap[i] != ' ') 
        {
            v->basemap[i] = modmap[i];
            v->basemul[i] = modmul[i];
            v->basemic[i].num = modmic[i].num;
            v->basemic[i].denom = modmic[i].denom;
            /*printf("basemic[%d] = %d %d\n",i,modmic[i].num,modmic[i].denom);*/
        }
    }
}

int 
AbcStore::get_tempo_from_name(char const *s)
{
    if(s == nullptr) return 0; 
    for(int i=0;i<19;i++) 
    {
        if(stricmp(s, this->temponame[i]) == 0)
            return this->temporate[i];
    }
    return 0;
}

/* interprets Q: once default length is known */
void 
AbcStore::tempounits(int *t_num, int *t_denom)
{
    /* calculate unit for tempo */
    if(this->tempo_num == 0) 
    {
        *t_num = 1;
        *t_denom = this->global.default_length;
    } 
    else 
    {
        if(this->relative_tempo) 
        {
            *t_num = this->tempo_num;
            *t_denom = this->tempo_denom*this->global.default_length;
        } 
        else 
        {
            *t_num = this->tempo_num;
            *t_denom = this->tempo_denom;
        }
    }
}

/* called after the K: field has been reached, signifying the end of */
/* the header and the start of the tune */
void
AbcStore::headerprocess() 
{
    int t_num, t_denom;
    voicecontext *p;

    if(this->headerpartlabel == 1)
    {
        char pc = this->genMidi.partspec.c_str()[0];
        int p = pc - 'A';
        this->genMidi.part_start[p] = this->nextFeature;
        this->addfeature(Abc::PART, pc, 0, 0);
    };
    this->pastheader = 1;
    this->gracenotes = 0; /* not in a grace notes section */
    if (!this->timesigset) 
        this->warning("No M: in header, using default");

    /* calculate time for a default length note */
    if(this->global.default_length == -1) 
    {
        if(((float) this->time_num)/this->time_denom < 0.75) 
            this->global.default_length = 16;
        else 
            this->global.default_length = 8;
    }

    /* ensure default_length is defined for all voices */
    p = this->head;
    while((p != nullptr)) 
    {
        if(p->default_length == -1) 
            p->default_length = this->global.default_length;
        p = p->next;
    }

    this->genMidi.zerobar();
    this->genMidi.set_meter(this->time_num, this->time_denom);
    if(this->hornpipe) 
    {
        if ((time_denom != 4) || ((time_num != 2) && (time_num != 4))) 
        {
            this->error("Hornpipe must be in 2/4 or 4/4 time");
            this->hornpipe = 0;
        }
    }

    this->tempounits(&t_num, &t_denom);
    /* make tempo in terms of 1/4 notes */
    this->current_tempo = (long) 60*1000000*t_denom/(Qtempo*4*t_num);
    this->voicesused = 0;
}


void 
AbcStore::dump_notestruct () 
{
  notestruct *s;
  for(int i=0;i<this->notesdefined; i++) 
  {
        s = this->noteaddr[i];
        printf("%d ",i);
        printf("%d ",s->index);
        printf("%d ",s->notetype);
        printf("%d ",s->pitch);
        printf("%d ",s->pitchup);
        printf("%d\n",s->pitchdown);
    }
} 

void 
AbcStore::free_notestructs() 
{
    for(int i=0;i<this->notesdefined;i++) 
    {
        notestruct *s = this->noteaddr[i];
        delete s;
        this->noteaddr[i] = nullptr;
    }
    this->notesdefined = 0;
}

void 
AbcStore::parse_mididef(char const *s)
{
    if(this->nmidicmd >= MAXMIDICMD) 
    {
        this->error("Too many %%MIDIdef's ");
        return;
    }
    char const *p = s;
    this->parser->skipspace(&p);
    int i = this->parser->readaln(this->midicmdname[nmidicmd], &p, 31);
    this->parser->skipspace(&p);
    this->midicmd[nmidicmd].assign(p);
    nmidicmd++;
}

/* The function handles the %%MIDIx command translating all the
   codewords into %%MIDI commands and sending the commands to
   event_midi(). If it encounters two %%MIDI controlstring commands,
   then it also sends a %%MIDI controlcombo so that both
   controlstrings are handled by the !shape! command.
*/
void
AbcStore::process_midix(char const *s)
{
    char const *p = s;
    char name[32];
    int i, j=1, k=0;
    while(j > 0) 
    {
        this->parser->skipspace(&p);
        j = this->parser->readaln(name, &p, 31);
        if(j < 1) 
            break;
        for(i=0; i<this->nmidicmd; i++) 
        {
            if(strcmp(this->midicmdname[i], name) == 0) 
                break;
        }
        if(i == nmidicmd) 
        {
            char msg[200];
            snprintf(msg, 200, 
                "cannot match %%%%MIDIx %s with corresponding MIDIdef", 
                name);
            this->error(msg);
        }
        if(k > 0 && strncmp(this->midicmd[i].c_str(), "controlstring", 12) == 0)
        {
            this->midi("controlcombo");
        }
        this->midi(this->midicmd[i].c_str()); 
        if(strncmp(this->midicmd[i].c_str(),"controlstring",12) == 0) 
            k++;
    }
}

/* Handles %%MIDI commands indirectly through the ->specific handler */
void
AbcStore::midi(char const *s)
{
    int ch;
    char command[40];
    char const *p = s;
    int done = 0;

    this->parser->skipspace(&p);
    this->parser->readstr(command, &p, 40);

    if(strcmp(command, "channel") == 0) 
    {
        this->parser->skipspace(&p);
        ch = this->parser->readnump(&p) - 1;
        if(v != NULL) 
        {
            if(ch == 9) 
                this->v->drumchannel = 1;
            else 
                this->v->drumchannel = 0;
        }
        if(ch <0 || ch >15) 
        {
            this->error("channel not between 1 and 16 ");
            ch = 1;
        }
        this->addfeature(Abc::CHANNEL, ch, 0, 0);
        if(v != NULL) 
        {
            if(v->midichannel == -1) 
                v->midichannel = ch; /* [SS] 2015-03-24 */
        }
        done = 1;
    }
    else 
    if((strcmp(command,"transpose") == 0) || 
        (strcmp(command,"rtranspose") == 0)) 
    {
        int neg=0;
        this->parser->skipspace(&p);
        if(*p == '+') 
            p = p + 1;
        if(*p == '-') 
        {
            p = p + 1;
            neg = 1;
        }
        this->parser->skipspace(&p);
        int val = this->parser->readnump(&p);
        if(neg) 
            val = - val;

        if(strcmp(command,"transpose") == 0)
            this->addfeature(Abc::GTRANSPOSE, val, 0, 0);
        else
            this->addfeature(Abc::RTRANSPOSE, val, 0, 0);
        done = 1;
    }
    else 
    if(strcmp(command, "C") == 0) 
    {
        this->parser->skipspace(&p);
        int val = this->parser->readnump(&p);
        this->middle_c = val;
        done = 1;
    }
    else 
    if(strcmp(command,"programbase") == 0) 
    {
        this->parser->skipspace(&p);
        int val = this->parser->readnump(&p);
        if(val != 0) val=1;
        this->programbase = val;
        done = 1;
    }
    else 
    if(strcmp(command, "nobarlines") == 0) 
    {
        this->propagate_accidentals = 0; 
        done = 1;
    }
    else 
    if(strcmp(command, "barlines") == 0) 
    {
        this->propagate_accidentals = 2;
        done = 1;
    }
    else 
    if(strcmp(command, "fermatafixed") == 0) 
    {
        this->fermata_fixed = 1;
        done = 1;
    }
    else 
    if(strcmp(command, "fermataproportional") == 0) 
    {
        this->fermata_fixed = 0;
        done = 1;
    }
    else 
    if(strcmp(command, "tuningsystem") == 0) 
    {
        this->parser->skipspace(&p);
        if(strcmp(p,"comma53") == 0) 
        {
            this->log(p);
            this->comma53 = 1;
            this->init_p48toc53();
            done = 1;
        }
    }
    else 
    if(strcmp(command, "ratio") == 0) 
    {
        int a, b;
        this->parser->skipspace(&p);
        b = this->parser->readnump(&p);
        this->parser->skipspace(&p);
        a = this->parser->readnump(&p);
        if((a > 0) && (b > 0)) 
        {
            this->ratio_a = a;
            this->ratio_b = b;
            if(this->ratio_a + this->ratio_b % 2 == 1) 
            {
                this->ratio_a = 2 * a;
                this->ratio_b = 2 * b;
            }
        } 
        else 
            this->error("Invalid ratio");
        done = 1;
    }
    else 
    if(strcmp(command, "grace") == 0)
    {
        int a, b;
        char msg[200];
        this->parser->skipspace(&p);
        a = this->parser->readnump(&p);
        if(*p != '/') 
        {
            this->error("Need / in MIDI grace command");
        } 
        else 
            p = p + 1;
        b = this->parser->readnump(&p);
        if((a < 1) || (b < 1) || (a >= b)) 
        {
            sprintf(msg, "%d/%d is not a suitable fraction", a, b);
            this->error(msg);
        } 
        else 
        {
            if(pastheader)
                this->addfeature(Abc::SETGRACE, 1, a, b);
            else 
            {
                this->gfact_num = a;
                this->gfact_denom = b;
            }
      }
      done = 1;
    }
    else 
    if(strcmp(command,"gracedivider") == 0)
    {
        this->parser->skipspace(&p);
        int b = this->parser->readnump(&p);
        if(b < 2)
        {
            char msg[200];
            sprintf(msg, "a number 2 or larger should follow MIDI gracedivider");
            this->error(msg);
        }
        if(this->pastheader)
            this->addfeature(Abc::SETGRACE, 0, 1, b);
        else 
        {
            this->gfact_denom = b; 
            this->gfact_method = 0;
        }
        done = 1;
    }
    else 
    if(strcmp(command, "trim") == 0) 
    {
        int a, b;
        this->parser->skipspace(&p);
        a = this->parser->readnump(&p);
        if(*p != '/')
            this->error("Need / in MIDI trim command (eg trim 1/4)");
        else 
        {
            p = p + 1;
            b = this->parser->readnump(&p);
            if(this->v != nullptr)
                this->addfeature(Abc::SETTRIM, 1, 4*a, b*v->default_length);
            else 
            {
                if(global.default_length == -1) 
                    this->error("Need to define L: before trim command: trim command ignored.");
                else 
                    this->addfeature(Abc::SETTRIM,1,4*a, b*global.default_length);
            }
        }
        done = 1;
    }
    else 
    if(strcmp(command, "expand") == 0)
    {
        int a, b;
        this->parser->skipspace(&p);
        a = this->parser->readnump(&p);
        if(*p != '/')
            this->error("Need / in MIDI expand command (eg trim 1/4)");
        else 
        {
            p = p + 1;
            b = this->parser->readnump(&p);
            if(v != nullptr)
                this->addfeature(Abc::EXPAND, 1, 4*a, b*v->default_length);
            else 
            {
                if(global.default_length == -1) 
                    this->error("Need to define L: before expand command: expand command ignored.");
                else 
                    this->addfeature(Abc::EXPAND, 1, 4*a, b*global.default_length);
            }
        }
        done = 1;
    }
    else 
    if(strcmp(command, "gchordon") == 0) 
    {
        this->addfeature(Abc::GCHORDON, 0, 0, 0);
        done = 1;
    }
    else 
    if(strcmp(command, "gchordoff") == 0) {
        this->addfeature(Abc::GCHORDOFF, 0, 0, 0);
        done = 1;
    }
    else 
    if(strcmp(command, "chordname") == 0) 
    {
        char name[20];
        int notes[10];
        this->parser->skipspace(&p);
        int i = 0;
        while((i<19) && (*p != ' ') && (*p != '\0')) 
        {
            name[i] = *p;
            p = p + 1;
            i = i + 1;
        }
        name[i] = '\0';
        if(*p != ' ')
            this->error("Bad format for chordname command");
        else 
        {
            i = 0;
            while((i<=10) && (*p == ' ')) 
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
    if(strcmp(command, "temperamentlinear") == 0)
    {
        double octave_cents=0.0;
        double fifth_cents=0.0;
        this->parser->temperament = TEMPERLN;
        this->middle_c = 60;
        if(sscanf(p," %lf %lf ",&octave_cents,&fifth_cents) == 2) 
        {
            this->octave_size = octave_cents;
            this->fifth_size = fifth_cents;
            this->sharp_size = 7*fifth_size - 4*octave_size;
            this->microstep_size = sharp_size; 
            if(this->verbose) 
            { 
                char buf[100];
                snprintf(buf, 100, "temperamentlinear:\n"
                    "\targs: %lf %lf\n", octave_cents, fifth_cents);
                this->log(buf);
                snprintf(buf, 100, "\toctave_size = %.3f (%.3lf cents)\n"
                    "\tfifth_size = %.3f (%.3lf cents)\n"
                    "\twhole-tone size = %.3f (%.3lf cents)\n"
                    "\taccidental_size = %.3f (%.3lf cents)\n",
                    octave_size, octave_size, fifth_size, fifth_size,
                    2*fifth_size - octave_size, 2.0*fifth_size-octave_size,
                    sharp_size,sharp_size);
                this->log(buf);
            }
        }
        else
            this->error("Bad format for temperamentlinear command");
        done = 1;
    }
    else 
    if(strcmp(command, "temperamentequal") == 0)
    {
        double octave_cents;
        float acc_size = -1.0;
        int ndiv, fifth_index, sharp_steps;
        int narg = sscanf(p," %d %lf %d %d ",&ndiv, &octave_cents, &fifth_index, &sharp_steps);
        switch (narg) 
        {
        case 1:
            this->octave_size = 1200.0;
            this->fifth_size = AbcMusic::compute_fifth_size(octave_size, ndiv);
            acc_size = 7*this->fifth_size - 4*this->octave_size;
            break;
        case 2:
            this->octave_size = octave_cents;
            this->fifth_size = AbcMusic::compute_fifth_size(octave_size, ndiv);
            acc_size = 7*this->fifth_size - 4*this->octave_size;
            break;
        case 3:
            this->octave_size = octave_cents;
            if(fifth_index > 0.0) 
            {
                /* user-defined fifth size */
                this->fifth_size =  1.0 * fifth_index * this->octave_size/ndiv;
            }
            else 
            {
                /* automatically computed fifth size */
              this->fifth_size = AbcMusic::compute_fifth_size(this->octave_size, ndiv);
            }
            acc_size = 7*this->fifth_size - 4*this->octave_size;
            break;
        case 4:
            this->octave_size = octave_cents;
            if(fifth_index > 0.0) 
            {
                /* user-defined fifth size */
                this->fifth_size =  1.0 * fifth_index * this->octave_size/ndiv ;
            }
            else 
            {               
                /* automatically computed fifth size */
                this->fifth_size = AbcMusic::compute_fifth_size(this->octave_size, ndiv);
            }
            /* user-defined accidental size */
            acc_size = sharp_steps * this->octave_size/ndiv ;
            break;
        default:
            this->error("Bad format for temperamentequal command");
            break;
        }
        this->microstep_size = this->octave_size/ndiv;
        this->sharp_size = acc_size;
        if(verbose) 
        {
            char msg[100];
            snprintf(msg, 100, "temperamentequal:\n\targs (%d):", narg);
            this->log(msg);
            if(narg>=1) snprintf(msg, 100, " %d\n", ndiv);
            if(narg>=2) snprintf(msg, 100, " %.3lf", octave_cents);
            if(narg>=3) snprintf(msg, 100, " %d", fifth_index);
            if(narg>=4) snprintf(msg, 100, " %d", sharp_steps);
            this->log(msg);
            snprintf(msg, 100, 
                "\tndiv = %d\n"
                "\toctave_size = %.3lf cents\n"
                "\tfifth_size = %.3lf (%d steps)\n"
                "\twhole-tone size = %.3lf (%d steps)\n"
                "\taccidental_size = %.3lf (%d steps)\n",
                ndiv, octave_size, fifth_size, 
                (int) ((fifth_size/octave_size*ndiv) + 0.5),
                2.0*fifth_size-octave_size,
                (int) (((2*fifth_size-octave_size)*ndiv/octave_size) + 0.5),
                acc_size,
                (int) ((acc_size*ndiv/octave_size) + 0.5)
             );
            this->log(msg);
            snprintf(msg, 100, "\tmicrostep_size = %.3lf cents (1 step)\n",
                microstep_size); /* [HL] 2020-06-20*/
            this->log(msg);
        }
        if(acc_size < 0)
            this->warning("temperamentequal <ndiv> [octave_in_cents] [fifth_in_steps] [sharp_in_steps]\n"
			 "\tValue(s) cause(s) inconsistent (inverted) accidentals.\n"
			 "\tTry with different parameters as, for example,\n"
			 "\tsome multiple of <ndiv> for 'temperamentequal',\n"
			 "\tor force a suitable value for 'sharp_in_steps'.\n"
			 );
      
        this->parser->temperament = TEMPEREQ;
        this->middle_c = 60;
        done = 1;
    }
    else 
    if(strcmp(command, "temperamentnormal") == 0) 
    {
        this->parser->temperament = TEMPERNORMAL;
        this->parser->microtone = 0; // event_normal_tone
        this->middle_c = 60;
        done = 1;
    }
    else 
    if(strcmp(command,"drumon") == 0 && this->dotune) 
    { 
        if(this->v == nullptr) 
        {
            this->error("%%MIDI drumon must occur after the first K: header");
            return;
        }
        this->addfeature(Abc::DRUMON, 0, 0, 0);
        this->v->hasdrums = 1;
        this->drumvoice = v->indexno; /* [SS] 2010-02-09 */
        done = 1;
    }
    else
    if(strcmp(command,"drumoff") == 0) 
    {
       this->addfeature(Abc::DRUMOFF, 0, 0, 0);
       done = 1;
    }
    else 
    if(strcmp(command,"droneon") == 0 && this->dotune) 
    {
        if(this->v == nullptr) 
        {
            this->error("%%MIDI droneon must occur after the first K: header");
            return;
        }
        this->addfeature(Abc::DRONEON, 0, 0, 0);
        this->v->hasdrone = 1;
        if((this->dronevoice != 0) && 
           (this->dronevoice != this->v->indexno)) 
        {
            this->warning("Implementation limit: drones only supported in one voice");
        }
        this->dronevoice = v->indexno;
        done = 1;
    }
    else 
    if(strcmp(command,"droneoff") == 0)
    {
       this->addfeature(Abc::DRONEOFF, 0, 0, 0);
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
	if(strcmp(command, "harpmode") == 0) 
    {
        this->parser->skipspace(&p);
        this->harpmode = this->parser->readnump(&p);
        done = 1;
    }
    else 
    if(strcmp(command,"ptstress") == 0) 
    {  
        char inputfile[256]; /* [SS] 2011-07-04 */
        this->parser->skipspace(&p);
        strncpy(inputfile, p, 250);
        if(this->verbose) 
        {
            char msg[300];
            snprintf(msg, 300, "ptstress file = %s\n", inputfile);
            this->log(msg);
        }
        if(this->genMidi.parse_stress_params(inputfile) == -1) 
            this->genMidi.readstressfile(inputfile);
        this->genMidi.calculate_stress_parameters(this->time_num, this->time_denom); 
        this->genMidi.beatmodel = 2;
        if(this->genMidi.stressmodel && 
          this->genMidi.beatmodel != this->genMidi.stressmodel) 
            this->genMidi.beatmodel = this->genMidi.stressmodel;
        done = 1;
    }

    if(done == 0) 
    {
        /* add as a command to be interpreted later */
        this->textfeature(Abc::DYNAMIC, s);
    }
}

/* parse abc note and advance character pointer */
void 
AbcStore::parse_drummap(char const **s)
{
    int octave;
    char msg[80];

    int mult = 1;
    char accidental = ' ';
    char note = ' ';
    /* read accidental */
    switch (**s) 
    {
    case '_':
        accidental = **s;
        *s = *s + 1;
        if(**s == '_') 
        {
            *s = *s + 1;
            mult = 2;
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
        break;
    case '=':
        accidental = **s;
        *s = *s + 1;
        if(**s == '^') 
        {
            accidental = **s;
            *s = *s + 1;
        } 
        else 
        if(**s == '_') 
        { 
            accidental = **s;
            *s = *s + 1;
        } 
        break;
    default:
        /* do nothing */
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
                sprintf(msg, "Bad pitch specifier , after note %c", note);
                this->error(msg);
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
                if(**s == '\'') 
                {
                    sprintf(msg, "Bad pitch specifier ' after note %c", 
                        note + 'A' - 'a');
                    this->error(msg);
                    octave = octave + 1;
                    *s = *s + 1;
                }
            }
        }
    }

    /*printf("note = %d octave = %d accidental = %d\n",note,octave,accidental);*/
    char const *anoctave = "cdefgab";
    int scale[7] = {0, 2, 4, 5, 7, 9, 11};
    int midipitch = (int) (strchr(anoctave, note) - anoctave);
    if(midipitch <0 || midipitch > 6) 
    {
        this->error("Malformed note in drummap : expecting a-g or A-G");
        return;
    } 
    midipitch = scale[midipitch];
    if(accidental == '^') midipitch += mult;
    if(accidental == '_') midipitch -= mult;
    midipitch = midipitch + 12*octave + 60;
    this->parser->skipspace(s);
    int mapto = this->parser->readnump(s);
    if(mapto == 0) 
    {
        this->error("Bad drummap: expecting note followed by space and number");
        return;
    }
    if(mapto < 35 || mapto > 81) 
        this->warning("drummap destination should be between 35 and 81 inclusive");
    this->genMidi.drum_map(midipitch, mapto);
}

/* work out filename stem from tune title */
/* name length cannot exceed namelimit characters */
void 
AbcStore::extract_filename(char const *f)
{
    char buffer[256];
    char const *p = f;
    int i = 0;
    this->parser->skipspace(&p);
    /* avoid initial 'The' or 'the' */
    if((strncmp(p, "The", 3) == 0) || (strncmp(p, "the", 3) == 0)) 
    {
        p = p + 3;
        this->parser->skipspace(&p);
    }
    while((*p != '\0') && (i < namelimit)) 
    {
        if(isalnum(*p)) 
        {
            buffer[i] = *p;
            i = i + 1;
        }
        p = p + 1;
    }
    buffer[i] = '\0';
    if(i == 0) 
    {
        strcpy(buffer, "notitle");
        buffer[namelimit] = '\0';
    }
    strcpy(&buffer[strlen(buffer)], ".mid");
    this->outname = buffer;
    this->got_titlename = 1;
}

/* check that we are in not in chord, grace notes or tuple */
/* called at voice change */
void 
AbcStore::checkbreak()
{
    if(this->tuplecount != 0) 
    {
        this->error("Previous voice has an unfinished tuple");
        this->tuplecount = 0;
    }
    if(this->v->inchord != 0) 
    {
        this->error("Previous voice has incomplete chord");
        this->chordoff(1, 1);
    }
    if(v->ingrace != 0)
    {
        this->error("Previous voice has unfinished grace notes");
        this->v->ingrace = 0;
    }
} 

/* converts an in-header P: field to a list of part labels 
 * returns the number of distinct parts enountered.
 * caller can inspect return.  If == 1, it may be an in-body
 * partlabel (since the line between header and notes is ill-defined)
 */
/* e.g. P:A(AB)3(CD)2 becomes P:AABABABCDCD */
int 
AbcStore::flattenPartSpec(char const *spec, std::string *partspec)
{
    int i, j;
    int stack[10]; // index of begining of pattern seq
    char lastch =  ' '; /* [SDG] 2020-06-03 */
    char errmsg[80];
    int stackptr = 0;
    int spec_length = strlen(spec);
    int hitmap[26];
    int k = 0;
    char const *in = spec;
    memset(hitmap, 0, sizeof(hitmap));
    while(*in != 0 && k < spec_length) 
    { 
        k++;
        if(((*in >= 'A') && (*in <= 'Z')) || (*in == '(') || (*in == '.') ||
            (*in == ')') || (*in == '+') || (*in == '-') || (*in == ' ') || 
            ((*in >= '0') && (*in <= '9'))) 
        {
            if(*in == '.' || *in == ' ')  // for readability
            {
                in = in + 1;
                continue; 
            }

            if(*in == '+' || *in == '-')  // no longer supported
                in = in + 1;

            if((*in >= 'A') && (*in <= 'Z')) 
            {
                hitmap[*in - 'A'] = 1;
                partspec->push_back(*in);
                lastch = *in;
                in = in + 1;
            }
            if(*in == '(') 
            {
                if(stackptr < 10) 
                {
                    stack[stackptr] = partspec->size() - 1;
                    stackptr = stackptr + 1;
                } 
                else 
                    this->error("nesting too deep in part specification");
                in = in + 1;
            }
            if(*in == ')')  // nb: we lookahead for number within
            {
                in = in + 1;
                if(stackptr > 0) 
                {
                    int repeats, start, stop;
                    if((*in >= '0') && (*in <= '9')) 
                        repeats = this->parser->readnump(&in);
                    else 
                        repeats = 1;
                    stackptr = stackptr - 1;
                    start = stack[stackptr];
                    stop = partspec->size() - 1;
                    for(i=1; i<repeats; i++) 
                    {
                        for(j=0; j<((int) (stop-start)); j++) 
                        {
                            char c = partspec->at(start+j);
                            hitmap[c = 'A'] = 1;
                            partspec->push_back(partspec->at(start+j));
                        }
                    }
                } 
                else 
                    this->error("Too many )'s in part specification");
            }

            if((*in >= '0') && (*in <= '9')) 
            {
                // handles: A3 (not (AB)3),CASEis->parser->readnump(&in);
                int repeats = this->parser->readnump(&in);
                if(partspec->size() > 0) 
                {
                    for(i = 1; i<repeats; i++) 
                    {
                        hitmap[lastch - 'A'] = 1;
                        partspec->push_back(lastch);
                    }
                } 
                else 
                    this->error("No part to repeat in part specification");
            }
        } 
        else 
        {
            if(!silent) 
            {
                snprintf(errmsg, 80, 
                        "illegal character \'%c\' in part specification.\n"
                        "The P: is ignored.", *in);
                this->error(errmsg);
            }
            this->genMidi.parts = -1;
            break;
        }
    } /* end of spec */
    if(stackptr != 0) 
    {
        this->error("Too many ('s in part specification");
        return 0;
    }
    if(partspec->size() == 0)
        return 0;
    else
    {
        int numparts = 0;
        for(int i=0;i<26;i++)
        {
            if(hitmap[i]) 
                numparts++;
        }
        return numparts;
    }
}

/* store away broken rhythm context on encountering grace notes */
void 
AbcStore::stack_broken(voicecontext * v)
{
    v->broken_stack[0] = v->laststart;
    v->broken_stack[1] = v->lastend;
    v->broken_stack[2] = v->thisstart;
    v->broken_stack[3] = v->thisend;
    v->broken_stack[4] = v->brokentype;
    v->broken_stack[5] = v->brokenmult;
    v->broken_stack[6] = v->brokenpending;
    v->laststart = -1;
    v->lastend = -1;
    v->thisstart = -1;
    v->thisend = -1;
    v->brokenpending = -1;
}

/* remember any broken rhythm context after grace notes */
void 
AbcStore::restore_broken(voicecontext *v)
{
    if(v->brokenpending != -1) 
    {
        this->error("Unresolved broken rhythm in grace notes");
    }
    v->laststart = v->broken_stack[0];
    v->lastend = v->broken_stack[1];
    v->thisstart = v->broken_stack[2];
    v->thisend = v->broken_stack[3];
    v->brokentype = v->broken_stack[4];
    v->brokenmult = v->broken_stack[5];
    v->brokenpending = v->broken_stack[6];
}

/* multiply note length by a/b */
void 
AbcStore::lenmul(int n, int a, int b)
{
    FeatureDesc &fd = this->featurelist[n];
    if((fd.feature == Abc::NOTE) || (fd.feature == Abc::REST) || 
       (fd.feature == Abc::CHORDOFF) || (fd.feature == Abc::CHORDOFFEX)) 
    {
        fd.num *= a;
        fd.denom *= b;
        AbcMusic::reduceFraction(&fd.num, &fd.denom);
    }
}

/* adjust lengths of broken notes */
void 
AbcStore::brokenadjust()
{
    int num1, num2;

    switch(v->brokenmult) 
    {
    case 1:
        num1 = ratio_b;
        num2 = ratio_a;
        break;
    case 2:
        num1 = 7;
        num2 = 1;
        break;
    case 3:
        num1 = 15;
        num2 = 1;
        break;
    default:
        num1 = num2 = 1; /* [SDG] 2020-06-03 */
    }
    int denom12 = (num1 + num2)/2;
    int j;
    if(v->brokentype == Abc::LT) 
    {
        j = num1;
        num1 = num2;
        num2 = j;
    }
    int failed = 0;
    if((v->laststart == -1) || (v->lastend == -1) || 
        (v->thisstart == -1) || (v->thisend == -1)) 
    {
        failed = 1;
    } 
    else 
    {
        /* check for same length notes */
        FeatureDesc &lastfd = this->featurelist[v->laststart];
        FeatureDesc &thisfd = this->featurelist[v->thisstart];
        if(lastfd.num * thisfd.denom != thisfd.num * lastfd.denom)
            failed = 1;
    }
    if(failed)
        this->warning("Cannot apply broken rhythm");
    else 
    {
        /*
        printf("Adjusting %d to %d and %d to %d\n",
            v->laststart, v->lastend, v->thisstart, v->thisend);
        */
        for(j=v->laststart; j<=v->lastend; j++) 
            lenmul(j, num1, denom12);
        for(j=v->thisstart; j<=v->thisend; j++)
            lenmul(j, num2, denom12);
    }
}

int 
AbcStore::patchup_chordtie(int chordstart,int chordend)
{
    int i,tieloc;
    for(i=chordend;i>=chordstart;i--) 
    {
        if(this->featurelist[i].feature == Abc::NOTE && 
           this->featurelist[i+1].feature != Abc::TIE) 
        {
            this->insertfeature(Abc::TIE,0,0,0,i+1);
            tieloc = i+1;
        }
    }
    return tieloc;
}

/* voice data structure keeps a record of last few notes encountered */
/* in order to process broken rhythm. This is called at the start of */
/* a note or chord */
void 
AbcStore::marknotestart()
{
    this->v->laststart = this->v->thisstart;
    this->v->lastend = this->v->thisend;
    this->v->thisstart = this->nextFeature - 1;
}

/* voice data structure keeps a record of last few notes encountered */
/* in order to process broken rhythm. This is called at the end of */
/* a note or chord */
void 
AbcStore::marknoteend()
{
    this->v->thisend = this->nextFeature - 1;
    if(this->v->brokenpending != -1) 
    {
        this->v->brokenpending = this->v->brokenpending + 1;
        if(this->v->brokenpending == 1) 
        {
            this->brokenadjust();
            this->v->brokenpending = -1;
        }
    }
}

/* when handling a single note, not a chord, marknotestart() and */
/* marknoteend() can be called together */
void 
AbcStore::marknote()
{
    this->marknotestart();
    this->marknoteend();
}

/* If we have used R:hornpipe, this routine modifies the rhythm by */
/* applying appropriate broken rhythm */
void 
AbcStore::hornp(int num, int denom)
{

    if(this->hornpipe && (this->nextFeature > 0) && 
       (this->featurelist[this->nextFeature-1].feature != Abc::GT)) 
    {
        if((num*last_denom == last_num*denom) && (num == 1) &&
            (denom*this->time_num == 32)) 
        {
            int bar_num, bar_denom;
            this->genMidi.getBarUnits(&bar_num, &bar_denom);
            if(((time_num == 4) && (bar_denom == 8)) ||
               ((time_num == 2) && (bar_denom == 16))) 
            {
                /* addfeature(GT, 1, 0, 0); */
                v->brokentype = Abc::GT;
                v->brokenmult = 1;
                v->brokenpending = 0;
            }
        }
        last_num = num;
        last_denom = denom;
    }
}

/* Computes MIDI pitch ignoring any key signature.
 * Required for drum track
 */
int 
AbcStore::barepitch(char note, char accidental, int mult, int octave) 
{
    int accidental_size = 1;
    int p = (int) (strchr(sOneOctave, note) - sOneOctave);
    p = sScale[p];
    if(accidental == '^') p = p + mult*accidental_size;
    if(accidental == '_') p = p - mult*accidental_size;
    return p + 12*octave + this->middle_c;
}

/* computes MIDI pitch for note. If global temperament is set,
   it will apply a linear temperament and return a
   pitchbend. If propagate_accs == 2, apply any accidental to all
   instances of  that note of the same pitch irrespective of the
   octave in the bar. If propagate_accs == 1, apply any accidental
   to all notes of the same pitch and same octave in the bar.
   If propagate_accs = 0, do not apply the accidental to other
   notes in the bar.
*/
int 
AbcStore::pitchof_bend(char note, char accidental, int mult, int octave, 
    int propagate_accs, int *pitchbend)
{
    const float accidental_size = this->sharp_size/100.0;
    const float tscale[7] = 
    {
        0,
        (2*this->fifth_size-this->octave_size),
        (4*this->fifth_size-2*this->octave_size),
        (-1*this->fifth_size+this->octave_size),
        this->fifth_size,
        (3*this->fifth_size-this->octave_size),
        (5*this->fifth_size-2*this->octave_size) 
    };

    char acc = accidental;
    int mul = mult;
    int noteno = (int)note - 'a';
    int a,b;
    int j;

    if(acc == ' ' && !this->parser->microtone) 
    {  
        /* no accidentals, apply current state */
        acc = this->v->workmap[noteno][octave+4];
        mul = this->v->workmul[noteno][octave+4];
        a = this->v->workmic[noteno][octave+4].num;
        b = this->v->workmic[noteno][octave+4].denom;
        this->microtone(1, a, b);
    } 
    else 
    {  
        /* some accidentals save the state if propagate_accs != 0 */
        if(propagate_accs) 
        {
            if(propagate_accs == 1) 
            {  
                /* accidentals applies to only current octave */
                this->v->workmap[noteno][octave+4] = acc;
                this->v->workmul[noteno][octave+4] = mul;
                this->v->workmic[noteno][octave+4].num = this->parser->setmicrotone.num;
                this->v->workmic[noteno][octave+4].denom = this->parser->setmicrotone.denom;
            } 
            else 
            { 
                for(j=0;j<10;j++) 
                { 
                    /* accidentals apply to all octaves */
                    this->v->workmap[noteno][j] = acc;
                    this->v->workmul[noteno][j] = mul;
                    this->v->workmic[noteno][j].num = this->parser->setmicrotone.num;
                    this->v->workmic[noteno][j].denom = this->parser->setmicrotone.denom;
                }
            }
        }
    }

    int p = (int) (strchr(sOneOctave, note) - sOneOctave);
    float pitchvalue;
    int pitch, bend;

    if(this->parser->temperament == TEMPERLN || 
       this->parser->temperament == TEMPEREQ)
    {
        pitchvalue = tscale[p]/100.0; /* cents to semitones */
        if(acc == '^') 
            pitchvalue = pitchvalue + mul*accidental_size;
        else
        if(acc == '_') 
            pitchvalue = pitchvalue - mul*accidental_size;
        pitchvalue = pitchvalue + octave*this->octave_size/100.0 + this->middle_c;

        /* Adjust for A=440.0 with zero pitchbend */
        pitchvalue += 9.f - (3.f*this->fifth_size - this->octave_size)/100.f;

        if(this->parser->microtone) 
        {
            if(this->parser->setmicrotone.denom == 100) 
            {
                /* microtone in cents */ 
                pitchvalue += this->parser->setmicrotone.num / 100.0;
            }
            else 
            if(this->parser->setmicrotone.denom == 0) 
            { 
                /* microstep_size is accidental_size for temperamentlinear,
                * or
                * microstep_size is the octave fraction for temperamentequal
                */
                pitchvalue += this->parser->setmicrotone.num * 
                                this->microstep_size/100.0;
            }
            else 
            {
                /* microtone relative to sharp step in the current temperament */
                pitchvalue += accidental_size * this->parser->setmicrotone.num /
                        static_cast<float>(this->parser->setmicrotone.denom);
            }
            /* needed? */
            this->parser->microtone = 0;
            this->parser->setmicrotone.num = this->parser->setmicrotone.denom = 0;
            this->active_pitchbend = 8192;
        }
    
        pitch =  (int) (pitchvalue + 0.5);
        bend = (int) (0.5 + 8192.0 + 4096.0 * (pitchvalue - (float) pitch));
        bend = bend<0?0:(bend>16383?16383:bend);
    }
    else 
    { 
        /* TEMPERNORMAL / TEMPERDT */
        p = sScale[p];
        if(acc == '^' && !this->parser->microtone) 
            p = p + mul;
        else
        if(acc == '_' && !this->parser->microtone) 
            p = p - mul;
        pitch = p + 12*octave + this->middle_c;
        bend = 8192; /* corresponds to zero bend */
        if(this->parser->temperament == TEMPERDT) 
        {
            bend += (int) (0.5 + 40.96 * temperament_dt[p]);
            bend = bend<0?0:(bend>16383?16383:bend);
        }
    }
    if(!this->parser->microtone) 
        *pitchbend = bend; /* don't override microtone */

    if(this->comma53) 
        this->convert_to_comma53(acc, &pitch, pitchbend); 
    this->parser->microtone = 0; 
    this->parser->setmicrotone.num = 0;
    this->parser->setmicrotone.denom = 0;
    return pitch; 
}

/* This code is used for handling gchords */
/* finds MIDI pitch value for note */
/* if propagate_accs is 1, apply any accidental to all instances of  */
/* that note in the bar. If propagate_accs is 0, accidental does not */
/* apply to other notes */
int 
AbcStore::pitchof(char note, char accidental, int mult, int octave, 
    int propagate_accs)
{
    int p = (int) (strchr(sOneOctave, note) - sOneOctave);
    p = sScale[p];
    char acc = accidental;
    int mul = mult;
    int noteno = (int)note - 'a';
    if(acc == ' ' && !this->parser->microtone)
    {
        /* if microtone, do not propagate accidentals to this note. */
        acc = v->workmap[noteno][octave+4];
        mul = v->workmul[noteno][octave+4];
    } 
    else 
    {
        if(propagate_accs)
        {
            v->workmap[noteno][octave+4] = acc;
            v->workmul[noteno][octave+4] = mul;
        }
    }
    if(acc == '^') 
        p = p + mul;
    else
    if(acc == '_') 
        p = p - mul;
    return p + 12*octave + this->middle_c;
}

void 
AbcStore::init_p48toc53() 
{
    int c = 0;
    for(int i=0; i< 48; i++) 
    {
        this->p48toc53[i] = c;
        /* if black note leave room for extra comma */
        if(i == 4 || i == 12 || i == 24 || i == 32 || i == 40)
            c = c+2;
        else
            c = c+1;
        //printf("%d  ",p48toc53[i]);
    }
}

/* The function converts *midipitch, *midibend to the
 * closest comma53 pitch values.
 */
void 
AbcStore::convert_to_comma53(char acc, int *midipitch, int* midibend) 
{
    float c53factor = 0.22641509f;
    float bendvalue = (float) ((*midibend - 8192.0)/4096.0);
    float eqtempmidi = (float) (*midipitch) + bendvalue;
    int octave = (int) (eqtempmidi / 12.0);
    int p48 = (int) (eqtempmidi * 4.0) % 48;
    int c53 = p48toc53[p48] + 53*octave;
    /* handle b4 or #5 (eg D4b4 or C4#5) in nameAE */
    if(p48 == 4 || p48 == 12 || p48 == 24 || p48 == 32 || p48 == 40)
    {
        if(bendvalue > 1.1 || bendvalue < -0.8) 
            c53++;
    }
    float c53midi = (float) c53 * c53factor;
    *midipitch = (int) c53midi;
    bendvalue = c53midi - (float) *midipitch;
    *midibend = 8192 + (int) (bendvalue * 4096);
}

/* assign lengths to grace notes. Call this just before  performing 
 * or converting.
 */
void 
AbcStore::dograce()
{
    int j = 0;
    this->gfact_method = 0; // new
    while(j < this->nextFeature) 
    {
        FeatureDesc &fd = this->featurelist[j];
        if(fd.feature == Abc::GRACEON)
            this->applygrace(j);
        else
        if(fd.feature == Abc::SETGRACE) 
        {
            this->gfact_method = fd.pitch;
            this->gfact_num =  fd.num;
            this->gfact_denom = fd.denom;
        }
        else
        if(fd.feature == Abc::LINENUM) 
            this->parser->lineno = fd.pitch;
        j = j + 1;
    };
    if (this->verbose >3) 
        this->log("dograce finished\n");
}

void
AbcStore::applygrace(int place)
{
    if(this->gfact_method) // default is 0
        this->applygrace_orig(place);
    else
        this->applygrace_new(place);
}

/* assign lengths to grace notes before generating MIDI */
/* This version adjusts the length of the grace notes
 * based on the length of the following note, the
 * number of the notes in the group of grace notes
 * and the desired fraction, gfact_num/gfact_denom.
 * This does not sound very natural.
 */
void 
AbcStore::applygrace_orig(int place)
{
    int j = place;
    int start = -1;
    while((j < this->nextFeature) && (start == -1)) 
    {
        FeatureDesc &fd = this->featurelist[j];
        if(fd.feature == Abc::GRACEON) 
            start = j; // no break
        else
        if(fd.feature == Abc::GRACEOFF) 
            this->error("} with no matching {");
        j = j + 1;
    }
    /* now find end of grace notes */
    int end = -1;
    while((j < this->nextFeature) && (end == -1)) 
    {
        FeatureDesc &fd = this->featurelist[j];
        if(fd.feature == Abc::GRACEOFF) 
            end = j; // no break;
        if(fd.feature == Abc::GRACEON && (j != start - 1)) 
            this->error("nested { not allowed"); // } match
        j = j + 1;
    }

    /* now find following note */
    int nextinchord = 0;
    int hostnotestart = -1;
    while((hostnotestart == -1) && (j < this->nextFeature)) 
    {
        FeatureDesc &fd = this->featurelist[j];
        if(fd.feature == Abc::SINGLE_BAR || 
           fd.feature == Abc::DOUBLE_BAR) 
        {
            this->cleargracenotes(start, end); /* [SS] 2021.01.24 */
            return;
        }      
        else
        if((fd.feature == Abc::NOTE) || (fd.feature == Abc::REST)) 
            hostnotestart = j; // no break
        else
        if(fd.feature == Abc::GRACEON) 
            this->error("Intervening note needed between grace notes");
        else
        if(fd.feature == Abc::CHORDON) 
            nextinchord = 1;
        j = j + 1;
    }
    int hostnoteend = -1;
    if(nextinchord) 
    {
        while((hostnoteend == -1) && (j < this->nextFeature)) 
        {
            FeatureDesc &fd = this->featurelist[j];
            if(fd.feature == Abc::CHORDOFF || 
               fd.feature == Abc::CHORDOFFEX) 
            {
                hostnoteend = j;
            }
            j = j + 1;
        }
    }
    else 
        hostnoteend = hostnotestart;

    if (hostnotestart == -1) 
        this->error("No note found to follow grace notes");
    else 
    {
        /* count up grace units */
        int next_num = 1, next_denom = 1; /* [SDG] 2020-06-03 */
        int fact_num, fact_denom;
        int grace_num = 0;
        int grace_denom = 1;
        int p = start;
        while (p <= end) 
        {
            FeatureDesc &fd = this->featurelist[p];
            if((fd.feature == Abc::NOTE) || 
               (fd.feature == Abc::REST)) 
            {
                grace_num = grace_num * fd.denom + grace_denom * fd.num;
                grace_denom = grace_denom * fd.denom;
                AbcMusic::reduceFraction(&grace_num, &grace_denom);
            }
            p = p + 1;
        }
        /* adjust host note or notes */
        p = hostnotestart;
        while (p <= hostnoteend) 
        {
            FeatureDesc &fd = this->featurelist[p];
            if((fd.feature == Abc::NOTE) || 
               (fd.feature == Abc::REST) || 
               (fd.feature == Abc::CHORDOFF) || 
               (fd.feature == Abc::CHORDOFFEX)) 
            {
                next_num = fd.num;
                next_denom = fd.denom;
                fd.num = next_num * (gfact_denom - gfact_num);
                fd.denom = next_denom * gfact_denom;
                AbcMusic::reduceFraction(&fd.num, &fd.denom);
            }
            p = p + 1;
        }
        fact_num = next_num * grace_denom * gfact_num;
        fact_denom = next_denom * grace_num * gfact_denom;
        AbcMusic::reduceFraction(&fact_num, &fact_denom);
        /* adjust length of grace notes */
        p = start;
        while (p <= end) 
        {
            this->lenmul(p, fact_num, fact_denom);
            p = p + 1;
        }
    }
}

/* assign lengths to grace notes before generating MIDI */
/* In this version each grace note has a predetermined
 * length, eg, (1/64 th note) and the total length of
 * the group of grace notes is stolen from the following
 * note. If the length of the following note is not
 * long enough to accommodate the group of grace notes,
 * then all the grace notes are given a length of 0.
 */
void 
AbcStore::applygrace_new(int place)
{
    int j = place;
    int start = -1;
    while((j < this->nextFeature) && (start == -1)) 
    {
        FeatureDesc &fd = this->featurelist[j];
        if(fd.feature == Abc::GRACEON) 
            start = j; // don't break here
        else
        if(fd.feature == Abc::GRACEOFF) 
            this->error("} with no matching {");
        j = j + 1;
    }

    /* now find end of grace notes */
    int end = -1;
    while((j < this->nextFeature) && (end == -1)) 
    {
        FeatureDesc &fd = this->featurelist[j];
        if(fd.feature == Abc::GRACEOFF) 
            end = j; // don't break here
        else
        if((fd.feature == Abc::GRACEON) && (j != start - 1)) 
            this->error("nested { not allowed"); // } match
        j = j + 1;
    }
    /* now find following note */
    int nextinchord = 0;
    int hostnotestart = -1;
    while((hostnotestart == -1) && (j < this->nextFeature)) 
    {
        FeatureDesc &fd = this->featurelist[j];
        if(fd.feature == Abc::SINGLE_BAR || 
           fd.feature == Abc::DOUBLE_BAR) 
        {
            this->cleargracenotes(start,end);
            return;
        } 
        else
        if((fd.feature == Abc::NOTE) || (fd.feature == Abc::REST)) 
            hostnotestart = j; // don't break
        else
        if(fd.feature == Abc::GRACEON) 
            this->error("Intervening note needed between grace notes");
        else
        if(fd.feature == Abc::CHORDON) 
            nextinchord = 1;
        j = j + 1;
    }
    int hostnoteend = -1;
    if(nextinchord) 
    {
        while((hostnoteend == -1) && (j < this->nextFeature)) 
        {
            FeatureDesc &fd = this->featurelist[j];
            if(fd.feature == Abc::CHORDOFF || 
               fd.feature == Abc::CHORDOFFEX) 
            {
                hostnoteend = j; // don't break
            }
            j = j + 1;
        }
    }
    else 
        hostnoteend = hostnotestart;

    if (hostnotestart == -1)
        this->error("No note found to follow grace notes");
    else 
    {
        /* count up grace units */
        int grace_num = 0;
        int grace_denom = 1;
        int p = start;
        int notesinchord = -1;
        while (p <= end) 
        {
            FeatureDesc &fd = this->featurelist[p];
            switch(fd.feature) 
            {
            case Abc::NOTE:
            case Abc::REST:
                if (notesinchord <= 0) 
                {
                    /* if chordal grace note, only count the first note */
                    grace_num = grace_num * fd.denom + grace_denom * fd.num;
                    grace_denom = grace_denom * fd.denom;
                    AbcMusic::reduceFraction(&grace_num, &grace_denom);
                    if (notesinchord == 0) 
                        notesinchord++;
                }
                break; 
                case Abc::CHORDON:
                    notesinchord = 0;
                break;
                case Abc::CHORDOFF:
                    notesinchord = -1;
                    break;
                default:
                    break;
            }
            p = p + 1;
        }

        /* new stuff starts here [SS] 2004/06/11 */
        /* is the following note long enough */
        FeatureDesc &fd = this->featurelist[p];
        p = hostnotestart;
        int adjusted_num = fd.num*grace_denom*gfact_denom - 
                           fd.denom*grace_num;
        int adjusted_den = fd.denom*grace_denom*gfact_denom;
        if(adjusted_num <= 0) /* not long enough [SS] 2014-10-09 */
        {
            p = start;
            this->removefeatures(start, end); 
            if(this->quiet == -1) 
                this->warning("Grace sequence exceeds host note. Grace sequence cut off.");
            return;
        }

        /* adjust host note or notes */
        p = hostnotestart;
        while (p <= hostnoteend) 
        {
            FeatureDesc &fd = this->featurelist[p];
            if(fd.feature == Abc::NOTE || 
               fd.feature == Abc::REST || 
               fd.feature == Abc::CHORDOFF || 
               fd.feature == Abc::CHORDOFFEX) 
            {
                fd.num = adjusted_num;
                fd.denom = adjusted_den;
                AbcMusic::reduceFraction(&fd.num, &fd.denom);
            }
            p = p + 1;
        }

        int fact_num = 1;
        int fact_denom = gfact_denom;
        AbcMusic::reduceFraction(&fact_num, &fact_denom);
        /* adjust length of grace notes */
        p = start;
        while (p <= end) 
        {
            this->lenmul(p, fact_num, fact_denom);
            p = p + 1;
        }
    }
}

void 
AbcStore::cleargracenotes(int start,int end) 
{
    this->removefeatures(start, end);
}

/* applies a roll to a note --------------------------------------  */
void 
AbcStore::doroll(char note, int octave, int n, int m, int pitch)
{
    int bend_up, bend_down;

    int upoct = octave;
    int downoct = octave;
    int t = (int) (strchr(sOneOctave, note) - sOneOctave);
    char up = *(sOneOctave + ((t+1) % 7));
    char down = *(sOneOctave + ((t+6) % 7));
    if(up == 'c') 
        upoct = upoct + 1;
    if(down == 'b') 
        downoct = downoct - 1;
    int pitchup = this->pitchof_bend(up, v->basemap[(int)up - 'a'], 1, upoct, 0,&bend_up);
    int pitchdown = this->pitchof_bend(down, v->basemap[(int)down - 'a'], 1, downoct, 0,&bend_down);

    FeatureDesc &fd = this->featurelist[this->nextFeature];
    fd.bentpitch = this->active_pitchbend;

    this->addfeature(Abc::NOTE, pitch, n*4, m*(v->default_length)*5);
    this->marknotestart();
    fd = this->featurelist[this->nextFeature];
    fd.bentpitch = bend_up;
    this->addfeature(Abc::NOTE, pitchup, n*4, m*(v->default_length)*5);
    fd = this->featurelist[this->nextFeature];
    fd.bentpitch = active_pitchbend;
    this->addfeature(Abc::NOTE, pitch, n*4, m*(v->default_length)*5);
    fd = this->featurelist[this->nextFeature];
    fd.bentpitch = bend_down;
    this->addfeature(Abc::NOTE, pitchdown, n*4, m*(v->default_length)*5);
    fd = this->featurelist[this->nextFeature];
    fd.bentpitch = active_pitchbend;
    this->addfeature(Abc::NOTE, pitch, n*4, m*(v->default_length)*5);
    this->marknoteend();
}

void 
AbcStore::doroll_setup(char note, int octave, int n, int m, int pitch)
{
    if(this->notesdefined < 0 || this->notesdefined>999)
    {
        char msg[80];
        snprintf(msg, 80, "illegal notesdefined = %d", this->notesdefined);
        this->error(msg);
    }
    else
    {
        int upoct = octave;
        int downoct = octave;
        int t = (int) (strchr(sOneOctave, note) - sOneOctave);
        char up = *(sOneOctave + ((t+1) % 7));
        char down = *(sOneOctave + ((t+6) % 7));
        if(up == 'c') 
            upoct = upoct + 1;
        if(down == 'b') 
            downoct = downoct - 1;
        int bend_up,bend_down;
        int pitchup = this->pitchof_bend(up, v->basemap[(int)up - 'a'], 1, 
                        upoct, 0, &bend_up);
        int pitchdown = this->pitchof_bend(down, v->basemap[(int)down - 'a'], 1, 
                        downoct, 0, &bend_down);

        notestruct *s = new notestruct();
        s->pitch = pitch;
        s->index = this->nextFeature;
        s->notetype = Abc::ROLL;
        s->pitchup = pitchup;
        s->pitchdown = pitchdown;
        s->bendup = bend_up;
        s->benddown = bend_down;
        s->default_length = global.default_length;
        this->noteaddr[this->notesdefined++] = s;
    }
}

void 
AbcStore::doroll_output(int featureIndex)
{
    FeatureDesc &fd = this->featurelist[featureIndex];
    int deco_index = fd.decotype;
    notestruct *s = this->noteaddr[deco_index];
    int pitch = s->pitch;
    int pitchdown = s->pitchdown;
    int pitchup = s->pitchup;
    this->active_pitchbend = fd.bentpitch;
    int bend_up = s->bendup;
    int bend_down = s->benddown;
    int default_length = s->default_length;
    int n = fd.num*default_length;
    int m = fd.denom*4;
    AbcMusic::reduceFraction(&n, &m);

    int a = n*4;
    int b = m*default_length*5;
    AbcMusic::reduceFraction(&a, &b);

    this->replacefeature(Abc::NOTE, pitch, a, b, featureIndex);

    featureIndex++;
    this->insertfeature(Abc::NOTE, pitchup, a, b, featureIndex);
    fd = this->featurelist[featureIndex];
    fd.bentpitch = bend_up;

    featureIndex++;
    this->insertfeature(Abc::NOTE, pitch, a, b, featureIndex);
    fd = this->featurelist[featureIndex];
    fd.bentpitch = this->active_pitchbend;

    featureIndex++;
    this->insertfeature(Abc::NOTE, pitchdown, a, b, featureIndex);
    fd = this->featurelist[featureIndex];
    fd.bentpitch = bend_down;

    featureIndex++;
    this->insertfeature(Abc::NOTE, pitch, a, b, featureIndex);
    fd = this->featurelist[featureIndex];
    fd.bentpitch = this->active_pitchbend;
}

void 
AbcStore::dotrill_setup(char note, int octave, int n, int m, int pitch)
{
    int upoct = octave;
    int t = (int) (strchr(sOneOctave, note) - sOneOctave);
    char up = *(sOneOctave + ((t+1) % 7));
    if(up == 'c') 
        upoct = upoct + 1;
    int bend;
    int pitchup = this->pitchof_bend(up, v->basemap[(int)up - 'a'], 1, 
                                upoct, 0, &bend);

    notestruct *s = new notestruct();
    s->pitch = pitch;
    s->index = this->nextFeature;
    s->notetype = Abc::TRILL;
    s->pitchup = pitchup;
    s->pitchdown = 0;
    s->default_length = global.default_length;
    s->bendup = bend;
    s->benddown = this->active_pitchbend;
    if(this->notesdefined < 0 || this->notesdefined>999)
    {
        char msg[64];
        snprintf(msg, 64, "illegal notesdefined = %d\n", this->notesdefined);
        this->error(msg);
    }
    else
    {
        this->noteaddr[this->notesdefined] = s;
        if(this->notesdefined < 1000) 
            this->notesdefined++;
    }
}

void 
AbcStore::dotrill_output(int featureIndex)
{
    FeatureDesc &fd = this->featurelist[featureIndex];
    int deco_index = fd.decotype;
    notestruct *s = this->noteaddr[deco_index];
    int pitch = s->pitch;
    int pitchdown = s->pitchdown;
    int pitchup = s->pitchup;
    int default_length = s->default_length;
    int bend = s->bendup;

    this->active_pitchbend = s->benddown;
    int n = fd.num * default_length;
    int m = fd.denom * 4;
    AbcMusic::reduceFraction(&n, &m);
    this->removefeature(featureIndex);

    int a = 4;
    int b = m*default_length;
    int count = n;
    while((this->current_tempo*a)/((long)b) > 100000L) 
    {
        count = count*2;
        b = b*2;
    }
    int j = 0;
    AbcMusic::reduceFraction(&a,&b);
    while(j < count) 
    {
        /*if(i == count - 1) {  **bug** [SS] 2006-09-10 */
        if(j%2 == 0) 
        {
            this->insertfeature(Abc::NOTE, pitchup, a, b, featureIndex);
            fd = this->featurelist[featureIndex];
            fd.bentpitch = bend;
            featureIndex++;
        } 
        else 
        {
            this->insertfeature(Abc::NOTE, pitch, a, b, featureIndex);
            fd = this->featurelist[featureIndex];
            fd.bentpitch = active_pitchbend;
            featureIndex++;
        }
        j = j + 1;
    }
}

/* applies a roll to a note */
void 
AbcStore::doornament(char note, int octave, int n, int m, int pitch)
{
    int upoct = octave;
    int downoct = octave;
    int t = (int) (strchr(sOneOctave, note)  - sOneOctave);
    char up = *(sOneOctave + ((t+1) % 7));
    char down = *(sOneOctave + ((t+6) % 7));
    if(up == 'c') 
        upoct = upoct + 1;
    if(down == 'b') 
        downoct = downoct - 1;

    int pitchup, pitchdown;
    int bend_up, bend_down;
    if(this->harpmode)
    {
        pitchup = pitch;
        pitchdown = pitch;
        bend_up = 8192;
        bend_down = 8192;
    }
    else
    {
        pitchup = this->pitchof_bend(up, v->basemap[(int)up - 'a'], 1, 
                        upoct, 0, &bend_up);
        pitchdown = this->pitchof_bend(down, v->basemap[(int)down - 'a'], 1, 
                        downoct, 0, &bend_down);
    }
    this->marknotestart();
    /* normalize notelength to L:1/8 */
    /*  nnorm =n*8; [SS] 2008-06-14 */
    int nnorm = n * (v->default_length);
    int mnorm = m * (v->default_length);
    AbcMusic::reduceFraction(&nnorm, &mnorm);
    if(nnorm == 3 && mnorm == 1) 
    {
        /* dotted quarter note treated differently */
        if(this->harpmode)  /* [JS] 2011-04-29 */
        {
            this->makeharproll3(pitch, active_pitchbend, n, m);
        }
        else
        {
            int nn = n/3; /* in case L:1/16 or smaller */
            if(nn < 1)
                nn=1;
            FeatureDesc &fd = this->featurelist[this->nextFeature];
            fd.bentpitch = active_pitchbend; /* [SS] 2006-11-3 */
            this->addfeature(Abc::NOTE, pitch, 4*nn,v->default_length);
            this->makecut(pitch, pitchup, active_pitchbend, bend_up, nn, m);
            this->makecut(pitch, pitchdown, active_pitchbend, bend_down, nn, m);
        }
    }
    else 
    {
        if(this->harpmode)
            this->makeharproll(pitch, active_pitchbend, n, m);
        else
            this->makecut(pitch,pitchup,active_pitchbend,bend_up,n,m);
    }
    this->marknoteend();
}

void 
AbcStore::makecut(int mainpitch, int shortpitch,int mainbend,int shortbend, 
    int n, int m)
{
    this->addfeature(Abc::GRACEON, 0, 0, 0);

    // for note
    FeatureDesc &fd = this->featurelist[this->nextFeature];
    fd.bentpitch = shortbend; 
    this->addfeature(Abc::NOTE, shortpitch, 4,v->default_length);

    this->addfeature(Abc::GRACEOFF, 0, 0, 0);

    // for note
    FeatureDesc &nfd = this->featurelist[this->nextFeature];
    nfd.bentpitch = mainbend;
    this->addfeature(Abc::NOTE, mainpitch, 4*n,m*v->default_length);
}

void 
AbcStore::makeharproll(int pitch, int bend, int n, int m)
{
    FeatureDesc &fd1 = this->featurelist[this->nextFeature];
    fd1.bentpitch = bend;
    this->addfeature(Abc::NOTE, pitch, 4*n/2, m*2*v->default_length);

    FeatureDesc &fd2 = this->featurelist[this->nextFeature];
    fd2.bentpitch = bend;
    this->addfeature(Abc::NOTE, pitch, 4*n/2, m*2*v->default_length);

    FeatureDesc &fd3 = this->featurelist[this->nextFeature];
    fd3.bentpitch = bend;
    this->addfeature(Abc::NOTE, pitch, 4*n/2, m*v->default_length);
}

void 
AbcStore::makeharproll3(int pitch, int bend, int n, int m)
{
    int a = n-1;
    FeatureDesc &fd1 = this->featurelist[this->nextFeature];
    fd1.bentpitch = bend;
    this->addfeature(Abc::NOTE, pitch, 4*(a)/2, m*2*v->default_length);

    FeatureDesc &fd2 = this->featurelist[this->nextFeature];
    fd2.bentpitch = bend;
    this->addfeature(Abc::NOTE, pitch, 4*(a)/2, m*2*v->default_length);

    FeatureDesc &fd3 = this->featurelist[this->nextFeature];
    fd3.bentpitch = bend;
    this->addfeature(Abc::NOTE, pitch, 4*(n/2+1), m*v->default_length);
}

/* read in accidental - used by event_handle_gchord() 
 * return updated place.
 */
char const *
AbcStore::get_accidental(
    char const *place, /* place in string being parsed */
    char *accidental/* pointer to char variable */
    )
{
    char const *p = place;
    *accidental = '=';
    if(*p == '#') 
    {
        *accidental = '^';
        p = p + 1;
    }
    if(*p == 'b')
    {
        *accidental = '_';
        p = p + 1;
    }
    return p;
}

/* The function attempts to clean up the missing left repeats |:
 * that occur in many multivoiced or multipart abc tunes.
 * The function scans the feature[] representation and
 * keeps track where the voice appears for the first time
 * in an individual part. If a :| appears in that voice,
 * but no |: was detected, then the function inserts a |:
 * at the point where the voice first appears.
 */
void 
AbcStore::scan_for_missing_repeats()
{
    int voicenum = 0;
    char part = '0';
    int num2add = 0; /* 2010-02-06 */
    int voicestart[64];
    int bar_rep_found[64];
    int add_leftrepeat_at[100];

    this->clear_voice_repeat_arrays(voicestart, bar_rep_found);

    /* set voicestart[0] in case there are no voices or parts */
    for(int i=0;i<this->nextFeature;i++) 
    {
        FeatureDesc &fd = this->featurelist[i];
        if (fd.feature == Abc::MUSICLINE) 
        {
            this->insertfeature(Abc::DOUBLE_BAR, 0, 0, 0, i+1);
            voicestart[0] = i+1; 
            break;
        }
    }

    for(int i=0;i<nextFeature;i++) 
    {
        FeatureDesc &fd = this->featurelist[i];
        if(fd.feature == Abc::PART && this->genMidi.parts != -1) 
        {
            clear_voice_repeat_arrays(voicestart, bar_rep_found);
            part = (char) fd.pitch;
            voicestart[0] = i;
        }
        else
        if(fd.feature == Abc::VOICE) 
        {
            voicenum = fd.pitch;
            if(!voicestart[voicenum]) 
            {
                voicestart[voicenum] = i;
                /*printf("voicestart[%d] = %d\n",voicenum,voicestart[voicenum]);*/
            }
        }
        else
        if(fd.feature == Abc::BAR_REP) 
            bar_rep_found[voicenum] = 1;
        else
        if ((fd.feature == Abc::REP_BAR || fd.feature == Abc::DOUBLE_REP) && 
            (!bar_rep_found[voicenum])) 
        {
            /* printf("missing BAR_REP for voice inserted for voice %d part %c\n",voicenum,part); [SS] 2011-04-19 */
            add_leftrepeat_at[num2add] = voicestart[voicenum]; 
            num2add++;
            bar_rep_found[voicenum] = 1;
        }
    }
    if(num2add > 0) 
        this->add_missing_repeats(num2add, add_leftrepeat_at); 
    if(this->verbose > 3) 
        this->log("scan_for_missing_repeats finished");
}

void 
AbcStore::clear_voice_repeat_arrays(int voicestart[64], int bar_rep_found[64]) 
{
    for(int i=0;i<64;i++) 
    {
        voicestart[i] = 0;
        bar_rep_found[i] = 0;
   }
}

void 
AbcStore::add_missing_repeats(int num2add, int add_leftrepeat_at[100])
{
    for(int i = num2add-1; i >= 0; i--) 
    {
        int leftrepeat = add_leftrepeat_at[i];
        int k = 0;
        if(this->voicesused) 
        {
            while(k < 20)
            {
                FeatureDesc &fd = this->featurelist[leftrepeat];
                if(fd.feature != Abc::VOICE)
                    leftrepeat++; 
                else
                    break;
                k++;
            }
        }
        this->insertfeature(Abc::BAR_REP, 0, 0, 0, leftrepeat+1); 
        for(int j=0;j<26;j++) 
        {
            if(this->genMidi.part_start[j] > add_leftrepeat_at[i])
                this->genMidi.part_start[j]++; // XXX?
        }
    }
}

/* connect up tied notes and cleans up the */
/* note lengths in the chords (eg [ace]3 ) */
void 
AbcStore::tiefix()
{
    int j = 0;
    int inchord = 0;
    int voiceno = 1;
    int chord_num = -1;
    int chord_denom=1;
    int chord_start, chord_end;
    while(j<this->nextFeature) 
    {
        FeatureDesc &fd = this->featurelist[j];
        switch(fd.feature)
        {
        case Abc::CHORDON:
            inchord = 1;
            chord_num = -1;
            chord_start = j;
            j = j + 1;
            break;
        case Abc::CHORDOFFEX:
            inchord = 0;
            chord_end=j;
            j = j + 1;
            break;
        case Abc::CHORDOFF:
            if(!((!inchord) || (chord_num == -1))) 
            {
                fd.num = chord_num; 
                fd.denom = chord_denom; 
            }
            inchord = 0;
            chord_end = j;
            j = j + 1;
            break;
        case Abc::NOTE:
            if(inchord && (chord_num == -1)) 
            {
                chord_num = fd.num;
                chord_denom = fd.denom;
                /* use note length in num,denom as chord length */
                /* if chord length is not set outside []        */
            }
            j = j + 1;
            break;
        case Abc::REST:
            if ((inchord) && (chord_num == -1)) 
            {
                chord_num = fd.num;
                chord_denom = fd.denom;
            }
            j = j + 1;
            break;
        case Abc::TIE:
            this->dotie(j, inchord, voiceno);
            j = j + 1;
            break;
        case Abc::LINENUM:
            this->parser->lineno = fd.pitch;
            j = j + 1;
            break;
        case Abc::VOICE:
            voiceno = fd.pitch;
            j = j + 1;
            break;
        default:
            j = j + 1;
            break;
        
        }
    }
    if(this->verbose >3) 
        this->log("tiefix finished");
}

/* called in preprocessing stage to handle ties */
/* we need the voiceno in case a tie is broken by a */
/* voice switch.                                    */
void 
AbcStore::dotie(int j, int xinchord, int voiceno)
{
    /* find note to be tied */
    int samechord = 0;
    int newbar =0;  /* if 1 it allows pitchline[] == pitchline[] test */
    if(xinchord) 
        samechord = 1;
    int tienote = j;
    int localvoiceno = voiceno;
    while(tienote > 0 && 
        (this->featurelist[tienote].feature != Abc::NOTE) &&
        (this->featurelist[tienote].feature != Abc::REST)) 
    {
        tienote = tienote - 1;
        if(this->featurelist[tienote].feature == Abc::VOICE) 
            break;
    }
    if(this->featurelist[tienote].feature != Abc::NOTE)
        this->error("Cannot find note before tie");
    else 
    {
        int inchord = xinchord;
        /* change NOTE + TIE to TNOTE + REST */
        this->featurelist[tienote].feature = Abc::TNOTE;
        this->featurelist[j].feature = Abc::REST;

        /* fix for tied microtone note */
        if(this->featurelist[tienote+1].feature == Abc::DYNAMIC) 
        {
            char command[40];
            strncpy(command, this->atext[tienote+1].c_str(), 40);
            if(strcmp(command, "pitchbend") == 0) 
            {
                /*printf("need to remove pitchbend following TNOTE\n"); */
                this->removefeature(tienote+1);
                j = j-1;
            }
        }

        /* fix for tied microtone note inside a slur */
        if(this->featurelist[tienote+1].feature == Abc::SLUR_TIE && 
           this->featurelist[tienote+2].feature == Abc::DYNAMIC) 
        {
            char command[40];
            strncpy(command, this->atext[tienote+2].c_str(), 40);
            if (strcmp(command, "pitchbend") == 0) 
            {
                /*printf("need to remove pitchbend following TNOTE in slur\n"); */
                this->removefeature(tienote+2);
                j = j - 1;
            }
        }

        this->featurelist[j].num = this->featurelist[tienote].num;
        this->featurelist[j].denom = this->featurelist[tienote].denom;

        int place = j;
        int tietodo = 1;
        int lasttie = j;
        int tied_num = this->featurelist[tienote].num;
        int tied_denom = this->featurelist[tienote].denom;
        int lastnote = -1;
        int done = 0;
        while((place < this->nextFeature) && (tied_num >=0) && (done == 0)) 
        {
            /*printf("%d %s   %d %d/%d ",place,featname[feature[place]],pitch[place],num[place],denom[place]); */
            switch(this->featurelist[place].feature) 
            {
            case Abc::SINGLE_BAR:
            case Abc::BAR_REP:
            case Abc::REP_BAR:
            case Abc::DOUBLE_BAR:
                if (tietodo) 
                    newbar = 1;
                else 
                    newbar=0;
                break; 
            case Abc::NOTE:
                if(localvoiceno != voiceno) 
                    break;
                lastnote = place;
                if ((tied_num == 0) && (tietodo == 0)) 
                    done = 1;
                
                if(((this->featurelist[place].pitchline == 
                     this->featurelist[tienote].pitchline && newbar) 
                    || (this->featurelist[place].pitch == 
                        this->featurelist[tienote].pitch)
                   ) && (tietodo == 1) && (samechord == 0)) 
                {
                    /* tie in note */
                    if(tied_num != 0) 
                        this->error("Time mismatch at tie");
                    tietodo = 0;
                    this->featurelist[place].pitch = 
                        this->featurelist[tienote].pitch; 
                    /* in case accidentals did not propagate                   */
                    /* add time to tied time */
                    AbcMusic::addFraction(&tied_num, &tied_denom, 
                        this->featurelist[place].num, 
                        this->featurelist[place].denom);
                    /* add time to tied note */
                    AbcMusic::addFraction(
                        &this->featurelist[tienote].num, 
                        &this->featurelist[tienote].denom, 
                        this->featurelist[place].num, 
                        this->featurelist[place].denom);
                    /* change note to a rest */
                    this->featurelist[place].feature = Abc::REST;
                    /* get rid of tie */
                    if(lasttie != j) 
                        this->featurelist[lasttie].feature = Abc::OLDTIE;
                }
                if(inchord == 0) 
                {
                    /* subtract time from tied time */
                    AbcMusic::addFraction(&tied_num, &tied_denom, 
                        -this->featurelist[place].num, 
                        this->featurelist[place].denom);
                }
                break;
            case Abc::REST:
                if(localvoiceno != voiceno) 
                    break;
                
                if((tied_num == 0) && (tietodo == 0))
                    done = 1;
                if(inchord == 0) 
                {
                    /* subtract time from tied time */
                    AbcMusic::addFraction(&tied_num, &tied_denom, 
                        -this->featurelist[place].num, 
                        this->featurelist[place].denom);
                }
                break;
            case Abc::TIE:
                if(localvoiceno != voiceno) 
                    break;
                if (lastnote == -1) 
                    this->error("Bad tie: possibly two ties in a row");
                else 
                {
                    if(this->featurelist[lastnote].pitch == 
                       this->featurelist[tienote].pitch && 
                       samechord == 0) 
                    {
                        lasttie = place;
                        tietodo = 1;
                        if (inchord) 
                            samechord = 1;
                    }
                }
                break;
            case Abc::CHORDON:
                if(localvoiceno != voiceno) 
                    break;
                inchord = 1;
                break;
            case Abc::CHORDOFF:
            case Abc::CHORDOFFEX:
                samechord = 0;
                if(localvoiceno != voiceno) 
                    break;
                inchord = 0;
                /* subtract time from tied time */
                AbcMusic::addFraction(&tied_num, &tied_denom, 
                    -this->featurelist[place].num, 
                    this->featurelist[place].denom);
                break;
            case Abc::VOICE:
                localvoiceno = this->featurelist[place].pitch;
                break;
            default:
                break;
            }
            /*printf("tied_num = %d done = %d inchord = %d\n",tied_num, done, inchord);*/
            place = place + 1;
        } // end while
        if (tietodo == 1 && !silent) 
            this->error("Could not find note to be tied");
    }
    /*if (verbose > 3) printf("dotie finished\n"); */
}


void
AbcStore::dumpfeat(int from, int to)
{
    char msg[80];
    for(int i=from;i<=to;i++)
    {
        FeatureDesc &fd = this->featurelist[i];
        int j = fd.feature;
        if(j<0 || j >= Abc::NUM_FEATURES) 
        {
            snprintf(msg, 80, "illegal feature[%d] = %d", i, j); 
            this->error(msg);
        }
        else 
        {
            snprintf(msg, 80, "%d %s   %d %d %d %d %d %d",
                i, Abc::featname(fd.feature),
                fd.pitch, fd.bentpitch, fd.decotype,  
                fd.num, fd.denom, fd.charloc);
        }
    }
}

/* find and correct missing repeats in music */
/* abc2midi places an extra || at the start of the music */
/* This can be converted to |: if necessary. */
void
AbcStore::fixreps()
{
    int expect_repeat = 0;   /* = 1 after |: or ::  otherwise 0*/
    int use_next = 0; /* if 1 set next bar line as ref_point */
    int nplays = 0; /* counts PLAY_ON_REP associated with a BAR_REP*/
    int rep_point = 0; /* where to assume a repeat starts */
    int j = 0;
    while(j < this->nextFeature) 
    {
        FeatureDesc &fd = this->featurelist[j];
        switch(fd.feature) 
        {
        case Abc::SINGLE_BAR:           /*  |  */
            if(use_next)
            {
                rep_point = j;
                use_next = 0;
            }
            break;
            case Abc::DOUBLE_BAR:           /*  || */
                rep_point = j;
                use_next = 0;
                break;
            case Abc::BAR_REP:              /* |:  */
                /* if |:  .. |: encountered. Report error and change second |: to :: */
                if(expect_repeat) 
                {
                    if(this->quiet == -1) 
                        this->error(" found another |: after a |:");
                    this->placeendrep(j);
                }
                expect_repeat = 1;
                use_next = 0;
                break;
            case Abc::REP_BAR:                /* :| */
                /* if :| encountered before |: place  "|:" at reference point */
                if((!expect_repeat) && (nplays < 1)) 
                {
                    this->placestartrep(rep_point);
                }
                expect_repeat = 0; 
                rep_point = j;
                use_next = 0;
                break;
            case Abc::PLAY_ON_REP: /*  [1 or [2 or [1,3 etc  */
                if(nplays == 0 && !expect_repeat) 
                {
                    if(this->quiet == -1) 
                        this->error(" found [1 or like before |:");
                    this->placestartrep(rep_point);
                }
                nplays++;
                break;
            case Abc::DOUBLE_REP:           /*  ::  */
                if(!expect_repeat) 
                {
                    if(this->quiet == -1) 
                        this->error(" found :: before |:");
                    this->placestartrep(rep_point);
                }
                expect_repeat = 1;
                break;
            default:
                break;
        }
        j = j + 1;
    }
}

/* patch up missing repeat */
void 
AbcStore::placestartrep(int j)
{
    if(this->quiet == -1) 
        this->warning("Assuming repeat");
    FeatureDesc &fd = this->featurelist[j];
    switch(fd.feature) 
    {
    case Abc::DOUBLE_BAR:
        fd.feature = Abc::BAR_REP;
        break;
    case Abc::SINGLE_BAR:
        fd.feature = Abc::BAR_REP;
        break;
    case Abc::REP_BAR:
        if(this->quiet == -1) 
            this->warning("replacing |: with double repeat (::)");
        fd.feature = Abc::DOUBLE_REP;
        break;
    case Abc::BAR_REP:
        if(this->quiet == -1) 
            this->error("Too many end repeats");
        break;
    case Abc::DOUBLE_REP:
        if(this->quiet == -1) 
            this->error("Too many end repeats");
        break;
    default:
        this->error("Internal error - please report");
        break;
    }
}

/* patch up missing repeat */
void 
AbcStore::placeendrep(int j)
{
    if(this->quiet == -1) 
        this->warning("Assuming repeat");

    FeatureDesc &fd = this->featurelist[j];
    switch(fd.feature) 
    {
    case Abc::DOUBLE_BAR:
        fd.feature = Abc::REP_BAR;
        break;
    case Abc::SINGLE_BAR:
        fd.feature = Abc::REP_BAR;
        break;
    case Abc::BAR_REP:
        fd.feature = Abc::DOUBLE_REP;
        this->warning("replacing with double repeat (::)");
        break;
    default:
        this->error("Internal error - please report");
        break;
    }
}


void 
AbcStore::expand_ornaments() 
{
    char msg[80];
    for(int i=0; i<this->nextFeature; i++) 
    {
        FeatureDesc &fd = this->featurelist[i];
        if(fd.decotype != 0)
        {
            if(fd.feature == Abc::TNOTE) 
                this->convert_tnote_to_note(i);
            else
            if(fd.feature == Abc::NOTE) 
            {
                int deco_index = fd.decotype;
                notestruct *s =  this->noteaddr[deco_index];
                int notetype = s->notetype;
                switch(notetype) 
                {
                case Abc::TRILL:
                    dotrill_output(i);
                    break;
                case Abc::ROLL:
                    doroll_output(i);
                    break;
                default: 
                    snprintf(msg, 80, "no such decoration %d/%d", 
                        fd.decotype, notetype);
                    this->error(msg);
                    break;
                }
            }
        }
    }
/*dump_notestruct();*/
    if(this->verbose> 3) 
        this->log("expand_ornaments finished");
}

/* tied notes TNOTE are handled in a different
 *  manner by genmidi so we need to change it
 *  to NOTE so it expand_ornament can process
 *  it. We change TNOTE to NOTE and eliminate the
 *  RESTS associated with TNOTE
 */
void 
AbcStore::convert_tnote_to_note(int loc)
{
    FeatureDesc &fd = this->featurelist[loc];
    fd.feature = Abc::NOTE;
    /* Look ahead and remove the REST associated with TNOTE    */
    /* The last REST in the TNOTE sequence has a nonzero pitch */
    int j = loc;
    for(int i=0;i<6;i++) 
    {
        fd = this->featurelist[j];
        if(fd.feature == Abc::REST) 
        {
            int pitchflag = fd.pitch;
            this->removefeature(j);
            if (pitchflag != 0) 
                break;
        } 
        else 
            j++; 
    }
}

/* The time signature placed before the next measure can
 * effect the current measure and result in warnings.
 */
void 
AbcStore::check_for_timesig_preceding_bar_line() 
{
    for(int i=4;i<this->nextFeature;i++) 
    {
        if(this->featurelist[i].feature == Abc::TIME && 
           this->featurelist[i+1].feature == Abc::SINGLE_BAR)
        {
            this->interchange_features(i, i+1);
        }
    }
}

/* update part_start array in case things have moved around */
void 
AbcStore::fix_part_start() 
{
    for(int i=0;i<this->nextFeature;i++) 
    {
        FeatureDesc &fd = this->featurelist[i];
        if(fd.feature == Abc::PART) 
        {
            /*printf("%d PART %d\n",i,pitch[i]); */
            int partnum = fd.pitch - 65;
            if (partnum >= 0 && partnum < 26) 
                this->genMidi.part_start[partnum] = i;
        }
    }
}

/* in event that the chord length was set outside [],
 * we must go back and adjust the note length of each note
 * inside the chord.
 */
void 
AbcStore::fix_enclosed_note_lengths(int from, int end) 
{
    FeatureDesc &fd = this->featurelist[end];
    if(this->apply_fermata_to_chord && !this->ignore_fermata)
    {
        if(this->fermata_fixed) 
            AbcMusic::addFraction(&fd.num, &fd.denom, 1, 1);
        else 
            fd.num *= 2;
    }
    int endnum = fd.num;
    int enddenom = fd.denom;
    for(int j = from; j < end; j++)
    {
        /*  also include REST which occurs in STACCATO CHORD */
        FeatureDesc &fd = this->featurelist[j];
        if(fd.feature == Abc::NOTE || 
           fd.feature == Abc::TNOTE || 
           fd.feature == Abc::REST) 
        {
            fd.num = endnum;
            fd.denom = enddenom; 
        }
    }
    this->apply_fermata_to_chord = 0;
}