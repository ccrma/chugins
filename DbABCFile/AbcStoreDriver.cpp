#include "AbcStore.h"

/* called at the beginning of an abc tune via this->refno */
/* This sets up all the default values */
void
AbcStore::startfile()
{
    int j;
    if(this->verbose)
        this->info("scanning tune\n");

    /* set up defaults */
    this->sf = 0;
    this->mi = 0;
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
    this->notes = 0;
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
    this->setup_trackstructure();
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
            this->info("handling grace notes\n");

        this->dograce(); // fixup _all_ gracenotes
        if(this->genMidi.barflymode) 
            this->apply_bf_stress_factors();
        this->tiefix(); /* [SS] 2014-04-03 */
        if((this->genMidi.parts == -1) && (this->voicecount == 1)) 
        {
            if(verbose > 1) 
                this->info("fixing repeats");
            this->fixreps();
        }

        this->expand_ornaments();
        this->check_for_timesig_preceding_bar_line();
        this->no_more_free_channels = 0;
        if(this->genMidi.parts >= 0) 
            this->fix_part_start();
        if(this->verbose > 5) 
            this->dumpfeat(0, this->notes);

        if(this->check) 
        {
            Mf_putc = nullputc;
            header_time_num = time_num;
            header_time_denom = time_denom;
            Mf_numbyteswritten = 0;
            if(ntracks == 1) 
            {
                this->writetrack(0);
            } 
            else 
            {
                if(!done_with_barloc) 
                {
                    diaghandle = fopen("barloc.txt","w");
                    for(i=0; i<ntracks; i++) 
                    {
                        this->writetrack(i);
                        this->dump_barloc(diaghandle,i);
                    }
                    fclose(diaghandle);
                    done_with_barloc = 1;
                } 
                else 
                {  
                    /* done_with_barloc == 1 */
                    for (i=0; i<ntracks; i++) 
                    {
                        this->writetrack(i);
                    }	
                }
            } /* more than 1 track */
        } 
        else 
        {    
            /* check != 0 */
            if((fp = fopen(outname, "wb")) == NULL) 
            {
                this->error("File open failed");
                return;
            }
            if (!silent) 
                printf("writing MIDI file %s\n", outname);
            Mf_putc = myputc;
            Mf_writetrack = writetrack;
            header_time_num = time_num;
            header_time_denom = time_denom;
            if(ntracks == 1) 
            {
                this->mfwrite(0, 1, division, fp);
            } 
            else 
            {
                this->mfwrite(1, ntracks, division, fp);
            }
            fclose(fp);
            #ifdef __MACINTOSH__
            (void) setOutFileCreator(outname,'Midi','ttxt');
            #endif /* __MACINTOSH__ */

        }
        this->atext.clear();
        this->wordlist.clear();
        this->genMidi.partspec.clear();
        this->free_notestructs();
    }
}