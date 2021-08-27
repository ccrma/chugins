#include "AbcMidiTrackCtx.h"

#include "AbcGenMidi.h"
#include "AbcMusic.h"
#include "AbcParser.h"

/* beginWriting is used to convey initState to newly minted MidiTrackCtx.
 * we aren't yet ready to initialize fully because MidiWriter may be
 * null.
 */
void
AbcMidiTrackCtx::beginWriting(int xtrack, 
    Abc::InitState const *initState, IMidiWriter *mh)
{
    this->initState = initState;
    this->tracknumber = xtrack;
    this->trackvoice = xtrack; // XXX: a single "voice" can have multiple tracks

    this->midi = mh; // mh may be null and provided eg during getNextEvent/

    this->barchecking = initState->barchecking;

    this->tempo = initState->tempo;
    this->header_time_num = initState->time_num;
    this->header_time_denom = initState->time_denom;
    this->global_transpose = 0; // may be overridden by feature

    // etc... all fields are member vars 
    velocity_increment = 10;
    drone = {1, 0, 70, 45, 80, 33, 80}; /* bassoon a# */
    notecount = 0;
    notedelay = 10;
    chordattack = 0;
    staticnotedelay = 10;
    staticchordattack = 0;
    totalnotedelay = 0;
    syllcount = 0;

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

    this->featureIndexBegin = 
    this->featureIndexEnd = 
    this->featureIndexCurrent = -1;
}

void
AbcMidiTrackCtx::error(char const *msg)
{
    printf("error: %s\n", msg);
}

void
AbcMidiTrackCtx::warning(char const *msg)
{
    printf("warning: %s\n", msg);
}

void
AbcMidiTrackCtx::log(char const *msg)
{
    printf("info: %s\n", msg);
}

/* called at the start of each MIDI track. Sets up all 
 * necessary default and initial values 
 */
