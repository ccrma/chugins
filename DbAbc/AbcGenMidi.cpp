#include "AbcGenMidi.h"
#include "Abc.h"
#include "AbcMusic.h"
#include "AbcMidiFile.h"
#include "AbcParser.h"
#include "AbcStore.h"

#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <cassert>
#include <cstring>

/* ------------------------------------------------------------ */
AbcGenMidi::AbcGenMidi() :
    initState(nullptr),
    performing(false),
    midi(nullptr),
    wctx(nullptr),
    ntracks(0)
{
}

AbcGenMidi::~AbcGenMidi()
{}

void
AbcGenMidi::Init(bool forPerformance, char const *logPrefix)
{
    this->performing = forPerformance;
    this->ntracks = 0;
    this->logPrefix = logPrefix;
    this->barflymode = 0; // overriden via -BF or "R:"
    this->beatmodel = 0;
    this->stressmodel = 0;
    this->stress_pattern_loaded = 0;
    this->parts = -1;
    this->partspec.clear(); // string
    for(int j=0; j<26; j++)
        this->part_start[j] = -1;
    for(int j=0; j < sizeof(this->channel_in_use) / sizeof(int); j++) // yes MAXTRACKS, not MAXCHAN
        this->channel_in_use[j] = 0; // xxx: it's actually len:19
    this->trackPool.clear();
    this->trackPool.push_back(AbcMidiTrackCtx(this));
    this->wctx = &this->trackPool[0];
}

/* -------------------------------------------------------------------------- */
// beginPerform is called during AbcStore::finishfile when
// the argv includes "-perform".  In midi-out case we proceed
// to writefile(below) and perform standard .mid file conversion. 
// This call occurs *after* tracks are setup and since we wish to "perform" 
// multiple tracks simultaneously, we create and init a wctx for each track.
// 
int
AbcGenMidi::beginPerformance(Abc::InitState const *initState)
{
    this->initState = initState;
    this->assignVoiceBounds();
    this->midi = nullptr; // filled at getNextEvent
    this->trackPool.clear();
    this->trackPool.reserve(this->ntracks);
    for(int i=0;i<this->ntracks;i++)
    {
        this->trackPool.push_back(AbcMidiTrackCtx(this));
        this->trackPool[i].beginWriting(i, initState, nullptr); 
    }
    this->wctx = &this->trackPool[0];
    return 0;
}

int
AbcGenMidi::rewindPerformance()
{
    if(!this->midi) return -1;

    for(int i=0;i<this->ntracks;i++)
        this->trackPool[i].rewind();
    this->midi->writeTempo(this->initState->tempo);
    return 0;
}

int
AbcGenMidi::getNextPerformanceEvents(int track, IMidiWriter *m)
{
    int active;
    this->midi = m;
    assert(track < this->trackPool.size());
    this->wctx = &this->trackPool[track];

    // initTrack and processFeature may generate callbacks to IMidiWriter
    // it's up to caller to serialize those for output.
    if(this->wctx->featureIndexCurrent == -1)
    {
        // NB: initTrack 0 in multitrack settings causes notes, etc
        // to be disabled - it's a tempomap track.
        this->wctx->initTrack(track, 
            this->trackdescriptor[track].featureIndexBegin,
            this->trackdescriptor[track].featureIndexEnd,
            this->midi);
    }

    if( this->wctx->featureIndexBegin >= 0 &&
        this->wctx->featureIndexCurrent <= this->wctx->featureIndexEnd)
    {
        this->wctx->featureIndexCurrent = 
            this->processFeature(this->wctx->featureIndexCurrent, track);
        this->wctx->featureIndexCurrent++;
        active = 1;
    }
    else
    {
        active = 0;
        if(this->initState->verbose)
        {
            char msg[32];
            snprintf(msg, 32, "done processing track %d", this->wctx->tracknumber);
            this->wctx->log(msg);
        }
    }

    this->wctx = nullptr;
    return active;
}


void
AbcGenMidi::assignVoiceBounds()
{
    assert(this->initState);
    for(int i=0; i<this->ntracks; i++)
    {
        Track &track = this->trackdescriptor[i];
        track.featureIndexBegin = this->findvoice(0, track.voicenum, i);
        if(track.featureIndexBegin == this->initState->nfeatures)
        {
            // XXX: this can occur for, eg, guitar chords, drones, etc
            // idea: a track may have chords interspersed
            track.featureIndexBegin = 0;
            track.featureIndexEnd = this->initState->nfeatures - 1;
        }
        else
            track.featureIndexEnd = this->findvoiceEnd(track.featureIndexBegin, 
                                                        track.voicenum, i);
    }
}

/* -------------------------------------------------------------------------- */
int
AbcGenMidi::writefile(char const *fpath, Abc::InitState const *initState)
{
    assert(this->wctx && this->trackPool.size() == 1);
    FILE *fp = fopen(fpath, "wb");
    if(!fp) 
    {
        char msg[512];
        snprintf(msg, 512, "Problem opening %s", fpath);
        this->wctx->error(msg);
        return -1;
    }
    AbcMidiFile mfile;
    this->midi = &mfile;
    this->initState = initState;
    this->assignVoiceBounds();
    this->wctx->beginWriting(0, initState, this->midi);
    if(this->ntracks == 1) 
        mfile.write(this, fp, 0, 1, this->wctx->division);
    else 
        mfile.write(this, fp, 1, this->ntracks, this->wctx->division);
    fclose(fp);
    this->midi = nullptr;
    return 0;
}

/* This routine writes a MIDI track, it's invoked as a 
 * callback from AbcMidiFile.  It should always be the
 * case that wctx is non-null.
 */
long 
AbcGenMidi::writetrack(int xtrack)
{
    this->wctx->initTrack(xtrack);

    /* write notes */
    int j = 0;
    if((this->initState->voicesused) && (this->wctx->trackvoice != 1)) 
        j = this->findvoice(j, this->wctx->trackvoice, xtrack);

    this->wctx->saveRepeatState(j);

    while(j < this->initState->nfeatures) 
    {
        j = this->processFeature(j, xtrack);
        j = j + 1;
    }
    if((this->wctx->expect_repeat)&&(this->wctx->pass==1) && !this->initState->silent) 
    {
        this->wctx->error("Missing :| at end of tune");
    }
    this->wctx->queue.clear(this->wctx->delta_time, 
                            this->wctx->delta_time_track0,
                            this->wctx->totalnotedelay, 
                            this->wctx->tracklen);

    this->wctx->tracklen = this->wctx->tracklen + this->wctx->delta_time;
    if (xtrack == 1) 
    {
        this->wctx->tracklen1 = this->wctx->tracklen;
    } 
    else 
    {
        if ((xtrack != 0) && (this->wctx->tracklen != this->wctx->tracklen1)) 
        {
            char msg[100];
            float fbeats  = (float) this->wctx->tracklen/(float) 480.0;
            float fbeats1 = (float) this->wctx->tracklen1/(float) 480.0;
            snprintf(msg, 100, 
                "Track %d is %f quarter notes long not %f",
                xtrack, fbeats, fbeats1);
            this->wctx->warning(msg);
        }
    }
    return this->wctx->delta_time;
}

