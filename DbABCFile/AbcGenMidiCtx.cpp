#include "AbcGenMidi.h"
#include "AbcMusic.h"
#include "AbcParser.h"

/* WriteContext -------------------------------------------*/

void
AbcGenMidi::WriteContext::beginWriting(FILE *fp, 
    InitState const *initState)
{
    this->fp = fp;
    this->initState = initState;
    this->barchecking = initState->barchecking;

    this->tempo = initState->tempo;
    this->header_time_num = initState->time_num;
    this->header_time_denom = initState->time_denom;
    velocity_increment = 10;
    drone = {1, 0, 70, 45, 80, 33, 80}; /* bassoon a# */
    notecount = 0;
    notedelay = 10;
    chordattack = 0;
    staticnotedelay = 10;
    staticchordattack = 0;
    totalnotedelay = 0;

    this->div_factor = this->division;

    trim = 1; /* to add a silent gap to note */
    trim_num = 1;
    trim_denom = 5;

    expand = 0; /* overlap note past next note */
    expand_num = 0;
    expand_denom = 5;

    gchord_error = 0;

    bendvelocity = 100;
    bendacceleration = 300;
    bendstate = 8192; /* also linked with queues.c */
    bendtype = 1;

    nlayers = 0;
    controlcombo = 0;

    onemorenote = 0;

    nchordchannels = 0;
}

void
AbcGenMidi::WriteContext::error(char const *msg)
{
    printf("error: %s\n", msg);
}

void
AbcGenMidi::WriteContext::warning(char const *msg)
{
    printf("warning: %s\n", msg);
}

void
AbcGenMidi::WriteContext::log(char const *msg)
{
    printf("info: %s\n", msg);
}

void
AbcGenMidi::WriteContext::initTrack(int xtrack)
{
    this->tracknumber = xtrack;
    tracklen = 0L;
    delta_time = 1L;
    delta_time_track0 = 0L;
    wordson = 0;
    noteson = 1;
    gchordson = 0;
    temposon = 0;
    drumson = 0;
    droneon = 0;
    notedelay = staticnotedelay;
    chordattack = staticchordattack;
    trim_num = 0;
    trim_denom = 1;
    /* ensure that the percussion channel is not selected by findchannel() */
    channel_in_use[9] = 1; 
    drumbars = 1;
    gchordbars = 1;

    bendtype = 1;
    single_velocity_inc = 0;
    single_velocity = -1;

    bendstate = 8192;
    for(int i=0;i<16; i++) 
        benddata[i] = 0;
    bendnvals = 0;
    for(int i=0;i<MAXLAYERS;i++) 
        controlnvals[i] = 0;
}

void
AbcGenMidi::WriteContext::set_meter(int n, int m)
{
    this->mtime_num = n;
    this->mtime_denom = m;
    this->time_num = n; 
    this->time_denom = m; 

    /* set up barsize */
    this->barsize = n;
    if (barsize % 3 == 0) 
    {
        this->beat = 3;
    } 
    else 
    {
        if (this->barsize % 2 == 0) 
            this->beat = 2;
        else 
            this->beat = barsize;
    }
    /* correction factor to make sure we count in the right units */
    if(m > 4) 
    {
        this->b_num = m/4;
        this->b_denom = 1;
    } 
    else 
    {
        this->b_num = 1;
        this->b_denom = 4/m;
    }
}

void
AbcGenMidi::WriteContext::resetBar()
{
    barno = 0;
    bar_num = 0;
    bar_denom = 1;
    err_num = 0;
    err_denom = 1;
}

