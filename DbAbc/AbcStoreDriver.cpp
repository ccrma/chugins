#include "AbcStore.h"

/* called at the beginning of an abc tune via this->refno */
/* This sets up all the default values */
void
AbcStore::startfile()
{
    int j;
    if(this->verbose)
        this->log("scanning tune\n");

    /* set up defaults */
    this->keySharps = 0;
    this->keyMinor = 0;
    this->tuplecount = 0;
    this->specialtuple = 0;
    for(j=0; j<sizeof(this->dependent_voice)/sizeof(int); j++) 
        this->dependent_voice[j] = 0;

    this->global.initmic();
    this->setmap(0, this->global.basemap, this->global.basemul);
    this->copymap(&global);
    this->global.octaveshift = 0;
    this->global.keyset = 0;
    this->voicecount = 0;
    this->head = nullptr;
    for(j=0;j<64;j++) 
        this->vaddr[j] = nullptr; 
    this->v = nullptr;
    this->got_titlename = 0;
    this->time_num = 4;
    this->time_denom = 4;
    this->mtime_num = 4;
    this->mtime_denom = 4;
    this->timesigset = 0;
    this->barchecking = 1;
    this->global.default_length = -1;
    this->tempo(this->default_tempo, 1, 4, 0, nullptr, nullptr);
    this->nextFeature = 0;
    // this->atext is currently sized to this->maxtexts, do not clear!
    this->ntexts = 0;
    
    this->wordlist.clear();
    this->gfact_num = 1;
    this->gfact_denom = 4;
    this->hornpipe = 0;
    this->karaoke = 0;
    this->numsplits = 0;
    this->splitdepth = 0;
    this->v1index = -1;
    this->propagate_accidentals = this->default_retain_accidentals;
    this->fermata_fixed = this->default_fermata_fixed;
    if(this->ratio_standard == -1) 
    {
        this->ratio_a = this->default_ratio_a;
        this->ratio_b = this->default_ratio_b;
    } 
    else 
    {
        /* use celtic ratio for swing */
        this->ratio_a = 2;
        this->ratio_b = 4;
    }
    this->wcount = 0;
    std::string logPrefix = this->getContext(false);
    this->genMidi.Init(this->performing, logPrefix.c_str());
    this->genMidi.set_gchords("x");
    this->genMidi.set_drums("z");

    this->middle_c = this->default_middle_c;
    this->parser->temperament = TEMPERNORMAL;
    this->headerpartlabel = 0;

    this->gchordvoice = 0;
    this->drumvoice = 0;
    this->dronevoice = 0;
    this->wordvoice = 0;
    this->notesdefined = 0;
    this->rhythmdesignator[0] = '\0';
}

/* end of _tune_ has been reached - write out MIDI file */
void 
AbcStore::finishfile()
{
    this->complete_all_split_voices();

    /* dump_voicecontexts(); for debugging*/
    // cbb: trackdescriptors reside in GenMidi, setup_trackstructure
    // resides in AbcStore and needs GenMidi::findvoice
    this->setup_trackstructure(); // from voice contexts
    this->clearvoicecontexts();

    /* init_drum_map(); [SS] 2017-12-10 moved to main() */
    if(!this->pastheader) 
    {
        this->error("No valid K: field found at start of tune");
    } 
    else 
    {
        this->scan_for_missing_repeats();
        if(this->genMidi.parts > -1) 
            this->addfeature(Abc::PART, ' ', 0, 0);

        if(this->headerpartlabel == 1 && !this->silent) 
            this->error("P: field in header should go after K: field");

        if(this->verbose > 1)
            this->log("handling grace notes");

        this->dograce(); // fixup _all_ gracenotes
        if(this->genMidi.barflymode) 
            this->apply_bf_stress_factors();
        this->tiefix(); /* [SS] 2014-04-03 */
        if((this->genMidi.parts == -1) && (this->voicecount == 1)) 
        {
            if(verbose > 1) 
                this->log("fixing repeats");
            this->fixreps();
        }

        this->expand_ornaments();
        this->check_for_timesig_preceding_bar_line();
        this->no_more_free_channels = 0;
        if(this->genMidi.parts >= 0) 
            this->fix_part_start();
        if(this->verbose > 5) 
            this->dumpfeat(0, this->nextFeature-1);

        this->initState = new Abc::InitState(this->nextFeature, 
                                this->featurelist,
                                this->atext,
                                this->chords,
                                this->wordlist);
        this->initState->verbose = this->verbose;
        this->initState->silent = this->silent;
        this->initState->quiet = this->quiet;
        this->initState->programbase = this->programbase;
        this->initState->voicesused = this->voicesused;
        this->initState->tempo = this->current_tempo;
        this->initState->time_num = this->time_num;
        this->initState->time_denom = this->time_denom;
        this->initState->mtime_num = this->mtime_num;
        this->initState->mtime_denom = this->mtime_denom;
        this->initState->keySharps = this->keySharps;
        this->initState->keyMinor = this->keyMinor;
        this->initState->karaoke = this->karaoke;
        this->initState->wcount = this->wcount;
        this->initState->retuning = this->retuning;
        this->initState->bend = this->bend;
        this->initState->dependent_voice = this->dependent_voice;
        this->initState->barchecking = this->barchecking;
        this->initState->lineno = this->parser->lineno;

        if(this->performing)
        {
            if(this->verbose)
                this->log("Ready to perform");
            this->genMidi.beginPerformance(this->initState);
            // client must call Cleanup or destroy AbcStore
        }
        else
        {
            this->genMidi.writefile(this->outname.c_str(), this->initState);
            this->Cleanup();
        }
    }
}