/* ------------------------------------------------------------------------- */
int
AbcGenMidi::processFeature(int j, int xtrack, MidiEvent *midiEvent)
{
    /* if (verbose >4) printf("%d %s\n",j,featname[feature[j]]);  */
    Abc::FeatureDesc &fd = this->initState->featurelist[j];
    if(this->initState->verbose > 4) 
    {
        printf("%d: %d %s %d %d/%d\n",  
            xtrack, 
            j, Abc::featname((Abc::FeatureType) fd.feature), 
            fd.pitch, fd.num, fd.denom);
    }
    this->wctx->lineposition = fd.charloc; 
    switch(fd.feature)
    {
    case Abc::NOTE:
        this->wctx->onemorenote = 0;
        if(this->wctx->wordson)
            this->write_syllable(j);
        if(this->wctx->nchordchannels > 0) 
            this->wctx->channel = this->wctx->chordchannels[0];
        if(this->wctx->noteson) 
        {
            if(this->wctx->inchord) 
            {
                this->wctx->notecount++;
                if(this->wctx->notecount > 1) 
                {
                    if(this->wctx->chordattack > 0) 
                    {
                        this->wctx->notedelay = (int) (
                            this->wctx->chordattack * 
                            rand()/(float) RAND_MAX);
                    }
                    this->wctx->delta_time += this->wctx->notedelay;
                    this->wctx->totalnotedelay += this->wctx->notedelay;
                    if(this->wctx->notecount <= this->wctx->nchordchannels+1)
                        this->wctx->channel = this->wctx->chordchannels[this->wctx->notecount-1];
                }
            }
            this->noteon(j, fd);
            /* set up note off */
            if(this->wctx->channel == 9) 
            {
                this->wctx->addtoQ(fd.num, fd.denom, 
                    this->wctx->drum_map[fd.pitch], 
                    this->wctx->channel, 0, 
                    -this->wctx->totalnotedelay -1);
            }
            else 
            {
                if((this->wctx->notecount > 1) && 
                   ((this->wctx->note_num * fd.denom) != 
                    (this->wctx->note_denom * fd.num)))
                {
                    char msg[100];
                    snprintf(msg, 100,
                        "unequal notes in chord %d/%d versus %d/%d",
                        this->wctx->note_num, this->wctx->note_denom, 
                        fd.num, fd.denom);
                    if(!this->initState->silent) 
                        this->wctx->warning(msg);
                    fd.num = this->wctx->note_num;
                    fd.denom = this->wctx->note_denom;
                }
                this->wctx->note_num = fd.num;
                this->wctx->note_denom = fd.denom;
                /* turn off slurring prematurely to separate two slurs in a row */
                if(this->wctx->slurring && 
                    this->initState->featurelist[j+2].feature == Abc::SLUR_OFF) 
                    this->wctx->slurring = 0; /* [SS] 2011-11-30 */
                if(this->wctx->trim && !this->wctx->slurring && !this->wctx->graceflag) 
                {
                    this->wctx->tnote_num = this->wctx->note_num;
                    this->wctx->tnote_denom = this->wctx->note_denom;
                    if(AbcMusic::gtFraction(this->wctx->note_num, this->wctx->note_denom, 
                        this->wctx->trim_num, this->wctx->trim_denom))
                    {
                        AbcMusic::addFraction(
                            &this->wctx->tnote_num, &this->wctx->tnote_denom,
                            -this->wctx->trim_num, this->wctx->trim_denom);
                    }
                    if(this->wctx->tnote_denom <= 0)
                    {
                        this->wctx->error("note length denominator is zero or less"); /* [SS] 2020-01-14 to prevent infinite loop on some systems */
                        return -1; // exit
                    }
                    this->wctx->addtoQ(
                        this->wctx->tnote_num, this->wctx->tnote_denom, 
                        fd.pitch +  this->wctx->transpose + this->wctx->global_transpose,
                        this->wctx->channel, this->wctx->effecton, 
                        -this->wctx->totalnotedelay -1);
                } 
                else 
                if(this->wctx->expand) 
                {
                    this->wctx->tnote_num = this->wctx->note_num;
                    this->wctx->tnote_denom = this->wctx->note_denom;
                    AbcMusic::addFraction(
                        &this->wctx->tnote_num, &this->wctx->tnote_denom,
                        this->wctx->expand_num, this->wctx->expand_denom);
                    this->wctx->addtoQ(
                        this->wctx->tnote_num, this->wctx->tnote_denom, 
                        fd.pitch + this->wctx->transpose + this->wctx->global_transpose,
                        this->wctx->channel, this->wctx->effecton, 
                        -this->wctx->totalnotedelay -1);
                }
                else
                {
                    this->wctx->addtoQ(
                        this->wctx->note_num, this->wctx->note_denom, 
                        fd.pitch + this->wctx->transpose + this->wctx->global_transpose,
                        this->wctx->channel, this->wctx->effecton, 
                        -this->wctx->totalnotedelay -1);
                }
            }
        }
        if(!this->wctx->inchord) 
        {
            this->wctx->delay(fd.num, fd.denom, 0);
            this->wctx->addBarUnits(fd.num, fd.denom);
            this->wctx->notecount = 0;
            this->wctx->totalnotedelay = 0;
        }
        this->wctx->effecton = 0;
        break;
    case Abc::TNOTE:
        this->wctx->onemorenote = 1;
        if(this->wctx->wordson) 
        {
            /* counts as 2 syllables : note + first tied note.
            * We ignore any bar line placed between tied notes
            * since this causes write_syllable to lose synchrony
            * with the notes.                                    */
            this->write_syllable(j);
            this->wctx->waitforbar = 0;
            this->write_syllable(j);
        }
        if(this->wctx->noteson) 
        {
            this->noteon(j, fd);
            /* set up note off */
            if(this->wctx->channel == 9) 
            {
                this->wctx->addtoQ(fd.num, fd.denom, 
                    this->wctx->drum_map[fd.pitch], 
                    this->wctx->channel, 0, 
                    -this->wctx->totalnotedelay -1);
            }
            else 
            {
                this->wctx->addtoQ(fd.num, fd.denom, 
                    fd.pitch + this->wctx->transpose + this->wctx->global_transpose, 
                    this->wctx->channel, this->wctx->effecton, 
                    -this->wctx->totalnotedelay -1);
            }
            this->wctx->effecton = 0;
        }
        break;
    case Abc::OLDTIE:
        if(this->wctx->wordson)
        {
            /* extra syllable beyond first two in a tie */
            this->write_syllable(j);
        }
        break;
    case Abc::REST:
        if(!this->wctx->inchord) 
        {
            this->wctx->delay(fd.num, fd.denom, 0);
            this->wctx->addBarUnits(fd.num, fd.denom);
        }
        break;
    case Abc::CHORDON:
        this->wctx->inchord = 1;
        break;
    case Abc::CHORDOFF:
    case Abc::CHORDOFFEX:
        if(this->wctx->wordson) 
            this->write_syllable(j);
        this->wctx->inchord = 0;
        this->wctx->delay(fd.num, fd.denom, 0);
        this->wctx->totalnotedelay = 0;
        this->wctx->notecount=0;
        this->wctx->notedelay = this->wctx->staticnotedelay;
        this->wctx->chordattack = this->wctx->staticchordattack;
        this->wctx->note_num = fd.num;
        this->wctx->note_denom = fd.denom;
        this->wctx->addBarUnits(this->wctx->note_num, this->wctx->note_denom);
        if(this->wctx->trim) 
        {
            if(AbcMusic::gtFraction(this->wctx->note_num, this->wctx->note_denom,
                            this->wctx->trim_num, this->wctx->trim_denom))
            {
                AbcMusic::addFraction(
                    &this->wctx->note_num,&this->wctx->note_denom,
                    -this->wctx->trim_num, this->wctx->trim_denom);
            }
        }
        break;
    case Abc::LINENUM:
        /* get correct line number for diagnostics */
        this->wctx->lineno = fd.pitch; // pitch is overloaded
        break;
    case Abc::MUSICLINE:
        if(this->wctx->wordson) 
        {
            this->wctx->thismline = j;
            this->wctx->nowordline = 0;
        }
        break;
    case Abc::MUSICSTOP:
        if(this->wctx->wordson)
            this->checksyllables();
        break;
    case Abc::PART:
        this->wctx->in_varend = 0;
        j = this->partbreak(xtrack, this->wctx->trackvoice, j);
        if(this->parts == -1) 
        {
            unsigned char msg[1];
            msg[0] = (unsigned char) fd.pitch;
            this->midi->writeMetaEvent(0L, MidiEvent::marker, msg, 1);
        }
        break;
    case Abc::VOICE:
        /* search on for next occurrence of voice */
        j = this->findvoice(j, this->wctx->trackvoice, xtrack);
        /* inline voice commands are not followed by MUSICLINE 
         * where we would normally get thismline 
         */
        if(this->wctx->wordson) 
        {
            this->wctx->thismline = j+1;
            this->wctx->nowordline = 0;
        }
        break;
    case Abc::TEXT:
        if(this->wctx->texton) 
        {
            std::string &s = this->initState->atext[fd.pitch];
            this->midi->writeMetaEvent(0L, 
                MidiEvent::text_event, (unsigned char *) s.c_str(), s.size());
        };
        break;
    case Abc::TITLE:
        /*  Write name of song as sequence name in track 0 and as track 1 name. */
        /*  karaokestarttrack routine handles this instead if tune is a Karaoke tune. */
        if(!this->initState->karaoke) 
        {
            if(xtrack < 2)
            {
                std::string &t = this->initState->atext[fd.pitch];
                this->midi->writeMetaEvent(0L, 
                    MidiEvent::sequence_name, 
                    (unsigned char *) t.c_str(), t.size());
            }
        }
        break;
    case Abc::SINGLE_BAR:
        this->wctx->waitforbar = 0;
        this->checkbar(this->wctx->pass);
        break;
    case Abc::DOUBLE_BAR:  /* || */
        this->wctx->in_varend = 0;
        this->wctx->waitforbar = 0;
        this->softcheckbar(this->wctx->pass);
        break;
    case Abc::BAR_REP: /* |: */
        /* ensures that two |: don't occur in a row                */
        /* saves position of where to return when :| is encountered */
        this->wctx->in_varend = 0;
        this->wctx->waitforbar = 0;
        this->softcheckbar(this->wctx->pass);
        if((this->wctx->pass==1)&&(this->wctx->expect_repeat)) 
        {
            this->wctx->error("Expected end repeat not found at |:");
        }
        this->wctx->saveRepeatState(j);
        this->wctx->expect_repeat = 1;
        this->wctx->pass = 1;
        this->wctx->maxpass = 2;
        break;
    case Abc::REP_BAR:  /* :|  */
        /* ensures it was preceded by |: so we know where to return */
        /* returns index j to the place following |:                */ 
        this->wctx->in_varend = 0;
        this->wctx->waitforbar = 0;
        this->softcheckbar(this->wctx->pass);
        if(this->wctx->pass == 1) 
        {
            if(!this->wctx->expect_repeat)
            {
                this->wctx->error("Found unexpected :|");
            } 
            else 
            {
                /*  pass = 2;  [SS] 2004-10-14 */
                this->wctx->pass++;   /* we may have multiple repeats */
                this->wctx->restoreRepeatState(&j);
                this->wctx->slurring = 0;
                this->wctx->was_slurring = 0;
                this->wctx->expect_repeat = 0;
            }
        } 
        else 
        {
            /* we could have multi repeats.                        */
            /* pass = 1;          [SS] 2004-10-14                  */
            /* we could have accidentally have                       */
            /*   |: .sect 1..  :| ...sect 2 :|.  We  don't want to */
            /* go back to sect 1 when we encounter :| in sect 2.   */
            /* We signal that we will expect |: but we wont't check */
            if(this->wctx->pass < this->wctx->maxpass)
            {
                this->wctx->expect_repeat = 0;
                this->wctx->pass++;   /* we may have multiple repeats */
                this->wctx->restoreRepeatState(&j);
                this->wctx->slurring = 0;
                this->wctx->was_slurring = 0;
            }
        }
        break;
    case Abc::PLAY_ON_REP: /* |[1 or |[2 or |1 or |2 */
        /* keeps count of the pass number and selects the appropriate   */
        /* to be played for each pass. This code was designed to handle */ 
        /* multirepeats using the inlist() function however the pass    */
        /* variable is not set up correctly for multirepeats.           */
        {
            int passnum;
            if(this->wctx->in_varend != 0) 
                this->wctx->error("Need || |: :| or ::  to mark end of variant ending");
            passnum = -1;
            if(((this->wctx->expect_repeat) || (this->wctx->pass>1))) 
                passnum = this->wctx->pass;

            if(passnum == -1) 
            {
                this->wctx->error("multiple endings do not follow |: or ::");
                passnum = 1;
            }

            if(this->inlist(j, passnum) != 1) 
            {
                j = j + 1;
                /* if this is not the variant ending to be played on this pass*/
                /* then skip to the end of this section watching out for voice*/
                /* changes. Usually a section end with a :|, but the last     */
                /* last section could end with almost anything including a    */
                /* PART change.                                               */
                if(this->initState->featurelist[j].feature == Abc::VOICE) 
                    j = this->findvoice(j, this->wctx->trackvoice, xtrack);
                while(j<this->initState->nfeatures && 
                    (this->initState->featurelist[j].feature != Abc::REP_BAR) && 
                    (this->initState->featurelist[j].feature != Abc::BAR_REP) &&
                    (this->initState->featurelist[j].feature != Abc::PART) &&
                    (this->initState->featurelist[j].feature != Abc::DOUBLE_BAR) &&
                    (this->initState->featurelist[j].feature != Abc::THICK_THIN) &&
                    (this->initState->featurelist[j].feature != Abc::THIN_THICK) &&
                    (this->initState->featurelist[j].feature != Abc::PLAY_ON_REP)) 
                {
                    j = j + 1;
                    if(this->initState->featurelist[j].feature == Abc::VOICE) 
                        j = this->findvoice(j, this->wctx->trackvoice, xtrack);
                }
                this->wctx->barno = this->wctx->barno + 1;
                if(j == this->initState->nfeatures /* || (feature[j] == PLAY_ON_REP) */) 
                { 
                    /* end of tune was encountered before finding end of */
                    /* variant ending.  */
                    this->wctx->error(
                        "Cannot find :| || [| or |] to close variant ending");
                } 
                else 
                {
                    if(this->initState->featurelist[j].feature == Abc::PART)
                        j = j - 1; 
                }
            }
            else 
            {
                this->wctx->in_varend = 1;   /* segment matches pass number, we play it */
                /*printf("playing at %d for pass %d\n",j,passnum); */
                if(this->wctx->maxpass < 4) 
                    this->wctx->maxpass = this->wctx->pass+1;
            }
        }
        break;
    case Abc::DOUBLE_REP:     /*  ::  */
        this->wctx->in_varend = 0;
        this->wctx->waitforbar = 0;
        this->softcheckbar(this->wctx->pass);
        if(this->wctx->pass > 1) 
        {
            /* Already gone through last time. Process it as a |:*/
            /* and continue on.                                  */
            this->wctx->expect_repeat = 1;
            this->wctx->saveRepeatState(j);
            this->wctx->pass = 1;
            this->wctx->maxpass = 2;
        }
        else
        {
            /* should do a repeat unless |: is missing.       */
            if (!this->wctx->expect_repeat)
            {
                /* missing |: don't repeat but set up for next repeat */
                /* section.                                           */
                this->wctx->error("Found unexpected ::");
                this->wctx->expect_repeat = 1;
                this->wctx->saveRepeatState(j);
                this->wctx->pass = 1;
            } 
            else 
            {
                /* go back and do the repeat */
                this->wctx->restoreRepeatState(&j);
                this->wctx->slurring = 0;
                this->wctx->was_slurring = 0;
                /*pass = 2;  [SS] 2004-10-14*/
                this->wctx->pass++;
            }
        }
        break;
    case Abc::GCHORD:
        this->wctx->basepitch = fd.pitch;
        this->wctx->inversion = fd.num;
        this->wctx->chordnum = fd.denom;
        this->wctx->g_started = 1;
        this->configure_gchord();
        break;
    case Abc::GCHORDON:
        if(this->wctx->gchordson)
            this->wctx->gchords = 1;
        break;
    case Abc::GCHORDOFF:
        this->wctx->gchords = 0;
        break;
    case Abc::DRUMON:
        if(this->wctx->drumson)
            this->wctx->drum_on = 1;
        break;
    case Abc::DRUMOFF:
        this->wctx->drum_on = 0;
        break;
    case Abc::DRONEON:
        if(this->wctx->droneon) 
            this->start_drone();
        break;
    case Abc::DRONEOFF:
        if(this->wctx->droneon) 
            this->stop_drone();
        break;
    case Abc::ARPEGGIO:
        this->wctx->notedelay = 3*this->wctx->staticnotedelay;
        this->wctx->chordattack= 3*this->wctx->staticchordattack;
        break;
    case Abc::GRACEON:
        this->wctx->graceflag = 1;
        break;
    case Abc::GRACEOFF:
        this->wctx->graceflag = 0;
        break;
    case Abc::DYNAMIC:
        this->dodeferred(this->initState->atext[fd.pitch].c_str(), 
                        this->wctx->noteson);
        break;
    case Abc::KEY:
        if(this->wctx->timekey) 
            this->wctx->write_keysig(fd.pitch, fd.denom);
        break;
    case Abc::TIME:
        if(this->wctx->timekey) 
        {
            this->wctx->barchecking = fd.pitch;
            this->wctx->write_meter(fd.num, fd.denom);
            this->wctx->setbeat(); 
        }
        break;
    case Abc::TEMPO: // for intra-score tempo changes only...
        #if 0
        /* tempo-changes can't generally be applied instantly, they
         * have delays
         */
        if(this->performing && this->wctx->delta_time == 0)
        {
            this->midi->writeTempo(fd.pitch); // contains "new_tempo"
        }
        #endif
        if(this->wctx->temposon || this->performing)
        {
            unsigned char data[3];
/*
            long newtempo;

            newtempo = ((long)num[j]<<16) | ((long)denom[j] & 0xffff);
            printf("New tempo = %ld [%x %x]\n", newtempo, num[j], denom[j]);
*/
            // The MIDI set tempo meta message sets the tempo of a MIDI 
            // sequence in terms of microseconds per quarter note.
            data[0] = fd.num & 0xff;
            data[1] = (fd.denom>>8) & 0xff;
            data[2] = fd.denom & 0xff;
            /* delta_time_track0 */
            if(ntracks != 1) 
            { 
                this->midi->writeMetaEvent(this->wctx->delta_time_track0, 
                    MidiEvent::set_tempo, data, 3);
                this->wctx->tracklen += this->wctx->delta_time_track0;
                this->wctx->delta_time_track0 = 0L;
            } 
            else 
            {
                this->midi->writeMetaEvent(
                    this->wctx->delta_time, 
                    MidiEvent::set_tempo, data, 3);
                this->wctx->delta_time = 0L;
            }
            /*
                if (j > 0) {
                    div_factor = pitch[j];
                };
            */
        }
        break;
    case Abc::CHANNEL:
        this->wctx->channel = fd.pitch;
        break;
    case Abc::TRANSPOSE:
        this->wctx->transpose = fd.pitch;
        break;
    case Abc::GTRANSPOSE:
        this->wctx->global_transpose = fd.pitch;
        break;
    case Abc::RTRANSPOSE:
        this->wctx->global_transpose +=  fd.pitch;
        break;
    case Abc::SLUR_ON:
        /*
        if (slurring) {
            event_error("Unexpected start of slur found");
        }; [SS] 2014-04-24
        */
        this->wctx->slurring = 1;
        this->wctx->was_slurring = 1; /* [SS] 2011-11-30 */
        break;
    case Abc::SLUR_OFF:
        /*
        if (!slurring && !was_slurring) { 
            event_error("Unexpected end of slur found");
        };
        [SS] 2014-04-24 */
        this->wctx->slurring = 0;
        this->wctx->was_slurring = 0;
        break;
    case Abc::COPYRIGHT:
        if(xtrack == 0)
        {
            std::string &str = this->initState->atext[fd.pitch];
            this->midi->writeMetaEvent(
                this->wctx->delta_time, MidiEvent::copyright_notice, 
                (unsigned char *) str.c_str(), str.size());
        }
        break;
    case Abc::SETTRIM:
        this->wctx->trim_num = fd.num;
        this->wctx->trim_denom = fd.denom;
        if(this->wctx->trim_num > 0) 
            this->wctx->trim = 1;
        else 
            this->wctx->trim = 0;
        break;
    case Abc::EXPAND:
        this->wctx->expand_num = fd.num;
        this->wctx->expand_denom = fd.denom;
        if(this->wctx->expand_num > 0) 
        {
            this->wctx->trim = 0;
            this->wctx->expand = 1;
        }
        else 
            this->wctx->expand = 0;
        break;
    case Abc::META:
        if(fd.pitch == 0 && this->wctx->noteson==1)
        {
            /*printf("linenum = %d charpos = %d\n",num[j],denom[j]);*/
            // easyabc_interface(j);
        }
        break;
    case Abc::PEDAL_ON:
        this->pedal_on();
        break;
    case Abc::PEDAL_OFF:
        this->pedal_off();
        break;
    case Abc::EFFECT: 
        if(fd.pitch == 1) // !bend!
            this->wctx->effecton = this->wctx->bendtype;
        else // fd.pitch == 2 (!shape!)
            this->wctx->effecton = 10;
        break;
    default:
        break;
    }
    return j; // if jout == jin we didn't jump anywhere (cf voice or pattern)
}