/* set up chord/fundamental sequence if not already set */
void
AbcGenMidi::WriteContext::setbeat()
{
    if((time_num == 2) && (time_denom == 2))
        this->set_gchords("fzczfzcz");
    else
    if((time_num == 4) && (time_denom == 4)) 
        this->set_gchords("fzczfzcz");
    else
    if((time_num == 2) && (time_denom == 4)) 
        this->set_gchords("fzcz");
    else
    if((time_num == 6)  && (time_denom == 4)) 
        this->set_gchords("fzczfzczfzcz");
    else
    if((time_num == 3) && (time_denom == 8))
        this->set_gchords("fzczcz");
    else
    if((time_num == 5) && ((time_denom == 8) || (time_denom == 4)))
        this->set_gchords("fzbcz");
    else
    if((time_num == 7) && ((time_denom == 8) || (time_denom == 4)))
        this->set_gchords("fzbczcz");
    else
    if((time_num == 3) && (time_denom == 4))
        this->set_gchords("fzczcz");
    if((time_num == 6) && (time_denom == 8))
        this->set_gchords("fzcfzc");
    else
    if((time_num == 9) && (time_denom == 8))
        this->set_gchords("fzcfzcfzc");
    else
    if((time_num == 12) && (time_denom == 8))
        this->set_gchords("fzcfzcfzcfzc");
}

/* add a/b to the count of units in the bar */
void
AbcGenMidi::WriteContext::addunits(int a, int b)
{
    this->bar_num = (this->bar_num * b * this->b_denom) + 
                    (this->bar_num * a * this->b_num);
    this->bar_denom = this->bar_denom * b * this->b_denom;
    AbcMusic::reduceFraction(&bar_num, &bar_denom);
}

/* wait for time a/b */
void
AbcGenMidi::WriteContext::delay(int a, int b, int c)
{
    int dt = (this->div_factor * a) / b + c;
    this->err_num = this->err_num * b + 
                    ((this->div_factor*a)%b)*this->err_denom;
    this->err_denom = this->err_denom * b;
    AbcMusic::reduceFraction(&this->err_num, &this->err_denom);
    dt = dt + (this->err_num/this->err_denom);
    this->err_num = this->err_num%this->err_denom;
    this->queue.timestep(dt, 0, 
        this->delta_time, 
        this->delta_time_track0,
        this->totalnotedelay, 
        this->tracklen); 
}


/* set up a string which indicates how to generate accompaniment from */
/* guitar chords (i.e. "A", "G" in abc). */
/* called from dodeferred(), startfile() and setbeat() */
void
AbcGenMidi::WriteContext::set_gchords(char const *s)
{
    char const *p = s;
    int j = 0;
    int seq_len = 0;
    while((strchr("zcfbghijGHIJx", *p) != NULL) && (j <39)) 
    {
        if (*p == 0) 
            break;
        this->gchord_seq[j] = *p;
        p = p + 1;
        if((*p >= '0') && (*p <= '9'))
            this->gchord_len[j] = AbcParser::Readnump(&p);
        else 
            this->gchord_len[j] = 1;
        seq_len = seq_len + this->gchord_len[j];
        j = j + 1;
    }
    if(seq_len == 0) 
    {
        this->error("Bad gchord");
        gchord_seq[0] = 'z';
        gchord_len[0] = 1;
        seq_len = 1;
    }
    gchord_seq[j] = '\0';
    if (j == 39)
        this->error("Sequence string too long");

    /* work out unit delay in 1/4 notes*/
    g_num = mtime_num * 4*gchordbars;
    g_denom = mtime_denom * seq_len;
    AbcMusic::reduceFraction(&g_num, &g_denom);
    /*  printf("%s  %d %d\n",s,g_num,g_denom); */
}

// adds a/b to count of units in bar
void
AbcGenMidi::WriteContext::addBarUnits(int a, int b)
{
    this->bar_num = this->bar_num * (b * this->b_denom) + 
                    this->bar_denom * (a * this->b_num) ;
    this->bar_denom = this->bar_denom * (b*this->b_denom);

    AbcMusic::reduceFraction(&this->bar_num, &this->bar_denom);
    /*printf("position = %d/%d\n",bar_num,bar_denom);*/
}

void
AbcGenMidi::WriteContext::zerobar()
{
    this->bar_num = 0;
    this->bar_denom = 1;
}