void
AbcMidiTrackCtx::initTrack(int xtrack, 
            int featureIndexBegin, int featureIndexEnd, 
            IMidiWriter *mw)
{
    char const *annotation;

    if(mw) this->midi = mw;
    
    this->tracknumber = xtrack;
    this->trackvoice = xtrack;
    this->featureIndexCurrent = featureIndexBegin;
    this->featureIndexBegin = featureIndexBegin;
    this->featureIndexEnd = featureIndexEnd;
    this->lineno = this->initState->lineno;

    this->timekey = 1;
    this->in_varend = 0;
    this->maxpass = 2;
    this->graceflag = 0;
    this->inchord = 0;
    this->tracklen = 0L;
    this->delta_time = 1L;
    this->delta_time_track0 = 0L;
    this->texton = 1;
    this->wordson = 0;
    this->noteson = 1;
    this->effecton = 0;
    this->gchordson = 0;
    this->temposon = 0;
    this->drumson = 0;
    this->droneon = 0;
    this->notedelay = staticnotedelay;
    this->chordattack = staticchordattack;
    this->trim_num = 0;
    this->trim_denom = 1;

    /* ensure that the percussion channel is not selected by findchannel() */
    this->genMidi->channel_in_use[9] = 1; 
    this->drumbars = 1;
    this->gchordbars = 1;

    this->bendtype = 1;
    this->single_velocity_inc = 0;
    this->single_velocity = -1;

    this->bendstate = 8192;
    for(int i=0;i<16; i++) 
        this->benddata[i] = 0;
    this->bendnvals = 0;
    for(int i=0;i<MAXLAYERS;i++) 
        this->controlnvals[i] = 0;
    
    // this case happens when we rewind-all but a client hasn't
    // expressed interest (via getNextPerformanceEvent) yet.
    if(!mw) 
        return;

    if(this->initState->karaoke)
    {
        if(xtrack == 0)                  
            this->karaokestarttrack(xtrack);
    }
    switch(this->genMidi->trackdescriptor[xtrack].tracktype)
    {
    case AbcGenMidi::NOTES:
        this->kspace = 0;
        this->noteson = 1;
        this->wordson = 0;
        annotation = "note track";
        this->midi->writeMetaEvent(0L, 
            MidiEvent::text_event, 
            annotation, strlen(annotation));
        this->trackvoice = this->genMidi->trackdescriptor[xtrack].voicenum;
        break;
    case AbcGenMidi::WORDS:
        this->kspace = 0;
        this->noteson = 0;
        this->wordson = 1;
        /*
        *  Turn text off for H:, A: and other fields.
        *  Putting it in Karaoke Words track (track 2) can throw off some Karaoke players.
        */   
        this->texton = 0;
        this->gchordson = 0;
        annotation = "lyric track";
        this->midi->writeMetaEvent(0L, 
            MidiEvent::text_event, 
            annotation, strlen(annotation));
        this->trackvoice = this->genMidi->trackdescriptor[xtrack].voicenum;
        break;
    case AbcGenMidi::NOTEWORDS:
        this->kspace = 0;
        this->noteson = 1;
        this->wordson = 1;
        annotation = "notes/lyric track"; /* [SS] 2015-06-22 */
        this->midi->writeMetaEvent(0L, 
            MidiEvent::text_event, 
            annotation, strlen(annotation));
        this->trackvoice = this->genMidi->trackdescriptor[xtrack].voicenum;
        break;
    case AbcGenMidi::GCHORDS:
        this->noteson = 0; 
        this->gchordson = 1;
        this->drumson = 0;
        this->droneon = 0;
        this->temposon = 0;
        annotation = "gchord track"; /* [SS] 2015-06-22 */
        this->midi->writeMetaEvent(0L, 
            MidiEvent::text_event, 
            annotation, 
            strlen(annotation));
        this->trackvoice = this->genMidi->trackdescriptor[xtrack].voicenum;
        /* be sure set_meter is called before setbeat even if we
         * have to call it more than once at the start of the track */
        this->set_meter(this->header_time_num, 
                             this->header_time_denom);
        /*    printf("calling setbeat for accompaniment track\n"); */
        this->setbeat();
        break;
    case AbcGenMidi::DRUMS: /* is this drum track ? */
        this->noteson = 0;
        this->gchordson = 0;
        this->drumson = 1;
        this->droneon =0;
        this->temposon = 0;
        annotation = "drum track"; /* [SS] 2015-06-22 */
        this->midi->writeMetaEvent(0L, 
            MidiEvent::text_event, 
            annotation, 
            strlen(annotation));
        this->trackvoice = this->genMidi->trackdescriptor[xtrack].voicenum;
        break;
    case AbcGenMidi::DRONE: /* is this drone track ? */
        this->noteson = 0;
        this->gchordson = 0;
        this->drumson = 0;
        this->droneon = 1;
        this->temposon = 0;
        annotation = "drone track"; /* [SS] 2015-06-22 */
        this->midi->writeMetaEvent(0L, 
            MidiEvent::text_event, 
            annotation, 
            strlen(annotation));
        this->trackvoice = this->genMidi->trackdescriptor[xtrack].voicenum;
        break;
    } // end switch tracktype
        
    this->nchordchannels = 0;
    if(xtrack == 0) 
    {
        this->midi->writeTempo(this->tempo);

        this->write_keysig(this->initState->keySharps, 
                                this->initState->keyMinor);
        this->write_meter(this->initState->time_num, 
                                this->initState->time_denom);
        this->gchordson = 0;
        this->temposon = 1;
        if(this->genMidi->ntracks > 1) 
        {
            /* type 1 files have no notes in first track */
            this->noteson = 0;
            this->texton = 0;
            this->trackvoice = 1;
            this->timekey = 0;
            /* return(0L); */
        }
    }

    if(this->initState->verbose) 
    {
        printf("trackvoice = %d track = %d", this->trackvoice, xtrack);
        if(this->noteson) printf("  noteson");
        if(this->wordson) printf("  wordson");
        if(this->gchordson) printf(" gchordson");
        if(this->drumson) printf(" drumson");
        if(this->droneon) printf(" droneon");
        if(this->temposon) printf(" temposon");
        printf("\n");
    }

    this->inchord = 0;

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
    this->gchord_index = 0;
    this->drum_ptr = 0;

    // was: startTrack
    AbcGenMidi::Track &currentTrack = this->genMidi->trackdescriptor[xtrack];
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
                    this->channel, xtrack);
                this->log(msg);
            }
        }
        else
            this->genMidi->channel_in_use[this->channel] = 1;
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
        /* here, we set pitch to -1 to signal special behavior from the queue
         */
        this->addtoQ(0, this->g_denom, -1, this->gchord_index, 0, 0);
        this->fun.base = 36;
        this->fun.vel = 80;
        this->gchord.base = 48;
        this->gchord.vel = 75;
        this->fun.chan = this->genMidi->findchannel();
        if(this->initState->verbose) 
        {
            snprintf(msg, 100, "assigning channel %d to bass voice\n", this->fun.chan);
            this->log(msg);
        }
        this->gchord.chan = this->genMidi->findchannel();
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
        /* here, we set pitch to -1 to signal special behavior from the queue
         */
        this->drum_ptr = 0;
        addtoQ(0, this->drum_denom, -1, this->drum_ptr,0, 0);
    }
    if(this->droneon) 
    {
        this->drone.event =0;
        this->drone.chan = this->genMidi->findchannel();
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

    this->resetBar();
    if(featureIndexBegin >= 0)
        this->saveRepeatState(featureIndexBegin);
    this->pass = 1;
    this->slurring = 0;
    this->was_slurring = 0;
    this->expect_repeat = 0;
}