void
AbcGenMidi::midierror(const char *msg)
{
    this->wctx->error(msg);
}

/* Write out a syllable. This routine must check that it has a line of 
 * lyrics and find one if it doesn't have one. The function is called
 * for each note encountered in feature[j] when the global variable
 * wordson is set. The function keeps count of the number of notes
 * in the music and words in the lyrics so that we can check that 
 * they match at the end of a music line. When waitforbar is set
 * by getword, the function  does nothing (allows feature[j]
 * to advance to next feature) until waitforbar is set to 0
 * (by writetrack).                                                 */
void
AbcGenMidi::write_syllable(int place)
{
    this->wctx->musicsyllables++;
    if(this->wctx->waitforbar) 
    {
        this->wctx->lyricsyllables++;
        return;
    }
    if((!this->wctx->nowordline) && (!this->wctx->waitforbar)) 
    {
        if(this->wctx->thiswline == -1) 
            this->wctx->thiswline = this->findwline(this->wctx->thismline);
        if(!this->wctx->nowordline) 
        {
            int done = 0;
            while(!done) 
            {
                if(this->wctx->syllcount == 0) 
                {
                    /* try to get fresh word */
                    this->wctx->syllcount = this->getword(&this->wctx->wordlineplace, 
                                                        this->wctx->windex);
                    if(this->wctx->waitforbar) 
                    {
                        done = 1;
                        if(this->wctx->syllcount == 0) 
                            this->wctx->lyricsyllables++;
                    } 
                    else 
                    {
                        if(this->wctx->syllcount == 0) 
                        {
                            this->wctx->thiswline = this->findwline(this->wctx->thiswline);
                            if(this->wctx->thiswline == -1) 
                                done = 1;
                        }
                    }
                }
                if(this->wctx->syllcount > 0) 
                {
                    /* still finishing off a multi-syllable item */
                    this->wctx->syllcount--;
                    this->wctx->lyricsyllables++;
                    done = 1;
                }
            }
        }
    }
}

/* decide whether passno matches list/number for variant section */
/* handles representation of [X in the abc */
int 
AbcGenMidi::inlist(int place, int passno)
{

    /* printf("passno = %d\n", passno); */
    Abc::FeatureDesc &fd = this->initState->featurelist[place];
    if(fd.denom != 0) 
    {
        /* special case when this is variant ending for only one pass */
        if (passno == fd.denom) 
            return 1;
        else
            return 0;
    } 
    else 
    {
        /* must scan list */
        char msg[100];
        char const *p = this->initState->atext[fd.pitch].c_str();
        int found = 0;
        while((found == 0) && (*p != '\0')) 
        {
            if (!isdigit(*p)) 
            {
                snprintf(msg, 100, "Bad variant list : %s", 
                    this->initState->atext[fd.pitch].c_str());
                this->wctx->error(msg);
                found = 1;
            }
            int a = AbcParser::Readnump(&p);
            if(passno == a)
                found = 1;
            if(*p == '-') 
            {
                p = p + 1;
                int b = AbcParser::Readnump(&p);
                if((passno >= a) && (passno <= b))
                    found = 1;
            }
            if (*p == ',') 
                p = p + 1;
        }
        return found;
    }
}

/* write text event to MIDI file --- */
void
AbcGenMidi::text_data(char const *s)
{
    this->midi->writeMetaEvent(this->wctx->delta_time, 
        MidiEvent::text_event, (unsigned char *) s, strlen(s));
    this->wctx->tracklen = this->wctx->tracklen + this->wctx->delta_time;
    this->wctx->delta_time = 0L;
}

/* check line of lyrics matches line of music. It grabs
 * all remaining syllables in the lyric line counting
 * them as it goes along. It then checks that the number
 * of syllables matches the number of notes for that music
 * line
*/
void
AbcGenMidi::checksyllables()
{
    int syllcount;
    char msg[80];

    /* first make sure all lyric syllables are read */
    int done = 0;
    while (!done) 
    {
        syllcount = this->getword(&this->wctx->wordlineplace, this->wctx->windex);
        if(syllcount > 0) 
        {
            this->wctx->lyricsyllables = this->wctx->lyricsyllables + syllcount;
        } 
        else 
        {
            this->wctx->thiswline = this->findwline(this->wctx->thiswline);
            if(this->wctx->thiswline == -1)
                done = 1;
            else 
            {
                this->wctx->windex = 
                    this->initState->featurelist[this->wctx->thiswline].pitch;
            }
        }
    }
    if(this->wctx->onemorenote == 1)
        this->wctx->lyricsyllables = this->wctx->lyricsyllables + 1;

    if(this->wctx->lyricsyllables != this->wctx->musicsyllables)
    {
        if(this->initState->verbose) 
        {
            snprintf(msg, 80, 
                "Verse %d mismatch;  %d syllables in music %d in lyrics",
                this->wctx->partrepno+1, this->wctx->musicsyllables, 
                this->wctx->lyricsyllables);
            this->wctx->error(msg);
        }
    }
    if(this->wctx->onemorenote == 1)
    {
        this->wctx->onemorenote = 0;
        /*printf("onemorenote please, hyphenstate to zero\n (using lyric- instead of note-hyphen)\n"); //not the most elegant solution.. but it works */
        this->wctx->hyphenstate = 0;
    }
    this->wctx->lyricsyllables = 0;
    this->wctx->musicsyllables = 0;
}