void
AbcGenMidi::WriteContext::getBarUnits(int *a, int *b)
{
    *a = this->bar_num;
    *b = this->bar_denom;
}

void
AbcGenMidi::WriteContext::set_drums(const char *s)
{
    char const* p = s;
    int count = 0;
    int drum_hits = 0;
    int seq_len = 0;

    while (((*p == 'z') || (*p == 'd')) && (count<39)) 
    {
        if (*p == 'd') 
            drum_hits = drum_hits + 1;

        this->drum_seq[count] = *p;
        p = p + 1;
        if ((*p >= '0') && (*p <= '9')) 
            this->drum_len[count] = AbcParser::Readnump(&p);
        else 
            this->drum_len[count] = 1;
        seq_len = seq_len + this->drum_len[count];
        count = count + 1;
    }
    this->drum_seq[count] = '\0';
    if(seq_len == 0) 
    {
        this->error("Bad drum sequence");
        this->drum_seq[0] = 'z';
        this->drum_len[0] = 1;
        seq_len = 1;
    }
    if (count == 39) 
        this->error("Drum sequence string too long");

    /* look for program and velocity specifiers */
    for(int i = 0; i<count; i++) 
    {
        this->drum_program[i] = 35;
        this->drum_velocity[i] = 80;
    };
    AbcParser::Skipspace(&p);
    int i = 0;
    int place = 0;
    while(isdigit(*p)) 
    {
        int j = AbcParser::Readnump(&p);
        if (i < drum_hits) 
        {
            while(this->drum_seq[place] != 'd')
                place++;
            if (j > 127) 
                this->error("Drum program must be in the range 0-127");
            else 
                this->drum_program[place] = j;
            place++;
        } 
        else 
        {
            if (i < 2*count) 
            {
                if (i == drum_hits) 
                    place = 0;
                while(this->drum_seq[place] != 'd') 
                    place++;
                if ((j < 1) || (j > 127)) 
                    this->error("Drum velocity must be in the range 1-127");
                else 
                    this->drum_velocity[place] = j;
                place++;
            }
        }
        i = i + 1;
        AbcParser::Skipspace(&p);
    }
    if (i > 2*drum_hits) 
        this->error("Too many data items for drum sequence");

    /* work out unit delay in 1/4 notes*/
    this->drum_num = this->mtime_num * 4*this->drumbars;
    this->drum_denom = this->mtime_denom * seq_len;
    AbcMusic::reduceFraction(&this->drum_num, &this->drum_denom);
}

void 
AbcGenMidi::WriteContext::write_keysig(int keySharps, int keyMinor)
{
    char data[2];
    data[0] = (char) (0xff & keySharps);
    data[1] = (char) keyMinor;
    this->mfile.writeMetaEvent(0L, AbcMidiFile::key_signature, data, 2);
}

/* write meter to MIDI file --- */
void 
AbcGenMidi::WriteContext::write_meter(int n, int m)
{
    this->set_meter(n, m);
    int dd = 0;
    int t = m;
    while (t > 1) 
    {
        dd = dd + 1;
        t = t/2;
    }
    char data[4];
    data[0] = (char)n;
    data[1] = (char)dd;
    if (n%2 == 0) 
        data[2] = (char)(24*2*n/m);
    else
        data[2] = (char)(24*n/m);
    data[3] = 8;
    this->mfile.writeMetaEvent(0L, AbcMidiFile::time_signature, data, 4);
}

/* called at the start of each MIDI track. Sets up all 
 * necessary default and initial values 
 */