void
AbcMidiTrackCtx::rewind()
{
    this->initTrack(this->tracknumber, 
        this->featureIndexBegin, this->featureIndexEnd,
        this->midi);
}

void
AbcMidiTrackCtx::set_meter(int n, int m)
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
AbcMidiTrackCtx::resetBar()
{
    this->barno = 0;
    this->bar_num = 0;
    this->bar_denom = 1;
    this->err_num = 0;
    this->err_denom = 1;
}

/* set up chord/fundamental sequence if not already set */
void
AbcMidiTrackCtx::setbeat()
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


/* wait for time a/b */
void
AbcMidiTrackCtx::delay(int a, int b, int c)
{
    int dt = (this->div_factor * a) / b + c;
    this->err_num = this->err_num * b + 
                    ((this->div_factor*a)%b)*this->err_denom;
    this->err_denom = this->err_denom * b;
    AbcMusic::reduceFraction(&this->err_num, &this->err_denom);
    dt += (this->err_num/this->err_denom);
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
AbcMidiTrackCtx::set_gchords(char const *s)
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
    this->gchord_index = 0;
    g_num = mtime_num * 4*gchordbars;
    g_denom = mtime_denom * seq_len;
    AbcMusic::reduceFraction(&g_num, &g_denom);
    /*  printf("%s  %d %d\n",s,g_num,g_denom); */
}

// adds a/b to count of units in bar
void
AbcMidiTrackCtx::addBarUnits(int a, int b)
{
    this->bar_num = this->bar_num * (b * this->b_denom) + 
                    this->bar_denom * (a * this->b_num) ;
    this->bar_denom = this->bar_denom * (b*this->b_denom);

    AbcMusic::reduceFraction(&this->bar_num, &this->bar_denom);
    /*printf("position = %d/%d\n",bar_num,bar_denom);*/
}

void
AbcMidiTrackCtx::zerobar()
{
    this->bar_num = 0;
    this->bar_denom = 1;
}

void
AbcMidiTrackCtx::getBarUnits(int *a, int *b)
{
    *a = this->bar_num;
    *b = this->bar_denom;
}

void
AbcMidiTrackCtx::set_drums(const char *s)
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
AbcMidiTrackCtx::write_keysig(int keySharps, int keyMinor)
{
    char data[2];
    data[0] = (char) (0xff & keySharps);
    data[1] = (char) keyMinor;
    this->midi->writeMetaEvent(0L, MidiEvent::key_signature, data, 2);
}