/* Find next line of lyrics at or after startline. */
int 
AbcGenMidi::findwline(int startline)
{

    /*   printf("findwline called with %d\n", startline); */
    int done = 0;
    int inwline = 0;
    this->wctx->nowordline = 0;
    int newwordline = -1;
    int target = this->wctx->partrepno;
    int extending;
    int versecount;
    if (startline == this->wctx->thismline) 
    {
        versecount = 0;
        extending = 0;
    } 
    else 
    {
        versecount = target;
        extending = 1;
    }
    if(this->wctx->thismline == -1) 
    {
        this->wctx->error("First lyrics line must come after first music line");
    } 
    else 
    {
        int place = startline + 1;
        /* search for corresponding word line */
        while((place < this->initState->nfeatures) && (!done)) 
        {
            Abc::FeatureDesc &fd = this->initState->featurelist[place];
            switch(fd.feature) 
            {
            case Abc::WORDLINE:
                inwline = 1;
                /* wait for words for this pass */
                if (versecount == target) 
                {
                    this->wctx->thiswfeature = place;
                    newwordline = place;
                    this->wctx->windex = fd.pitch;
                    this->wctx->wordlineplace = 0; // overloaded
                    done = 1;
                }
                break;
            case Abc::WORDSTOP:
                if (inwline) {
                    versecount = versecount + 1;
                };
                inwline = 0;
                /* stop if we are part-way through a lyric set */
                if(extending)
                    done = 1;
                break;
            case Abc::PART:
                done = 1;
                break;
            case Abc::VOICE:
                done = 1;
                break;
            case Abc::MUSICLINE:
                done = 1;
                break;
            default:
                break;
            }
            place = place + 1;
            if (done && (newwordline == -1) && (versecount > 0) && (!extending)) 
            {
                target = this->wctx->partrepno % versecount ;
                versecount = 0;
                place = startline+1;
                done = 0;
                inwline = 0;
            }
        } // end while
        if(newwordline == -1) 
        {
            /* remember that we couldn't find lyrics */
            this->wctx->nowordline = 1;
            if(this->wctx->lyricsyllables == 0) 
                this->wctx->warning("Line of abc music without lyrics");
        }
    }
    return newwordline;
}

/* picks up next syllable out of w: field.
 * It strips out all the control codes ~ - _  * in the
 * words and sends each syllable out to the Karaoke track.
 * Using the place variable, it loops through each character
 * in the word until it encounters a space or next control
 * code. The syllstatus variable controls the loop. After,
 * the syllable is sent, it then positions the place variable
 * to the next syllable or control code.
 * inword   --> grabbing the characters in the syllable and
 *             putting them into syllable for output.
 * postword --> finished grabbing all characters
 * foundnext--> ready to repeat process for next syllable
 * empty    --> between syllables.
 *
 * The variable i keeps count of the number of characters
 * inserted into the syllable[] char for output to the
 * karaoke track. The kspace variables signals that a
 * space was encountered.
 */
int 
AbcGenMidi::getword(int *place, int w)
{
    char syllable[200];
    unsigned char c; /* [BY] 2012-10-03 */
    int i;
    int syllcount;
    enum {empty, inword, postword, foundnext, failed} syllstatus;
    /* [BY] 2012-10-03  Big5 chinese character support */

    /*printf("GETWORD: w = %d\n",c);*/
    i = 0;
    syllcount = 0;
    if(w >= this->initState->words.size()) 
    {
        syllable[i] = '\0';
        return '\0';
    }
    if(*place == 0) 
    {
        if ((w % 2) == 0) 
            syllable[i] = '/'; 
        else
            syllable[i] = '\\'; 
        i = i + 1;
    }
    if(this->wctx->kspace) 
    {
        syllable[i] = ' ';
        i = i + 1;
    }
    syllstatus = empty;
    if(*place < this->initState->words[w].size())
        c = this->initState->words[w].at(*place);
    else
        c = '\0';
    int isBig5 = 0; /* boolean check for first byte of Big-5: 0xA140 ~ 0xF9FE */
    while((syllstatus != postword) && (syllstatus != failed)) 
    {
        syllable[i] = c;
        /* printf("syllstatus = %d c = %c i = %d place = %d row= %d \n",syllstatus,c,i,*place,w); */
        if(isBig5) 
        {
            i = i + 1;
            *place = *place + 1;
            isBig5 = 0;
        } 
        else 
        {
            switch(c) 
            {
            case '\0':
                if (syllstatus == empty)
                    syllstatus = failed;
                else
                {
                    syllstatus = postword;
                    this->wctx->kspace = 1;
                }
                break;
            case '~':
                syllable[i] = ' ';
                syllstatus = inword;
                *place = *place + 1;
                i = i + 1;
                this->wctx->hyphenstate = 0;
                break;
            case '\\':
                if(this->initState->words[w].size() > (*place+1) &&
                   this->initState->words[w].at(*place+1) == '-') 
                {
                    syllable[i] = '-';
                    syllstatus = inword;
                    *place = *place + 2;
                    i = i + 1;
                } 
                else 
                {
                    /* treat like plain text */
                    *place = *place + 1;
                    if (i>0) 
                    {
                        syllstatus = inword;
                        i = i + 1;
                    }
                }
                break;
            case ' ':
                if(syllstatus == empty) 
                {
                    *place = *place + 1;
                } 
                else 
                {
                    syllstatus = postword;
                    *place = *place + 1;
                    this->wctx->kspace = 1;
                }
                break;
            case '-':
                if(this->wctx->hyphenstate == 1) 
                {
                    i = i + 1; syllstatus = postword; *place = *place + 1;
                    break;
                }
                if (syllstatus == inword) 
                {
                    syllstatus = postword;
                    *place = *place + 1;
                    this->wctx->kspace = 0;
                } 
                else 
                    *place = *place + 1;
                this->wctx->hyphenstate = 1;
                break;
            case '*':
                if(syllstatus == empty) 
                {
                    syllstatus = postword;
                    *place = *place + 1;
                } 
                else 
                    syllstatus = postword;
                this->wctx->hyphenstate = 0;
                break;
            case '_':
                if(syllstatus == empty)
                {
                    syllstatus = postword;
                    *place = *place + 1;
                } 
                else 
                    syllstatus = postword;
                this->wctx->hyphenstate = 0;
                break;
            case '|':
                if(syllstatus == empty) 
                {
                    syllstatus = failed;
                    *place = *place + 1;
                } 
                else 
                {
                    syllstatus = postword;
                    *place = *place + 1;
                    this->wctx->kspace = 1;
                }
                this->wctx->waitforbar = 1;
                this->wctx->hyphenstate = 0;
                break;
            default:
                /* copying plain text character across */
                /* first character must be alphabetic */
                this->wctx->hyphenstate = 0;
                if(c >= 0xA1)	/* 0xA1, 161 */
                    isBig5 = 1;
                if((i>0) || isalpha(syllable[0]) || (c >= 0xA1)) 
                {
                    syllstatus = inword;
                    i = i + 1;
                }
                *place = *place + 1;
                break;
            } // end switch
        } // end !isBig5
        if(*place < this->initState->words[w].size())
            c = this->initState->words[w].at(*place);
        else
            c = '\0';
    } // end while
    syllable[i] = '\0';
    if(syllstatus == failed) 
        syllcount = 0;
    else 
    {
        syllcount = 1;
        if(strlen(syllable) > 0) 
        {
            this->text_data(syllable);
            /*printf("TEXT DATA %s\n",syllable);*/
        }
    }
    /* now deal with anything after the syllable */
    while((syllstatus != failed) && (syllstatus != foundnext))
    {
        if(*place < this->initState->words[w].size())
            c = this->initState->words[w].at(*place);
        else
            c = '\0';
        /*printf("next character = %c\n",c);*/
        switch (c) 
        {
        case ' ':
            *place = *place + 1;
            break;
        case '-':
            *place = *place + 1; 
            this->wctx->kspace = 0;
            syllcount = syllcount + 1; /* [SS] 2011-02-23 */
            break;
        case '\t':
            *place = *place + 1;
            break;
        case '_':
            *place = *place + 1;
            syllcount = syllcount + 1;
            break;
        case '|':
            if(this->wctx->waitforbar == 0) 
            {
                *place = *place + 1;
                this->wctx->waitforbar = 1;
            } 
            else 
                syllstatus = failed;
            break;
        default:
            syllstatus = foundnext;
            break;
        }
        /* printf("now place = %d syllcount = %d syllstatus = %d\n",*place,syllcount,syllstatus); */
    }
    return syllcount;
}

/* ------------------------------------------------------------------- */
int 
AbcGenMidi::parse_stress_params(char *input) 
{
    char *next;
    float f = (float) strtod(input, &next);
    input = next;
    if(*input == '\0') 
    {
        /* no data, probably file name */
        return -1;
    } 
    if(f == 0.0f) 
        return -1;

    this->nseg = (int) (f +0.0001);
    if(this->nseg > 31) 
        return -1;

    int n;
    for(n=0;n<32;n++) 
    {
        this->fdur[n]= 0.0; 
        this->ngain[n] = 0;
    }

    n = 0;
    while (*input != '\0' && n < nseg) 
    {
        f = (float) strtod(input, &next);
        this->ngain[n] = (int) (f + 0.0001);
        if(this->ngain[n] > 127 || this->ngain[n] <0) 
        {
            printf("**error** bad velocity value ngain[%d] = %d in ptstress command\n",
                n, this->ngain[n]);
        }
        input = next;
        f = (float) strtod (input,&next);
        fdur[n] = f;
        if (fdur[n] > (float) nseg || fdur[n] < 0.0) 
        {
            printf("**error** bad expansion factor fdur[%d] = %f in ptstress command\n",
                n, this->fdur[n]);
        }
        input = next;
        n++;
    }
    if (n != this->nseg) 
        return -1;
    else 
    {
        this->beatmodel = 2; /* [SS] 2018-04-14 */
        this->barflymode = 1; /* [SS] 2018-04-16 */
        return 0;
    }
} 

void 
AbcGenMidi::readstressfile(char const * filename)
{
    this->maxdur = 0;
    FILE *inputhandle = fopen(filename,"r");
    if (inputhandle == nullptr) 
    {
        printf("Failed to open file %s\n", filename);
        return;
    }
    for(int n=0;n<32;n++) 
    {
        this->fdur[n]= 0.0; 
        this->ngain[n] = 0;
    }
    this->fdursum[0] = fdur[0];
    this->beatmodel = 2; /* for Phil Taylor's stress model */
    int idummy = fscanf(inputhandle,"%d",&nseg);
    /*printf("%d\n",nseg);*/
    if (nseg > 31) 
        nseg=31;

    for(int n=0;n<this->nseg+1;n++) 
    {
        idummy = fscanf(inputhandle,"%d %f",
                    &this->ngain[n],
                    &this->fdur[n]);
    }
    fclose(inputhandle);
    this->stress_pattern_loaded = 1;
}

void 
AbcGenMidi::calculate_stress_parameters(int time_num, int time_denom) 
{
    int qnotenum,qnoteden;
    float lastsegvalue;
    this->segnum = time_num;
    this->segden = time_denom*this->nseg;
    AbcMusic::reduceFraction(&this->segnum, &this->segden);

    /* compute number of segments in quarter note */
    qnotenum = this->segden;
    qnoteden = this->segnum*4;
    AbcMusic::reduceFraction(&qnotenum, &qnoteden);
    float qfrac = qnotenum / (float) qnoteden;
    for (int n=0; n<this->nseg+1; n++) 
    {
        this->maxdur = std::max(this->maxdur, this->fdur[n]);
        if (n > 0) 
            this->fdursum[n] = this->fdursum[n-1] + 
                                this->fdur[n-1] * qfrac;
        if (this->fdursum[n] > (float) this->nseg + 0.05) 
        {
            printf("**error** bad abc stress file: sum of the expansion factors exceeds number of segments\nAborting stress model\n");
            this->beatmodel = 0;
            return;
        }
        if (ngain[n] > 127 || ngain[n] < 0) 
        {  
            printf("**error** bad abc stress file: note velocity not between 0 and 127\n Aborting the stress model\n");
            this->beatmodel = 0;
            return;
        }
        lastsegvalue = (float) this->nseg * qfrac;
        /* ensure fdursum[nseg] = lastsegvalue [SS] 2011-09-06 */
        if(this->fdursum[this->nseg] != lastsegvalue)
        {
            printf("**warning** the sum of the abc expansion factors is not %d\n some adjustments are made.\n",
                    nseg);
            this->fdursum[nseg] = lastsegvalue;
        }
    }
}