void
AbcGenMidi::WriteContext::starttrack(int tracknum)
{
    char msg[100];

    this->loudnote = 105;
    this->mednote = 95;
    this->softnote = 80;
    this->beatstring[0] = '\0';
    this->beataccents = 1;
    this->nbeats = 0;
    this->transpose = 0;

    /* make sure meter is reinitialized for every track
    * in case it was changed in the middle of the last track */
    this->set_meter(header_time_num, header_time_denom);
    this->div_factor = this->division;
    this->gchords = 1;
    this->partno = -1;
    this->partlabel = -1;
    this->g_started = 0;
    this->g_ptr = 0;
    this->drum_ptr = 0;

    Track &currentTrack = this->genMidi->trackdescriptor[tracknum];

    this->queue.init();

    if(this->noteson) 
    {
        this->channel = currentTrack.midichannel;
        if(this->channel == -1) 
        {
            this->channel = this->genMidi->findchannel();
            currentTrack.midichannel = this->channel;
            if(this->initState->verbose) 
            {
                snprintf(msg, 100, 
                    "assigning channel %d to track %d",
                    this->channel, tracknum);
                this->log(msg);
            }
            this->channel_in_use[this->channel] = 1;
        }
        else
            this->channel_in_use[this->channel] = 1;
        if(this->initState->retuning) 
            this->genMidi->midi_re_tune(this->channel);
    } 
    else 
    {
        /* set to valid value just in case - should never be used */
        this->channel = 0;
    }
    if(this->gchordson) 
    {
        this->addtoQ(0, this->g_denom, -1, this->g_ptr,0, 0);
        this->fun.base = 36;
        this->fun.vel = 80;
        this->gchord.base = 48;
        this->gchord.vel = 75;
        this->fun.chan = this->genMidi->findchannel();
        this->channel_in_use[fun.chan] = 1;
        if(this->initState->verbose) 
        {
            snprintf(msg, 100, "assigning channel %d to bass voice\n", this->fun.chan);
            this->log(msg);
        }
        this->gchord.chan = this->genMidi->findchannel();
        this->channel_in_use[this->gchord.chan] = 1;
        if(this->initState->verbose) 
        {
            snprintf(msg, 100, 
                "assigning channel %d to chordal accompaniment\n",
                gchord.chan);
            this->log(msg);
        }
        if (this->initState->retuning) 
        {
            this->genMidi->midi_re_tune(this->fun.chan);
            this->genMidi->midi_re_tune(this->gchord.chan);
        }
    }
    if(this->drumson) 
    { 
        this->drum_ptr = 0;
        addtoQ(0, this->drum_denom, -1, this->drum_ptr,0, 0);
    }
    if(this->droneon) 
    {
        this->drone.event =0;
        this->drone.chan = this->genMidi->findchannel();
        this->channel_in_use[this->drone.chan] = 1;
        if(this->initState->verbose) 
        {
            snprintf(msg, 100, 
                "assigning channel %d to drone", 
                this->drone.chan);
            this->log(msg);
        }
        if(this->initState->retuning) 
            this->genMidi->midi_re_tune(this->drone.chan);
    }

    this->g_next = 0;
    this->partrepno = 0;
    /*  thismline = -1; [SS] july 28 2006 */
    /* This disables the message 
    First lyrics line must come after first music line 
    When a new voice is started with an inline voice command
    eg [V:1] abcd| etc. Unfortunately this  is part of the 
    abc2-draft.html standard. See canzonetta.abc in
    abc.sourceforge.net/standard/abc2-draft.html 
    */
    this->thiswline = -1;
    this->nowordline = 0;
    this->waitforbar = 0;
    this->musicsyllables = 0;
    this->lyricsyllables = 0;
    for(int i=0; i<26; i++) 
        this->part_count[i] = 0;

    for(int i=0;i<MAXCHANS;i++) 
    {
        this->current_pitchbend[i] = 8192; /* neutral */
        this->current_program[i] = 0; /* acoustic piano */
    }
}