/* write meter to MIDI file --- */
void 
AbcMidiTrackCtx::write_meter(int n, int m)
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
    this->midi->writeMetaEvent(0L, MidiEvent::time_signature, data, 4);
}


/* header information for karaoke track based on w: fields */
void
AbcMidiTrackCtx::karaokestarttrack(int track)
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
        this->midi->writeMetaEvent(0L, MidiEvent::sequence_name, "Words", 5);
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

        Abc::FeatureDesc &fd = this->initState->featurelist[j];
        if(fd.feature == Abc::TITLE) 
        {
            if(track != 2)
            {
                this->midi->writeMetaEvent(0L, 
                    MidiEvent::sequence_name, 
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
AbcMidiTrackCtx::saveRepeatState(int voiceno)
{
    this->state[0] = voiceno;
    this->state[1] = this->barno;
    this->state[2] = this->div_factor;
    this->state[3] = this->transpose;
    this->state[4] = this->channel;
    this->state[5] = this->lineno;  // it's in featurelist
}

// &j, &barno, &div_factor, &transpose, &channel, &lineno
void
AbcMidiTrackCtx::restoreRepeatState(int *voiceno)
{
    *voiceno = this->state[0];
    this->barno = this->state[1];
    this->div_factor = this->state[2];
    this->transpose = this->state[3];
    this->channel = this->state[4];
    this->lineno = this->state[5];
}

/* generate accompaniment notes */
/* note no microtone or linear temperament support ! */
void 
AbcMidiTrackCtx::dogchords(int i)
{
    if(this->gchord_index >= (int) strlen(this->gchord_seq)) 
        this->gchord_index = 0;
    if(i != this->gchord_index) 
        return;
    int j;
    char action = this->gchord_seq[this->gchord_index];
    int len = gchord_len[gchord_index];
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
        printf("no such gchord code %d (%c)\n", action, action);
        return;
    }

    gchord_index++;
    this->addtoQ(g_num*len, g_denom, -1, gchord_index, 0, 0);
    if(gchord_index >= (int) strlen(gchord_seq)) 
        gchord_index = 0;
}

/* generate drum notes */
void 
AbcMidiTrackCtx::dodrums(int i)
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
AbcMidiTrackCtx::addtoQ(int num, int denom, int pitch, int chan, 
    int effect, int dur)
{
    this->queue.append(num, denom, pitch, chan, effect, dur,
            this->div_factor, this->notedelay);
}

/* output 'note on' queue up 'note off' for later */
void 
AbcMidiTrackCtx::save_note(int num, int denom, 
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
AbcMidiTrackCtx::progress_sequence(int chan)
{
    if(this->gchordson)
        this->dogchords(chan);
    if(drumson)
        this->dodrums(chan);
}

void 
AbcMidiTrackCtx::midi_noteoff(long delta_time, int pitch, int chan)
{
    this->genMidi->midi_noteoff(delta_time, pitch, chan);
}

void 
AbcMidiTrackCtx::midi_event(long delta_time, int evt, int chan, 
    char data[], int len)
{
    // usually delta_time is 0 (so note 'delayed')
    this->midi->writeMidiEvent(delta_time, evt, chan, data, len);
}

void 
AbcMidiTrackCtx::midi_event_with_delay(long delta_time, 
    int evt, int chan, char data[], int len)
{
    this->midi->writeMidiEvent(delta_time, evt, chan, data, len);
}

void 
AbcMidiTrackCtx::getEffectsState(long **delta_time, 
    int *bendstate, int *bendvelocity, int *bendacceleration)
{
   *delta_time = &this->delta_time; 
   *bendstate = this->bendstate;
   *bendvelocity = this->bendvelocity;
   *bendacceleration = this->bendacceleration;
}

void 
AbcMidiTrackCtx::getEffectsState(long **delta_time, 
    int *bendstate, int *bendnvals, int **benddata)
{
   *delta_time = &this->delta_time; 
   *bendstate = this->bendstate;
   *bendnvals = this->bendnvals;
   *benddata = this->benddata;
}

void 
AbcMidiTrackCtx::getEffectsState(long **delta_time, 
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