/* these updates may occur while parsing the abc ---- */
void 
AbcGenMidi::set_meter(int num, int denom)
{
    this->wctx->set_meter(num, denom);
}

/* add a/b to the count of units in the bar */
void
AbcGenMidi::addBarUnits(int a, int b)
{
    this->wctx->addBarUnits(a, b);
}

void
AbcGenMidi::getBarUnits(int *a, int *b)
{
    this->wctx->getBarUnits(a, b);
}

void
AbcGenMidi::zerobar()
{
    this->wctx->zerobar();
}

/* set up a string which indicates drum pattern */
/* called from dodeferred() */
void 
AbcGenMidi::set_drums(char const *s)
{
    this->wctx->set_drums(s);
}

void
AbcGenMidi::drum_map(int mpitch, int mapto)
{
    this->wctx->drum_map[mpitch] = mapto;
}

void 
AbcGenMidi::set_gchords(char const *s)
{
    this->wctx->set_gchords(s);
}

/* Work out next available midi channel.  
 * MIDI spec maxes out at 16 channels.
 */
int 
AbcGenMidi::findchannel()
{
    int j;
    for(j=0; j<AbcMidiTrackCtx::MAXTRACKS; j++) // eg: 64
    {
        if(this->channel_in_use[j] == 0)
            break;
    }
    if(j == AbcMidiTrackCtx::MAXCHANS) // eg: 16
        this->wctx->warning("All 16 abc channels used up, recycling.");
    this->channel_in_use[j] = 1;
    return (j % AbcMidiTrackCtx::MAXCHANS);
}

/* find where next occurrence of correct voice is */
int 
AbcGenMidi::findvoice(int initplace, int voice, int xtrack)
{
    int j = initplace;
    while(j < this->initState->nfeatures)
    {
        Abc::FeatureDesc &fd = this->initState->featurelist[j];
        if(fd.feature == Abc::LINENUM) 
            this->wctx->lineno = fd.pitch;
        else
        if(fd.feature == Abc::PART) 
        {
            j = this->partbreak(xtrack, voice, j);
            if (voice == 1) 
                return j; // <--- found
        } 
        else 
        {
            if((fd.feature == Abc::VOICE) && (fd.pitch == voice)) 
                return j; // <--- found
        }
        j++;
    }
    return j; // <--- not found (j == nfeatures)
} 

// a voiceEnd is inferred when we encounter another voice or
// we reach the end of the featurelist.
int 
AbcGenMidi::findvoiceEnd(int voiceBegin, int voice, int xtrack)
{
    int j = voiceBegin + 1;
    while(j < this->initState->nfeatures)
    {
        Abc::FeatureDesc &fd = this->initState->featurelist[j];

        if((fd.feature == Abc::VOICE) && (fd.pitch != voice)) 
            return j-1; // <--- found
        // XXX: what about part?
        else
            j++;
    }
    return j-1; // eof (last voice)
} 

/* compute note data and call noteon_data to write MIDI note event */
void 
AbcGenMidi::noteon(int n, Abc::FeatureDesc &fd)
{
    int vel = 0;
    if(this->beatmodel != 0)
        this->stress_factors(n, &vel);
    if(vel == 0) 
        this->note_beat(n, &vel);

    if(this->wctx->single_velocity >= 0)
        vel = this->set_velocity_for_one_note();
    else 
    if(this->wctx->single_velocity_inc != 0)
        vel = this->apply_velocity_increment_for_one_note (vel);

    if(this->wctx->channel == 9) 
        this->noteon_data(fd.pitch, fd.bentpitch, this->wctx->channel, vel);
    else 
        this->noteon_data(fd.pitch + this->wctx->transpose + this->wctx->global_transpose, 
                    fd.bentpitch, this->wctx->channel, vel);
}

void
AbcGenMidi::noteon_data(int pitch, int pitchbend, int chan, int vel)
{
    this->midi_noteon(this->wctx->delta_time, pitch, pitchbend, chan, vel);
    this->wctx->tracklen = this->wctx->tracklen + this->wctx->delta_time;
    this->wctx->delta_time = 0L;
}

void 
AbcGenMidi::midi_noteon(long delta_time, int pitch, int pitchbend, int chan, int vel)
{
    unsigned char data[2];
    if(pitchbend < 0 || pitchbend > 16383) 
    {
        this->wctx->error("Internal error concerning pitch bend on note on.");
    }

    if(pitchbend != this->wctx->current_pitchbend[this->wctx->channel] && 
        chan != 9) 
    {
        data[0] = (unsigned char) (pitchbend & 0x7f);
        data[1] = (unsigned char) ((pitchbend>>7) & 0x7f);
        this->wctx->bendstate = pitchbend;
        this->midi->writeMidiEvent(delta_time, 
            MidiEvent::pitch_wheel, chan, data, 2);
        delta_time = 0;
        this->wctx->current_pitchbend[this->wctx->channel] = pitchbend;
    }

    if(chan == 9) 
        data[0] = this->wctx->drum_map[pitch];
    else
        data[0] = pitch;
    data[1] = vel;
    this->midi->writeMidiEvent(delta_time, 
        MidiEvent::note_on, chan, data, 2);
}

/* write note off event to MIDI file */
void 
AbcGenMidi::midi_noteoff(long delta_time, int pitch, int chan)
{
    unsigned char data[2];
    if (chan == 9) 
        data[0] = this->wctx->drum_map[pitch];
    else
        data[0] = pitch;
    data[1] = 0;
    this->midi->writeMidiEvent(delta_time, 
            MidiEvent::note_off, chan, data, 2);
}

/* changes the master coarse tuning and master fine tuning
   using Register Parameter Number (RPN) for a specific
   track. See http://home.roadrunner.com/~jgglatt/tech/midispec/rpn.htm
   or http://www.2writers.com/eddie/TutNrpn.htm for a tutorial on how
   this is done.
*/
void 
AbcGenMidi::midi_re_tune(int channel) 
{
    unsigned char data[2];
    #if 0 
    // XXX - unused
    data[0] = (this->initState->bend & 0x7f); /* least significant bits */
    data[1] = ((this->initState->bend >>7) & 0x7f);
    #endif
    /* indicate that we are applying RPN fine and gross tuning using
        the following two control commands. 
        control 101 0  
        control 100 1 */
    data[0] = 101; /* RPN command */
    data[1] = 0;   /* type of command */
    this->midi->writeMidiEvent(0, MidiEvent::control_change, channel, data, 2);

    data[0] = 100; /* RPN command */
    data[1] = 1;   /* type of command */
    this->midi->writeMidiEvent(0, MidiEvent::control_change, channel, data, 2);
    /* now enter the bend parameters using the control data entry
        commands for the least significant and most significant bits
    */
    data[0] = 6; /* control data entry for coarse bits */
    data[1] = ((this->initState->bend >>7) & 0x7f);
    this->midi->writeMidiEvent(0, MidiEvent::control_change, channel, data, 2);

    data[0] = 38; /* control data entry for fine bits */
    data[1] = (this->initState->bend & 0x7f); /* least significant bits */
    this->midi->writeMidiEvent(0, MidiEvent::control_change, channel, data, 2);
} 

/* set velocity */
void
AbcGenMidi::note_beat(int n, int *vel)
{
    if(this->wctx->beataccents == 0) 
        *vel = this->wctx->mednote;
    else 
    if(this->wctx->nbeats > 0) 
    {
        if((this->wctx->bar_num*this->wctx->nbeats) % 
            (this->wctx->bar_denom*this->wctx->barsize) != 0) 
        {
            /* not at a defined beat boundary */
            *vel = this->wctx->softnote;
        } 
        else 
        {
            /* find place in beatstring */
            int i = ((this->wctx->bar_num*this->wctx->nbeats)/
                     (this->wctx->bar_denom*this->wctx->barsize)) % this->wctx->nbeats;
            switch(this->wctx->beatstring[i]) 
            {
            case 'f':
            case 'F':
                *vel = this->wctx->loudnote;
                break;
            case 'm':
            case 'M':
                *vel = this->wctx->mednote;
                break;
            default:
            case 'p':
            case 'P':
                *vel = this->wctx->softnote;
                break;
            }
        }
    } 
    else 
    {
        /* no beatstring - use beat algorithm */
        if(this->wctx->bar_num == 0)
        {
            *vel = this->wctx->loudnote;
        } 
        else 
        {
            if ((this->wctx->bar_denom == 1) && 
                ((this->wctx->bar_num % this->wctx->beat) == 0)) 
            {
                *vel = this->wctx->mednote;
            }
            else 
                *vel = this->wctx->softnote;
        }
    }
}

int 
AbcGenMidi::set_velocity_for_one_note()
{
    int velocity = this->wctx->single_velocity;
    this->wctx->single_velocity = -1;
    return velocity;
}

int 
AbcGenMidi::apply_velocity_increment_for_one_note(int vel)
{
    vel = vel + this->wctx->single_velocity_inc;
    if(vel < 0) vel = 0;
    if(vel > 127) vel = 127;
    this->wctx->single_velocity_inc = 0; 
    return vel;
}

/* come to part label in note data 
 * - check part length, then advance to next part 
 * if there was a P: field in the header 
 */
int 
AbcGenMidi::partbreak(int xtrack, int voice, int place)
{
    int newplace = place;
    if(this->initState->dependent_voice[voice]) 
        return newplace;
    if(xtrack > 0)
        this->fillvoice(this->wctx->partno, xtrack, voice);
    if(this->parts != -1) 
    {
        /* go to next part label */
        newplace = this->findpart(newplace);
    }
    this->wctx->partlabel = (int) this->initState->featurelist[newplace].pitch - 'A';
    return newplace;
}

/* find out where next part starts and update partno */
int
AbcGenMidi::findpart(int j)
{
    int place = j;
    this->wctx->partno = this->wctx->partno + 1;
    if(this->wctx->partno < this->parts) 
    {
        this->wctx->partlabel = (int)this->partspec[this->wctx->partno] 
                                        - (int)'A';
    }
    while((this->wctx->partno < this->parts) && 
        (this->part_start[this->wctx->partlabel] == -1)) 
    {
        if(!this->initState->silent) 
            this->wctx->error("Part not defined");
        this->wctx->partno++;
        if(this->wctx->partno < this->parts) 
        {
            this->wctx->partlabel = (int)this->partspec[this->wctx->partno] - 'A';
        }
    }
    if(this->wctx->partno >= this->parts) 
    {
        place = this->initState->nfeatures; // done
    } 
    else 
    {
        this->wctx->partrepno = this->wctx->part_count[this->wctx->partlabel];
        this->wctx->part_count[this->wctx->partlabel]++;
        place = this->part_start[this->wctx->partlabel];
    }
    if(this->initState->verbose) 
    {
        if(this->wctx->partno < this->parts) 
        {
            char buf[100];
            snprintf(buf, 100, "Playing part abc %c number %d of %d\n", 
                this->partspec[this->wctx->partno], 
                this->wctx->partno+1, 
                this->parts);
            this->wctx->log(buf);
        }
    }
    return place;
}