/* header information for karaoke track based on w: fields */
void
AbcGenMidi::WriteContext::karaokestarttrack(int track)
{
    /*
    *  Print Karaoke file headers in track 0.
    *  @KMIDI KARAOKE FILE - Karaoke midi file marker)
    */
    if(track == 0)
    {
        this->genMidi->text_data("@KMIDI KARAOKE FILE");
    }

    /*
    *  Name track 2 "Words" for the lyrics track.
    *  @LENGL - language
    *  Print @T information.
    *  1st @T line signifies title.
    *  2nd @T line signifies author.
    *  3rd @T line signifies copyright.
    */
    char atitle[200];
    if(track == 2)
    {
        this->mfile.writeMetaEvent(0L, AbcMidiFile::sequence_name, "Words", 5);
        this->kspace = 0;
        this->genMidi->text_data("@LENGL");
        strcpy(atitle, "@T");
    }
    else 
    {
        /*
        *  Write name of song as sequence name in track 0 and as track 1 name. 
        *  Print general information about the file using @I marker.
        *  Add to tracks 0 and 1 for various Karaoke (and midi) players to find.
        */
        strcpy(atitle, "@I");
    }

    int j = 0;
    int done = 3;
    std::vector<std::string> &atext = this->initState->atext;
    while((j < this->initState->nfeatures) && (done > 0))
    {
        j = j+1;

        FeatureDesc &fd = this->initState->featurelist[j];
        if(fd.feature == Abc::TITLE) 
        {
            if(track != 2)
            {
                this->mfile.writeMetaEvent(0L, 
                    AbcMidiFile::sequence_name, 
                    atext[fd.pitch].c_str(), 
                    atext[fd.pitch].size());
            }
            strcpy(atitle+2, atext[fd.pitch].c_str());
            this->genMidi->text_data(atitle);
            done--;
        }
        else
        if(fd.feature == Abc::COMPOSER) 
        {
            strcpy(atitle+2, atext[fd.pitch].c_str());
            this->genMidi->text_data(atitle);
            done--;
        }     
        else
        if(fd.feature == Abc::COPYRIGHT) 
        {
            strcpy(atitle+2, atext[fd.pitch].c_str());
            this->genMidi->text_data(atitle);
            done--;
        }
    }
}

// j, barno, div_factor, transpose, channel, lineno
void
AbcGenMidi::WriteContext::saveRepeatState(int voiceno, int state[6])
{
    state[0] = voiceno;
    state[1] = this->barno;
    state[2] = this->div_factor;
    state[3] = this->transpose;
    state[4] = this->channel;
    state[5] = this->lineno;  // it's in featurelist
}

// &j, &barno, &div_factor, &transpose, &channel, &lineno
void
AbcGenMidi::WriteContext::restoreRepeatState(int *voiceno, int state[6])
{
    *voiceno = state[0];
    this->barno = state[1];
    this->div_factor = state[2];
    this->transpose = state[3];
    this->channel = state[4];
    this->lineno = state[5];
}

