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
    this->wordlist.clear();
    this->ntexts = 0;
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
    this->genMidi.Init();
    this->genMidi.set_gchords("x");
    this->genMidi.set_drums("z");

    this->middle_c = this->default_middle_c;
    this->parser->temperament = TEMPERNORMAL;
    this->headerpartlabel = 0;

    this->gchordvoice = 0;
    this->drumvoice = 0;
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
            this->log("handling grace notes\n");

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
            this->dumpfeat(0, this->nextFeature);

        AbcGenMidi::InitState initState(this->nextFeature, 
                                        this->featurelist,
                                        this->atext,
                                        this->chords,
                                        this->wordlist);
        initState.verbose = this->verbose;
        initState.silent = this->silent;
        initState.quiet = this->quiet;
        initState.programbase = this->programbase;
        initState.ntracks = this->ntracks;
        initState.voicesused = this->voicesused;
        initState.tempo = this->current_tempo;
        initState.time_num = this->time_num;
        initState.time_denom = this->time_denom;
        initState.mtime_num = this->mtime_num;
        initState.mtime_denom = this->mtime_denom;
        initState.keySharps = this->keySharps;
        initState.keyMinor = this->keyMinor;
        initState.karaoke = this->karaoke;
        initState.wcount = this->wcount;
        initState.retuning = this->retuning;
        initState.bend = this->bend;
        initState.dependent_voice = this->dependent_voice;
        initState.barchecking = this->barchecking;

        this->genMidi.writefile(this->outname.c_str(), &initState);

        this->atext.clear();
        this->wordlist.clear();
        this->genMidi.partspec.clear();
        this->chords.clear();
        this->free_notestructs();
    }
}