/* check length of this voice at the end of a part */
/* if it is zero, extend it to the correct length */
void 
AbcGenMidi::fillvoice(int partno, int xtrack, int voice)
{
    char msg[100];
    if(this->wctx->partlabel < -1 || this->wctx->partlabel > 25) 
    {
        snprintf(msg, 100, "genmidi fillvoice partlabel %d out of range",
            this->wctx->partlabel);
        this->wctx->error(msg);
        return;
    }

    long now = this->wctx->tracklen + this->wctx->delta_time;
    if(this->wctx->partlabel == -1)
    {
        if(xtrack == 1) 
        {
            this->wctx->introlen = now;
        } 
        else 
        {
            if (now == 0) 
            {
                this->wctx->delta_time += this->wctx->introlen;
                now = this->wctx->introlen;
            } 
            else 
            {
                if (now != this->wctx->introlen) 
                {
                    snprintf(msg, 100, "Time 0-%ld voice %d, has length %ld", 
                        this->wctx->introlen, voice, now);
                    this->wctx->error(msg);
                }
            }
        }
    } 
    else 
    {
        if (xtrack == 1) 
        {
            this->wctx->partlen[this->wctx->partlabel] = now - this->wctx->lastlen;
        } 
        else 
        {
            if(now - this->wctx->lastlen == 0) 
            {
                this->wctx->delta_time += this->wctx->partlen[this->wctx->partlabel];
                now = now + this->wctx->partlen[this->wctx->partlabel];
            } 
            else 
            {
                if(now - this->wctx->lastlen != this->wctx->partlen[this->wctx->partlabel]) 
                {
                    snprintf(msg, 100,
                        "Time %ld-%ld voice %d, part %c has length %ld", 
                        this->wctx->lastlen, 
                        this->wctx->lastlen+this->wctx->partlen[this->wctx->partlabel], 
                        voice, 
                        (char) (this->wctx->partlabel + (int) 'A'), 
                        now - this->wctx->lastlen);
                    this->wctx->error(msg);
                }
            }
        }
    }
    this->wctx->lastlen = now;
}

/* check to see we have the right number of notes in the bar */
void 
AbcGenMidi::checkbar(int pass)
{
    char msg[100];
    if(this->wctx->barno >= 0 && this->wctx->barno < 1024 && pass == 1) 
        this->wctx->barloc[this->wctx->barno] = this->wctx->bar_num;

    if(this->wctx->barchecking) 
    {
        /* only generate these errors once */
        if(this->wctx->noteson && (this->wctx->partrepno == 0)) 
        {
            /* allow zero length bars for typesetting purposes */
            if((this->wctx->bar_num - this->wctx->barsize*this->wctx->bar_denom != 0) &&
                this->wctx->bar_num != 0 && 
                ((pass == 2) || (this->wctx->barno != 0)) &&
                this->initState->quiet == -1) 
            {
                // report messages in index-origin 1
                snprintf(msg, 100, "abc track %d bar %d has %d",
                    this->wctx->tracknumber+1, this->wctx->barno+1, this->wctx->bar_num);
                if(this->wctx->bar_denom != 1) 
                    sprintf(msg+strlen(msg), "/%d", this->wctx->bar_denom);
                sprintf(msg+strlen(msg), " units instead of %d", this->wctx->barsize);
                if(pass == 2) 
                    strcat(msg, " in repeat");
                if(this->initState->quiet == -1) 
                    this->wctx->warning(msg);
            }
            else
            if(0) // debuggin
            {
                // report messages in index-origin 1
                snprintf(msg, 100, "abc track %d bar %d has %d",
                    this->wctx->tracknumber+1, this->wctx->barno+1, this->wctx->bar_num);
                if(this->wctx->bar_denom != 1) 
                    sprintf(msg+strlen(msg), "/%d", this->wctx->bar_denom);
                sprintf(msg+strlen(msg), " units (%d)", this->wctx->barsize);
                if(this->initState->quiet == -1) 
                    this->wctx->warning(msg);
            }

        }
    }
    if(this->wctx->bar_num > 0) 
    {
        this->wctx->barno = this->wctx->barno + 1;
    }
    this->wctx->bar_num = 0;
    this->wctx->bar_denom = 1;
    /* zero place in gchord sequence */
    if(this->wctx->gchordson) 
    {
        if(this->wctx->gchordbars < 2) 
            this->wctx->gchord_index = 0;

        this->wctx->addtoQ(0, this->wctx->g_denom, -1, this->wctx->gchord_index, 0, 0);
    }
    if(this->wctx->drumson) 
    {
        if(this->wctx->drumbars < 2) 
        {
            this->wctx->drum_ptr = 0;
            this->wctx->addtoQ(0, this->wctx->drum_denom, -1, this->wctx->drum_ptr, 0, 0);
        }
        this->wctx->addtoQ(0, this->wctx->drum_denom, -1, this->wctx->drum_ptr, 0, 0);
    }
}

/* allows repeats to be in mid-bar */
void 
AbcGenMidi::softcheckbar(int pass)
{
    if(this->wctx->barchecking) 
    {
        if((this->wctx->bar_num - this->wctx->barsize*this->wctx->bar_denom >= 0) 
            || (this->wctx->barno <= 0)) 
        {
            this->checkbar(pass);
        }
    }
}

/* handle package-specific command which has been held over to be
 * interpreted as MIDI is being generated
 */