void
AbcStore::Cleanup()
{
    this->atext.clear();
    this->wordlist.clear();
    this->genMidi.partspec.clear();
    this->chords.clear();
    this->free_notestructs();
    delete this->initState;
    this->initState = nullptr;
}

void 
AbcStore::dump_trackdescriptor()
{
    char msg[100];
    this->log("tracks {");
    for(int i=0;i<this->genMidi.ntracks;i++) 
    {
        AbcGenMidi::Track &track = this->genMidi.trackdescriptor[i];
        snprintf(msg, 100, " i:%d tt:%d v:%d midi:%d @ r:%d->%d",
            i, track.tracktype, track.voicenum, track.midichannel,
            track.featureIndexBegin, track.featureIndexEnd);
        this->log(msg);
    }
    this->log("tracks }");
}

void 
AbcStore::setup_trackstructure() 
{
    AbcGenMidi::Track *td = this->genMidi.trackdescriptor;
    td[0].tracktype = AbcGenMidi::NOTES;
    td[0].voicenum = 1; // each voice can have gchords
    td[0].midichannel = -1; // unspecified, 

    voicecontext *p = this->head;
    voicecontext *q;

    // for multi-voice (including gchords, etc, the first track is for tempo)
    // below we reset tracks to 1 in the special single track case.
    this->genMidi.ntracks = 1; 
    while (p != nullptr) 
    {
        if(this->verbose) 
        {
            char msg[100];
            snprintf(msg, 100, 
                "num %d index %d bars %d "
                "gchords %d words %d drums %d "
                "drone %d tosplit %d fromsplit %d ",
                p->voiceno, p->indexno, p->nbars,
                p->hasgchords, p->haswords, p->hasdrums,
                p->hasdrone, p->tosplitno, p->fromsplitno);
            this->log(msg);
        }
        if(this->genMidi.ntracks > AbcMidiTrackCtx::MAXTRACKS)
        {
           this->error("Too many tracks");
           return;
        }
        td[this->genMidi.ntracks].tracktype = AbcGenMidi::NOTES;
        td[this->genMidi.ntracks].voicenum = p->indexno;
        td[this->genMidi.ntracks].midichannel = p->midichannel; // -1 means unspecified (ok)
        if(p->haswords)
        {
            if(!this->separate_tracks_for_words) 
            {
                td[this->genMidi.ntracks].tracktype = AbcGenMidi::NOTEWORDS;
                td[this->genMidi.ntracks].voicenum = p->indexno;
            } 
            else 
            {
                this->genMidi.ntracks++;
                td[this->genMidi.ntracks].tracktype = AbcGenMidi::WORDS;
                td[this->genMidi.ntracks].voicenum = td[this->genMidi.ntracks-1].voicenum;
            }
        }
        if(p->hasgchords)
        {
            this->genMidi.ntracks++;
            td[this->genMidi.ntracks].tracktype = AbcGenMidi::GCHORDS;
            td[this->genMidi.ntracks].voicenum = p->indexno;
        }
        if(p->hasdrums)
        {
            this->genMidi.ntracks++;
            td[this->genMidi.ntracks].tracktype = AbcGenMidi::DRUMS;
            td[this->genMidi.ntracks].voicenum = p->indexno;
        }
        if(p->hasdrone) 
        {
            this->genMidi.ntracks++;  
            td[this->genMidi.ntracks].tracktype = AbcGenMidi::DRONE;
            td[this->genMidi.ntracks].voicenum = p->indexno;
        }
        this->genMidi.ntracks++;
        q = p->next;
        p = q;
    }

    /* does the tune need any gchord, drum, drone or word track */
    if((this->voicesused == false) && (!this->karaoke) && 
       (this->gchordvoice == 0) && (this->drumvoice == 0) && (this->dronevoice==0)) 
    {
        this->genMidi.ntracks = 1;
    } 
    if(this->verbose > 1)
        this->dump_trackdescriptor();
}