/* generate accompaniment notes */
/* note no microtone or linear temperament support ! */
void 
AbcGenMidi::WriteContext::dogchords(int i)
{
    if (g_ptr >= (int) strlen(gchord_seq)) 
        g_ptr = 0;
    if(i != g_ptr) 
        return;
    int j;
    char action = gchord_seq[g_ptr];
    int len = gchord_len[g_ptr];
    if((chordnum == -1) && (action == 'c')) 
        action = 'f';

    if(gchords)
    switch(action) 
    {
    case 'z':
        break;
    case 'f':
        if(g_started && gchords) 
        {
            /* do fundamental */
            if(inversion == -1)
                this->save_note(g_num*len, g_denom, basepitch+fun.base, 8192, 
                    fun.chan, fun.vel);
            else
                this->save_note(g_num*len, g_denom, inversion+fun.base, 8192, 
                    fun.chan, fun.vel);
        }
        break;
    case 'b':
        if(g_started && gchords) 
        {
            /* do fundamental */
            if (inversion == -1)
                this->save_note(g_num*len, g_denom, basepitch+fun.base, 8192, fun.chan, fun.vel);
            else
                this->save_note(g_num*len, g_denom, inversion+fun.base, 8192, fun.chan, fun.vel);
        }
        break; // should we fall through?

    case 'c':
        /* do chord with handling of any 'inversion' note */
        if (g_started && gchords) 
        {
            for(j=0;j<gchordnotes_size;j++)
                this->save_note(g_num*len, g_denom, gchordnotes[j], 8192,
            gchord.chan, gchord.vel);
        }
        break;

    case 'g':
        if(gchordnotes_size>0 && g_started && gchords)
            this->save_note(g_num*len, g_denom, gchordnotes[0], 8192, gchord.chan, gchord.vel); 
        else
            this->save_note(g_num*len, g_denom, gchordnotes[gchordnotes_size], 8192, gchord.chan, gchord.vel); 
        break;

    case 'h':
        if(gchordnotes_size >1 && g_started && gchords)
            this->save_note(g_num*len, g_denom, gchordnotes[1], 8192, gchord.chan, gchord.vel); 
        else
            this->save_note(g_num*len, g_denom, gchordnotes[gchordnotes_size], 8192, gchord.chan, gchord.vel); 
        break;

    case 'i':
        if(gchordnotes_size >2 && g_started && gchords)
            this->save_note(g_num*len, g_denom, gchordnotes[2], 8192, gchord.chan, gchord.vel); 
        else /* [SS] 2016-01-03 */
            this->save_note(g_num*len, g_denom, gchordnotes[gchordnotes_size], 8192, gchord.chan, gchord.vel); 
        break;

    case 'j':
        if(gchordnotes_size >3 && g_started && gchords)
            this->save_note(g_num*len, g_denom, gchordnotes[3], 8192, gchord.chan, gchord.vel); 
        else /* [SS] 2016-01-03 */
            this->save_note(g_num*len, g_denom, gchordnotes[gchordnotes_size], 8192, gchord.chan, gchord.vel); 
        break;

    case 'G':
        if(gchordnotes_size>0 && g_started && gchords)
            this->save_note(g_num*len, g_denom, gchordnotes[0]-12, 8192, gchord.chan, gchord.vel); 
        else /* [SS] 2016-01-03 */
            this->save_note(g_num*len, g_denom, gchordnotes[gchordnotes_size], 8192, gchord.chan, gchord.vel); 
        break;

    case 'H':
        if(gchordnotes_size >1 && g_started && gchords)
            this->save_note(g_num*len, g_denom, gchordnotes[1]-12, 8192, gchord.chan, gchord.vel); 
        else /* [SS] 2016-01-03 */
            this->save_note(g_num*len, g_denom, gchordnotes[gchordnotes_size], 8192, gchord.chan, gchord.vel); 
        break;

    case 'I':
        if(gchordnotes_size >2 && g_started && gchords)
            this->save_note(g_num*len, g_denom, gchordnotes[2]-12, 8192, gchord.chan, gchord.vel); 
        else /* [SS] 2016-01-03 */
            this->save_note(g_num*len, g_denom, gchordnotes[gchordnotes_size], 8192, gchord.chan, gchord.vel); 
        break;

    case 'J':
        if(gchordnotes_size >3 && g_started && gchords)
            this->save_note(g_num*len, g_denom, gchordnotes[3]-12, 8192, gchord.chan, gchord.vel); 
        else /* [SS] 2016-01-03 */
            this->save_note(g_num*len, g_denom, gchordnotes[gchordnotes_size], 8192, gchord.chan, gchord.vel); 
        break;

    case 'x':
        if(!gchord_error) 
        {
            gchord_error++;
            this->warning("no default gchord string for this meter");
        }
        break;

    default:
        printf("no such gchord code %c\n",action);
    }

    g_ptr = g_ptr + 1; /* [SS] 2018-06-23 */
    this->addtoQ(g_num*len, g_denom, -1, g_ptr, 0, 0);
    if (g_ptr >= (int) 
        strlen(gchord_seq)) g_ptr = 0; /* [SS] 2018-06-23 */
}