void
AbcGenMidi::dodeferred(char const *s, int noteson)
{
    int i;
    char const *p = s;
    int done = 0;
    char command[40];
    char msg[100];
    AbcParser::Skipspace(&p);
    AbcParser::Readstr(command, &p, 40);
    AbcParser::Skipspace(&p);

    if(this->initState->verbose>1)
    {
        snprintf(msg, 100, "dodeferred: track = %d cmd = %s\n",
            this->wctx->tracknumber, command);
        this->wctx->log(msg);
    }

    if (strcmp(command,"makechordchannels") == 0) 
    {
        AbcParser::Skipspace(&p);
        int val = AbcParser::Readnump(&p);
        this->makechordchannels(val);
        done = 1;
    } 
    else 
    if(strcmp(command, "program") == 0)
    {
        AbcParser::Skipspace(&p);
        int prog = AbcParser::Readnump(&p);
        int chan = this->wctx->channel;
        AbcParser::Skipspace(&p);
        if ((*p >= '0') && (*p <= '9')) 
        {
            chan = prog - 1;
            prog = AbcParser::Readnump(&p);
        }
        if(noteson) 
        {
            this->wctx->current_program[chan] = prog;
            this->write_program(prog, chan);
        }
        done = 1;
    }
    else 
    if(strcmp(command, "gchord") == 0) 
    {
        this->set_gchords(p);
        done = 1;
    }
    else 
    if (strcmp(command, "drum") == 0) 
    {
        this->set_drums(p);
        done = 1;
    }
    else 
    if((strcmp(command, "drumbars") == 0)) 
    {
        this->wctx->drumbars = AbcParser::Readnump(&p);
        if(this->wctx->drumbars < 1 || this->wctx->drumbars > 10) 
            this->wctx->drumbars = 1;
        done = 1;
        this->wctx->drum_ptr = 0;
        this->wctx->addtoQ(0, this->wctx->drum_denom,-1, this->wctx->drum_ptr,0,0);
    }
    else 
    if ((strcmp(command, "gchordbars") == 0))
    {
        this->wctx->gchordbars = AbcParser::Readnump(&p);
        if(this->wctx->gchordbars < 1 || this->wctx->gchordbars > 10)     
            this->wctx->gchordbars = 1;
        done = 1;
        this->wctx->gchord_index = 0; /* [SS] 2018-06-23 */
        this->wctx->addtoQ(0, this->wctx->g_denom, -1, this->wctx->gchord_index ,0, 0);
    }
    else 
    if((strcmp(command, "chordprog") == 0))  
    {
        int prog = AbcParser::Readnump(&p);
        if(this->wctx->gchordson) 
        {
            this->write_program(prog, this->wctx->gchord.chan);
            p = strstr(p,"octave=");
            if (p != 0)
            {
                p = p+7;
                int octave;
                int found = sscanf(p,"%d",&octave);
                if (found == 1 && octave > -3 && octave < 3) 
                    this->wctx->gchord.base = 48 + 12*octave;
                snprintf(msg, 100, "gchord.base = %d", this->wctx->gchord.base);
                this->wctx->log(msg);
            }
        }
        done = 1;
    }
    else 
    if((strcmp(command, "bassprog") == 0))
    {
        int prog = AbcParser::Readnump(&p);
        if(this->wctx->gchordson) 
        {
            this->write_program(prog, this->wctx->fun.chan);
            p = strstr(p,"octave=");
            if (p != 0)
            {
                p = p+7;
                int octave;
                int found = sscanf(p,"%d",&octave);
                if(found == 1 && octave > -3 && octave < 3)
                    this->wctx->fun.base = 36 + 12*octave;
                snprintf(msg, 100, "fun.base = %d", this->wctx->fun.base);
                this->wctx->log(msg);
            }
        }
        done = 1;
    }
    else
    if(strcmp(command, "chordvol") == 0)
    {
        this->wctx->gchord.vel = AbcParser::Readnump(&p);
        done = 1;
    }
    else 
    if(strcmp(command, "bassvol") == 0) 
    {
        this->wctx->fun.vel = AbcParser::Readnump(&p);
        done = 1;
    }
    else 
    if(strcmp(command, "bendvelocity") == 0) 
    {
        /* We use bendstring code so that bendvelocity integrates with !shape!.
        Bends a note along the shape of a parabola. The note is
        split into 8 segments. Given the bendacceleration and
        initial bend velocity, the new pitch bend is computed
        for each time segment.
        */
        this->wctx->bendvelocity = this->wctx->bendacceleration = 0;
        AbcParser::Skipspace(&p);
        int val = AbcParser::Readsnump(&p);
        this->wctx->bendvelocity = val;
        AbcParser::Skipspace(&p);
        val = AbcParser::Readsnump(&p);
        this->wctx->bendacceleration = val;
        this->wctx->bendnvals = 0;
        if(this->wctx->bendvelocity != 0 || this->wctx->bendacceleration != 0) 
        {
            for (i = 0; i<8; i++) 
            {
                this->wctx->benddata[i] = this->wctx->bendvelocity;
                this->wctx->bendvelocity = this->wctx->bendvelocity + this->wctx->bendacceleration;
            }
            this->wctx->bendnvals = 8;
        }
        if(this->wctx->bendnvals == 1) 
            this->wctx->bendtype = 3;
        else 
            this->wctx->bendtype = 2;
        done = 1;
    }
    else 
    if(strcmp(command, "bendstring") == 0)
    {
        i = 0;
        while(i<256) 
        {
            this->wctx->benddata[i] = AbcParser::Readsnump(&p);
            AbcParser::Skipspace(&p);
            i = i + 1;
            /* [SS] 2015-08-31 */
            if (*p == 0) 
                break;
        }
        this->wctx->bendnvals = i;
        done = 1;
        if(this->wctx->bendnvals == 1) 
            this->wctx->bendtype = 3;
        else 
            this->wctx->bendtype = 2;
    }
    else 
    if(strcmp(command, "bendstringex") == 0)  // "smooth" (4x) the incoming data
    {
        int bendinput[64];
        i = 0;
        while(i<64) 
        {
            bendinput[i] = AbcParser::Readsnump(&p);
            AbcParser::Skipspace(&p);
            i = i + 1;
            if(*p == 0) 
                break;
        }
        this->expand_array(bendinput, i, this->wctx->benddata, 4);
        this->wctx->bendnvals = i*4;
        done = 1;
        if(this->wctx->bendnvals == 1) 
            this->wctx->bendtype = 3;
        else 
            this->wctx->bendtype = 2;
    }
    else 
    if(strcmp(command, "drone") == 0) 
    {
        // %%MIDI drone 70 45 33 90 90
        AbcParser::Skipspace(&p);
        int val = AbcParser::Readnump(&p);
        if(val > 0) 
            this->wctx->drone.prog = val;
        AbcParser::Skipspace(&p);
        val = AbcParser::Readnump(&p);
        if(val >0) 
            this->wctx->drone.pitch1 = val;
        AbcParser::Skipspace(&p);
        val = AbcParser::Readnump(&p);
        if(val >0)  
            this->wctx->drone.pitch2 = val;
        AbcParser::Skipspace(&p);
        val = AbcParser::Readnump(&p);
        if(val >0) 
            this->wctx->drone.vel1 = val;
        AbcParser::Skipspace(&p);
        val = AbcParser::Readnump(&p);
        if(val >0) 
            this->wctx->drone.vel2 = val;
        if(this->wctx->drone.prog > 127) 
            this->wctx->error("drone prog must be in the range 0-127");
        if(this->wctx->drone.pitch1 >127) 
            this->wctx->error("drone pitch1 must be in the range 0-127");
        if(this->wctx->drone.vel1 >127) 
            this->wctx->error("drone vel1 must be in the range 0-127");
        if(this->wctx->drone.pitch2 >127) 
            this->wctx->error("drone pitch1 must be in the range 0-127");
        if(this->wctx->drone.vel2 >127) 
            this->wctx->error("drone vel1 must be in the range 0-127");
        done = 1;
    }
    else
    if(strcmp(command, "beat") == 0)
    {
        AbcParser::Skipspace(&p);
        this->wctx->loudnote = AbcParser::Readnump(&p);
        AbcParser::Skipspace(&p);
        this->wctx->mednote = AbcParser::Readnump(&p);
        AbcParser::Skipspace(&p);
        this->wctx->softnote = AbcParser::Readnump(&p);
        AbcParser::Skipspace(&p);
        this->wctx->beat = AbcParser::Readnump(&p);
        if(this->wctx->beat == 0)
            this->wctx->beat = this->wctx->barsize;
        done = 1;
    }
    else 
    if(strcmp(command, "beatmod") == 0) 
    {
        AbcParser::Skipspace(&p);
        this->wctx->velocity_increment = AbcParser::Readsnump(&p);
        this->wctx->loudnote += this->wctx->velocity_increment;
        this->wctx->mednote  += this->wctx->velocity_increment;
        this->wctx->softnote += this->wctx->velocity_increment;
        if(this->wctx->loudnote > 127) 
            this->wctx->loudnote = 127;
        else
        if(this->wctx->loudnote < 0) 
            this->wctx->loudnote = 0;

        if(this->wctx->mednote > 127) 
            this->wctx->mednote = 127;
        else
        if(this->wctx->mednote < 0) 
            this->wctx->mednote = 0;

        if(this->wctx->softnote > 127) 
            this->wctx->softnote = 127;
        else
        if(this->wctx->softnote < 0) 
            this->wctx->softnote = 0;
        done = 1;
    }
    else 
    if(strcmp(command, "beatstring") == 0) 
    {
        AbcParser::Skipspace(&p);
        int count = 0;
        while((count < 99) && (strchr("fFmMpP", *p) != NULL)) 
        {
            this->wctx->beatstring[count++] = *p;
            p = p + 1;
        }
        this->wctx->beatstring[count] = '\0';
        if(strlen(this->wctx->beatstring) == 0) 
            this->wctx->error("beatstring expecting string of 'f', 'm' and 'p'");
        this->wctx->nbeats = strlen(this->wctx->beatstring);
        done = 1;
    }
    else 
    if(strcmp(command, "control") == 0)
    {
        int chan;
        unsigned char data[20];
        p = this->select_channel(&chan, p);
        int n = 0;
        while ((n<20) && (*p >= '0') && (*p <= '9')) 
        {
            int datum = AbcParser::Readnump(&p);
            if (datum > 127) 
            {
                this->wctx->error("data must be in the range 0 - 127");
                datum = 0;
            }
            data[n] = datum;
            n = n + 1;
            AbcParser::Skipspace(&p);
        }
        this->midi->writeMidiEvent(0, MidiEvent::control_change, 
                                        chan, data, n);
        this->wctx->controldefaults[(int) data[0]] = (int) data[1];
        done = 1;
    }
    else 
    if(strcmp(command, "controlstring") == 0)
    {
        if(!this->wctx->controlcombo) 
        {
            for(i=0;i<AbcMidiTrackCtx::MAXLAYERS;i++) 
                this->wctx->controlnvals[i] = 0;
            this->wctx->layerIndex = 0;  /* overwrite layer 0 if not a combo */
        }
        if(this->wctx->layerIndex >= AbcMidiTrackCtx::MAXLAYERS) 
            this->wctx->error("too many combos for control data");
        else 
        {
            i = 0;
            while(i<256)
            {
                this->wctx->controldata[this->wctx->layerIndex][i] = AbcParser::Readsnump(&p);
                AbcParser::Skipspace(&p);
                i = i + 1;
                if(*p == 0) 
                    break;
            }
            this->wctx->controlnvals[this->wctx->layerIndex] = i;
            /* [SS] 2015-08-23 */
            if(this->wctx->controlnvals[this->wctx->layerIndex] < 2) 
                this->wctx->error("empty %%MIDI controlstring"); 
            this->wctx->controlcombo = 0; /* turn off controlcombo */
            done = 1;
        }
    }
    else 
    if(strcmp(command, "controlcombo") == 0)
    {
        this->wctx->controlcombo = 1;
        this->wctx->layerIndex++;
        done = 1;
    }
    else 
    if(strcmp(command, "beataccents") == 0) 
    {
        this->wctx->beataccents = 1;
        this->beatmodel = 0;
        done = 1;
    }
    else 
    if(strcmp(command, "nobeataccents") == 0)
    {
        this->wctx->beataccents = 0;
        done = 1;
    }
    else 
    if(strcmp(command,"portamento") == 0)
    {
        int chan, datum;
        unsigned char data[2];
        p = this->select_channel(&chan, p);
        data[0] = 65;
        data[1] = 127;
        /* turn portamento on */
        this->midi->writeMidiEvent(0, 
            MidiEvent::control_change, chan, data, 2);
        data[0] = 5; /* coarse portamento */
        datum = AbcParser::Readnump(&p);
        if(datum > 63) 
        {
            this->wctx->error("data must be in the range 0 - 63");
            datum = 0;
        }
        data[1] = datum;
        this->midi->writeMidiEvent(0, 
            MidiEvent::control_change, chan, data, 2);
        done = 1;
    } 
    else 
    if(strcmp(command,"noportamento") == 0) 
    {
        int chan;
        unsigned char data[2];
        p = this->select_channel(&chan, p);
        data[0] = 65;
        data[1] = 0;
        /* turn portamento off */
        this->midi->writeMidiEvent(0, 
            MidiEvent::control_change, chan, data, 2);
        done = 1;
    }
    else 
    if(strcmp(command, "pitchbend") == 0) 
    {
        int chan;
        unsigned char data[2];
        p = this->select_channel(&chan, p);
        int n = 0;
        data[0] = 0;
        data[1] = 0;
        while ((n<2) && (*p >= '0') && (*p <= '9')) 
        {
            int datum = AbcParser::Readnump(&p);
            if (datum > 255) 
            {
                this->wctx->error("data must be in the range 0 - 255");
                datum = 0;
            }
            data[n] = datum;
            n = n + 1;
            AbcParser::Skipspace(&p);
        }
        /* don't write pitchbend in the header track [SS] 2005-04-02 */
        if(noteson) 
        {
            this->midi->writeMidiEvent(0, 
                MidiEvent::control_change, chan, data, 2);
            this->wctx->tracklen += this->wctx->delta_time;
            this->wctx->delta_time = 0L;
        }
        done = 1;
    }
    else 
    if(strcmp(command, "snt") == 0)
    {
        /*single note tuning */
        int midikey = AbcParser::Readnump(&p);
        float midipitch;
        sscanf(p, "%f", &midipitch);
        this->midi->singleNoteTuningChange(midikey,  midipitch);
        done = 1;
    }
    else 
    if(strcmp(command,"chordattack") == 0) 
    {
        this->wctx->staticnotedelay = AbcParser::Readnump(&p);
        this->wctx->notedelay = this->wctx->staticnotedelay;
        done = 1;
    }
    else 
    if(strcmp(command,"randomchordattack") == 0) 
    {
        this->wctx->staticchordattack = AbcParser::Readnump(&p);
        this->wctx->chordattack = this->wctx->staticchordattack;
        done = 1;
    }
    else 
    if(strcmp(command,"drummap") == 0) 
    {
        this->parse_drummap(&p);
        done = 1;
    }
    else 
    if(strcmp(command,"stressmodel") == 0) 
    {
        if(this->barflymode == 0) 
            fprintf(stderr, "**warning stressmodel is ignored without -BF runtime option\n");
        done = 1;
    }
    else 
    if(strcmp(command,"volinc") == 0) 
    {
        this->wctx->single_velocity_inc = AbcParser::Readsnump(&p);
        done = 1;
    }
    else 
    if (strcmp(command,"vol") == 0) 
    {
        this->wctx->single_velocity = AbcParser::Readnump(&p);
        done = 1;
    }
    if (done == 0) 
    {
        char errmsg[80];
        sprintf(errmsg, "%%%%MIDI command \"%s\" not recognized",command);
        this->wctx->error(errmsg);
    }
    if((this->wctx->wordson + noteson +
       this->wctx->gchordson + this->wctx->drumson+this->wctx->droneon) == 0) 
        this->wctx->delta_time = 0L;
}

/* parse abc note and advance character pointer 
 * code stolen from parseabc.c and simplified 
 */
void 
AbcGenMidi::parse_drummap(char const **s)
{
    int octave;
    int midipitch;
    int mapto;
    char msg[80];
    char const *anoctave = "cdefgab";
    int scale[7] = {0, 2, 4, 5, 7, 9, 11};

    int mult = 1;
    char accidental = ' ';
    char note = ' ';
    /* read accidental */
    switch(**s) 
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
                snprintf(msg, sizeof(msg), "Bad pitch specifier , after note %c", note);
                this->wctx->error(msg);
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
                    snprintf(msg, sizeof(msg), "Bad pitch specifier ' after note %c", 
                        note + 'A' - 'a');
                    this->wctx->error(msg);
                    octave = octave + 1;
                    *s = *s + 1;
                }
            }
        }
    }
    /*printf("note = %d octave = %d accidental = %d\n",note,octave,accidental);*/
    midipitch = (int) (strchr(anoctave, note) - anoctave);
    if(midipitch <0 || midipitch > 6) 
    {
        this->wctx->error("Malformed note in drummap : expecting a-g or A-G");
        return;
    } 
    midipitch = scale[midipitch];
    if(accidental == '^') 
        midipitch += mult;
    if(accidental == '_') midipitch -= mult;
        midipitch = midipitch + 12*octave + 60;
    AbcParser::Skipspace(s);
    mapto = AbcParser::Readnump(s);
    if(mapto == 0) 
    {
        this->wctx->error("Bad drummap: expecting note followed by space and number");
        return;
    }
    if(mapto < 35 || mapto > 81) 
        this->wctx->warning("drummap destination should be between 35 and 81 inclusive");
    /*fprintf(stderr, "midipitch = %d map to %d \n",midipitch,mapto);*/ 
    this->wctx->drum_map[midipitch] = mapto;
}