/* generate drum notes */
void 
AbcGenMidi::WriteContext::dodrums(int i)
{
    if(this->drum_ptr >= (int) strlen(this->drum_seq))
        this->drum_ptr = 0;
    if(i == drum_ptr) 
    {  
        char action = this->drum_seq[this->drum_ptr];
        int len = this->drum_len[this->drum_ptr];
        switch(action) 
        {
        case 'z':
            break;
        case 'd':
            if(drum_on) 
            {
                this->save_note(
                    this->drum_num*len, 
                    this->drum_denom, 
                    this->drum_program[this->drum_ptr],
                    8192, 9,
                    this->drum_velocity[this->drum_ptr]);
            }
            break;
        }
        this->drum_ptr = this->drum_ptr + 1;
        this->addtoQ(this->drum_num*len, this->drum_denom, -1, 
                    this->drum_ptr, 0, 0);
        if(this->drum_ptr >= (int) strlen(drum_seq)) 
            this->drum_ptr = 0;
    }
}

void
AbcGenMidi::WriteContext::addtoQ(int num, int denom, int pitch, int chan, 
    int effect, int dur)
{
    this->queue.append(num, denom, pitch, chan, effect, dur,
            this->div_factor, this->notedelay);
}

/* output 'note on' queue up 'note off' for later */
void 
AbcGenMidi::WriteContext::save_note(int num, int denom, 
    int pitch, int pitchbend, int chan, int vel)
{
    if(chan == 9) 
    {
        /* don't transpose drum channel */
        this->genMidi->noteon_data(pitch, pitchbend, chan, vel);
        this->addtoQ(num, denom, pitch, chan, 0, -1);
    }
    else  
    {
        this->genMidi->noteon_data(pitch + transpose + global_transpose, 
            pitchbend, chan, vel);
        this->addtoQ(num, denom, pitch + transpose + global_transpose, 
            chan, 0, -1);
    }
}

/* AbcQueue client methods --------------------------------------------*/
void 
AbcGenMidi::WriteContext::progress_sequence(int chan)
{
    if(this->gchordson)
        this->dogchords(chan);
    if(drumson)
        this->dodrums(chan);
}

void 
AbcGenMidi::WriteContext::midi_noteoff(long delta_time, int pitch, int chan)
{
    this->genMidi->midi_noteoff(delta_time, pitch, chan);
}

void 
AbcGenMidi::WriteContext::midi_event(long delta_time, int evt, int chan, 
    char data[], int len)
{
    // usually delta_time is 0 (so note 'delayed')
    this->mfile.writeMidiEvent(delta_time, evt, chan, data, len);
}

void 
AbcGenMidi::WriteContext::midi_event_with_delay(long delta_time, 
    int evt, int chan, char data[], int len)
{
    this->mfile.writeMidiEvent(delta_time, evt, chan, data, len);
}

void 
AbcGenMidi::WriteContext::getEffectsState(long **delta_time, 
    int *bendstate, int *bendvelocity, int *bendacceleration)
{
   *delta_time = &this->delta_time; 
   *bendstate = this->bendstate;
   *bendvelocity = this->bendvelocity;
   *bendacceleration = this->bendacceleration;
}

void 
AbcGenMidi::WriteContext::getEffectsState(long **delta_time, 
    int *bendstate, int *bendnvals, int **benddata)
{
   *delta_time = &this->delta_time; 
   *bendstate = this->bendstate;
   *bendnvals = this->bendnvals;
   *benddata = this->benddata;
}

void 
AbcGenMidi::WriteContext::getEffectsState(long **delta_time, 
    int *bendstate, int *bendnvals, int **benddata,
    int *nlayers, int **controlnvals,  int **controldefaults,
    int **controldata)
{
   *delta_time = &this->delta_time; 
   *bendstate = this->bendstate;
   *bendnvals = this->bendnvals;
   *benddata = this->benddata;
   *nlayers = this->nlayers;
   *controlnvals = this->controlnvals;
   *controldefaults = this->controldefaults;
   *controldata = (int *) this->controldata;
}