/* used by dodeferred() to set channel to be used */
/* reads 'bass', 'chord' or nothing from string pointed to by p */
char const *
AbcGenMidi::select_channel(int *chan, char const *s)
{
    char sel[40];
    char const *p = s;
    AbcParser::Skipspace(&p);
    *chan = this->wctx->channel;
    if(isalpha(*p)) 
    {
        AbcParser::Readstr(sel, &p, 40);
        AbcParser::Skipspace(&p);
        if (strcmp(sel, "bass") == 0) 
            *chan = this->wctx->fun.chan;
        if (strcmp(sel, "chord") == 0)
            *chan = this->wctx->gchord.chan;
    }
    return p;
}

/* if shortarray = {21,-40} and factor = 4
 * longarray will be {5,6,5,5,-10,-10,-10,-10}
 * It is used for smoothing a bendstring.
 */
void 
AbcGenMidi::expand_array(int shortarray[], int size, int longarray[], int factor)
{
    if(size*factor > 256) 
    {
        this->wctx->error("not enough room in bendstring");
        return;
    }
    int j = 0;
    for(int i = 0; i< size; i++) 
    {
        float increment = (float) shortarray[i]/factor;
        float accumulator = 0.f;
        float last_accumulator = 0.f;
        for (int k = 0; k < factor; k++) 
        {
            accumulator += increment;
            if(increment > 0)
                longarray[j] = (int) (accumulator + 0.5) - (int) (last_accumulator + 0.5);
            else 
                longarray[j] = (int) (accumulator - 0.5) - (int) (last_accumulator - 0.5);
            last_accumulator = accumulator;
            j++;
        }
    }
}

/* creates a list of notes to played as chord for
 * a specific guitar chord. Most of the code figures out
 * how to order the notes when inversions are encountered.
*/
void
AbcGenMidi::configure_gchord()
{
    int inchord = 0;
    int chordindex = this->wctx->chordnum-1; // chordnum isn't an index
    Abc::Chord &chord = this->initState->chords[chordindex];
    if(this->wctx->inversion != -1) 
    {
        /* try to match inversion with basepitch+chordnotes.. */
        for(int j=0; j<chord.notes.size(); j++) 
        {
            if((this->wctx->basepitch + chord.notes[j]) % 12 == 
               (this->wctx->inversion % 12))
                inchord = j;
        }

        /* do not add strange note to chord */
        /*  if ((inchord == 0) && (inversion > basepitch)) 
        **  {
        **      inversion = inversion - 12;
        **      gchordnotes[gchordnotes_size] = inversion+gchord.base;
        **      gchordnotes_size++;
        **   }
        ***/
    }
    int note;
    this->wctx->gchordnotes_size = 0;
    for(int j=0; j<chord.notes.size(); j++) 
    {
        note = this->wctx->basepitch + chord.notes[j]; 
        if(j < inchord) 
            note += 12;
        this->wctx->gchordnotes[this->wctx->gchordnotes_size] = this->wctx->gchord.base+note;
        this->wctx->gchordnotes_size++;
    }
}

/* write 'change program' (new instrument) command to MIDI file */
void 
AbcGenMidi::write_program(int p, int channel)
{
    // fprintf(stderr, "AbcGenMidi: write_program %d %d\n", p, channel);
    unsigned char data[1];
    p = p - this->initState->programbase;
    if (p <0) 
        p = 0;
    data[0] = p;
    if(channel >= AbcMidiTrackCtx::MAXCHANS) 
        this->wctx->error("Channel limit exceeded\n");
    else 
    {
        this->midi->writeMidiEvent(this->wctx->delta_time,
            MidiEvent::program_chng, channel, data, 1);
    }
    this->wctx->tracklen += this->wctx->delta_time;
    this->wctx->delta_time = 0L;
}

void
AbcGenMidi::start_drone()
{
    int delta = this->wctx->delta_time - this->wctx->drone.event;    
    if(this->wctx->drone.event == 0)  
        this->write_program(this->wctx->drone.prog, this->wctx->drone.chan);
    this->midi_noteon(delta, 
        this->wctx->drone.pitch1+this->wctx->global_transpose,   
        8192, this->wctx->drone.chan, this->wctx->drone.vel1);
    this->midi_noteon(delta,
        this->wctx->drone.pitch2+this->wctx->global_transpose,
        8192, this->wctx->drone.chan, this->wctx->drone.vel2);
    this->wctx->drone.event = this->wctx->delta_time;
}

void
AbcGenMidi::stop_drone()
{
    int delta = this->wctx->delta_time - this->wctx->drone.event;
    this->midi_noteoff(delta, 
        this->wctx->drone.pitch1 + this->wctx->global_transpose, 
        this->wctx->drone.chan);
    this->midi_noteoff(0,
        this->wctx->drone.pitch2 + this->wctx->global_transpose,
        this->wctx->drone.chan);
    this->wctx->drone.event = this->wctx->delta_time;
}

void
AbcGenMidi::pedal_on()
{
    unsigned char data[2];
    data[0] = 64;
    data[1] = 127;
    this->midi->writeMidiEvent(0, MidiEvent::control_change, 
        this->wctx->channel, data, 2);
}

void
AbcGenMidi::pedal_off()
{
    unsigned char data[2];
    data[0] = 64;
    data[1] = 0;
    this->midi->writeMidiEvent(0, MidiEvent::control_change, 
        this->wctx->channel, data, 2);
}

/*----------------------------------------------------------------------------*/
void 
AbcGenMidi::stress_factors(int n, int *vel) 
{
    if(this->beatmodel == 2) 
       *vel = this->initState->featurelist[n].stressvelocity;
    else
        this->articulated_stress_factors(n, vel);
} 


/* computes the Phil Taylor stress factors for a note
   positioned between begnum/begden and endnum/endden.
   The segment size is resnum/resden.
   Method compute the segments that are overlapped by
   the note and average the segments parameters.
*/
void 
AbcGenMidi::articulated_stress_factors(int n,  int *vel)
{
    Abc::FeatureDesc &fd = this->initState->featurelist[n];

    int stepnum = fd.num;
    int stepden = fd.denom;

    /* undo the b_num/b_denom application in addunits() */
    /* note b_num/b_denom defined in set_meter() has nothing to do
    with L: unit length */
    int begnum = this->wctx->bar_num*this->wctx->b_denom;
    int begden = this->wctx->bar_denom*this->wctx->b_num;

    int endnum =  begnum*stepden + begden*stepnum;
    int endden =  stepden*begden;
    
    AbcMusic::reduceFraction(&endnum, &endden);

    /* determine the segment number by dividing by the segment size
    * and truncating the result.
    * firstseg = integer (begnum/begden divided by segnum/segden) */
    int firstsegnum = begnum*segden;
    int firstsegden = begden*segnum*4;
    AbcMusic::reduceFraction(&firstsegnum, &firstsegden);

    /* note coordinates are in quarter note units so divide by 4 */
    int firstseg = firstsegnum/firstsegden;
    int firstsegrem = firstsegnum % firstsegden;

    /* lastseg = integer (endnum/endden divided by resnum/resden) */
    int lastsegnum = endnum*segden;
    int lastsegden = endden*segnum*4;
    AbcMusic::reduceFraction(&lastsegnum, &lastsegden);
    /* scale down the fraction endnum/endden so that we do avoid the
        next segment unless endnum/endden is large enough */
    int lastseg = lastsegnum/lastsegden;
    int lastsegrem = lastsegnum%lastsegden;
    if(lastseg > nseg) return; /* do nothing if note extends beyond bar */

    if(lastseg - firstseg > 2) return; /* do nothing if note extends over 3 segments */ 

    // int gain;
    // float segsize;
    // int tnotenum,tnotedenom;

    float dur = 0.f;
    float segrange = 0.f, segsize;
    /* now that we know the segment span of the notes average fdur 
    * over those segments. [SS] 2011-08-25 */
    if(lastseg == firstseg) 
    { 
        /*note is included is entirely included in segment*/
        dur = fdur[firstseg];
    } 
    else 
    {  
        /* note may overlap more than one segment */
        for(int i=firstseg;i<lastseg;i++) 
        {
            if(i == firstseg) 
            { 
                /* for the first segment */
                if(firstsegrem == 0) 
                {
                    /* note starts at beginning of segment*/
                    dur = fdur[i];
                    segsize = 1.f;
                    segrange = segsize;
                }
                else 
                { 
                    /* note starts in the middle of segment */
                    segsize = (firstsegden - firstsegrem) / (float) firstsegden;
                    segrange = segsize; 
                    dur = this->fdur[i] * segsize;
                }
            } 
            else 
            {
                dur = dur + fdur[i]; /* for other segments that note overlaps */
                segrange += 1.f;  
            }
        }

        if(lastsegrem != 0) 
        {
            segsize = lastsegrem / (float) lastsegden;
            dur = dur +  fdur[lastsegrem] *  segsize;
            segrange = segrange + segsize;
        }
        dur = dur/segrange;
    } /* end of note may overlap more than one segment */

    /* gain is set to the value of ngain[] in the first segment.*/
    float gain = this->ngain[firstseg];
    dur = dur/maxdur;
    /* since we can't lengthen notes we shorten them based on the maximum*/

    if(this->initState->verbose > 1) 
    {
        char msg[100];
        snprintf(msg, 100, "%d %d/%d = %d/%d to  %d/%d = %d/%d",
            fd.pitch, begnum, begden, firstsegnum, firstsegden,
            endnum, endden, lastsegnum, lastsegden);
        this->wctx->log(msg);
        snprintf(msg, 100, " dur gain = %g %g", dur, gain);
        this->wctx->log(msg);
    }
    /* tnotenum and tnotedenom is used for debugging only.
        int tnotenum = (int) (0.5 +dur*100.0);
        int tnotedenom = 100;
    */

    *vel = gain;
    /* compute the trim values that are applied and the end of the NOTE:
    * block in the writetrack() switch complex.*/
    this->wctx->trim_num = (int) (fd.num*100.f*(1.f - dur));
    this->wctx->trim_denom = (int) (fd.denom * 100.f); 
    /*printf("dur = %f %d/%d %d/%d gain = %d\n",dur,tnotenum,tnotedenom,trim_num,trim_denom,gain);*/
}

/* Allocate channels for in voice chords containing microtones.
 * n is the number of channels to allocate which should be
 * less than 10.
 */
int 
AbcGenMidi::makechordchannels(int n)
{
    /* do not make chord channels for track 0 if multi track */
    if(this->ntracks != 1 && this->wctx->tracknumber == 0) 
        return 0; 
    if(n < 1) 
        return -1;
    if(n > 9) 
        n = 9;
    int prog = this->wctx->current_program[this->wctx->channel];
    /* save active channel number */
    this->wctx->chordchannels[0] = this->wctx->channel; 
    if(this->initState->verbose >1) 
    {
        char msg[64];
        snprintf(msg, 64, "making %d chord channels", n);
        this->wctx->log(msg);
    }
    for (int i=1; i<=n; i++) 
    {
        int chan = this->findchannel();
        this->wctx->chordchannels[i] = chan;
        this->write_program(prog, chan);
    }
    this->wctx->nchordchannels = n;
    return n;
}