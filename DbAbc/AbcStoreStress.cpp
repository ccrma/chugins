#include "AbcStore.h"
#include <algorithm> // std::max
#include <cstring>

void
AbcStore::init_stresspat()
{
    if (this->stresspat[0].name.size() != 0) 
    {
        this->error("stresspat already initialized\n");
        return;
    } 
    stresspat[0].name = "Hornpipe";
    stresspat[0].meter = "4/4";
    stresspat[0].nseg = 8;
    stresspat[0].nval = 2;
    stresspat[0].vel[0] = 110;
    stresspat[0].vel[1] = 90;
    stresspat[0].expcoef[0] = 1.4f;
    stresspat[0].expcoef[1] = 0.6f;

    stresspat[1].name = "Hornpipe";
    stresspat[1].meter = "C|";
    stresspat[1].nseg = 8;
    stresspat[1].nval = 2;
    stresspat[1].vel[0] = 110;
    stresspat[1].vel[1] = 90;
    stresspat[1].expcoef[0] = 1.4;
    stresspat[1].expcoef[1] = 0.6;

    stresspat[2].name = "Hornpipe";
    stresspat[2].meter = "2/4";
    stresspat[2].nseg = 8;
    stresspat[2].nval = 2;
    stresspat[2].vel[0] = 110;
    stresspat[2].vel[1] = 90;
    stresspat[2].expcoef[0] = 1.4;
    stresspat[2].expcoef[1] = 0.6;

    stresspat[3].name = "Hornpipe";
    stresspat[3].meter = "9/4";
    stresspat[3].nseg = 9;
    stresspat[3].nval = 3;
    stresspat[3].vel[0] = 110;
    stresspat[3].vel[1] = 80;
    stresspat[3].vel[2] = 100;
    stresspat[3].expcoef[0] = 1.4;
    stresspat[3].expcoef[1] = 0.6;
    stresspat[3].expcoef[2] = 1.0;

    stresspat[4].name = "Hornpipe";
    stresspat[4].meter = "3/2";
    stresspat[4].nseg = 12;
    stresspat[4].nval = 2;
    stresspat[4].vel[0] = 110;
    stresspat[4].vel[1] = 90;
    stresspat[4].expcoef[0] = 1.4;
    stresspat[4].expcoef[1] = 0.6;

    stresspat[5].name = "Hornpipe";
    stresspat[5].meter = "12/8";
    stresspat[5].nseg = 12;
    stresspat[5].nval = 3;
    stresspat[5].vel[0] = 110;
    stresspat[5].vel[1] = 80;
    stresspat[5].vel[2] = 100;
    stresspat[5].expcoef[0] = 1.4;
    stresspat[5].expcoef[1] = 0.6;
    stresspat[5].expcoef[2] = 1.0;

    stresspat[6].name = "Double hornpipe";
    stresspat[6].meter = "6/2";
    stresspat[6].nseg = 12;
    stresspat[6].nval = 6;
    stresspat[6].vel[0] = 110;
    stresspat[6].vel[1] = 80;
    stresspat[6].vel[2] = 80;
    stresspat[6].vel[3] = 105;
    stresspat[6].vel[4] = 80;
    stresspat[6].vel[5] = 80;
    stresspat[6].expcoef[0] = 1.2;
    stresspat[6].expcoef[1] = 0.9;
    stresspat[6].expcoef[2] = 0.9;
    stresspat[6].expcoef[3] = 1.2;
    stresspat[6].expcoef[4] = 0.9;
    stresspat[6].expcoef[5] = 0.9;

    stresspat[7].name = "Reel";
    stresspat[7].meter = "4/4";
    stresspat[7].nseg = 8;
    stresspat[7].nval = 2;
    stresspat[7].vel[0] = 120;
    stresspat[7].vel[1] = 60;
    stresspat[7].expcoef[0] = 1.1;
    stresspat[7].expcoef[1] = 0.9;

    stresspat[8].name = "Reel";
    stresspat[8].meter = "C|";
    stresspat[8].nseg = 8;
    stresspat[8].nval = 8;
    stresspat[8].vel[0] = 80;
    stresspat[8].vel[1] = 60;
    stresspat[8].vel[2] = 120;
    stresspat[8].vel[3] = 60;
    stresspat[8].vel[4] = 80;
    stresspat[8].vel[5] = 60;
    stresspat[8].vel[6] = 120;
    stresspat[8].vel[7] = 60;
    stresspat[8].expcoef[0] = 1.1;
    stresspat[8].expcoef[1] = 0.9;
    stresspat[8].expcoef[2] = 1.1;
    stresspat[8].expcoef[3] = 0.9;
    stresspat[8].expcoef[4] = 1.1;
    stresspat[8].expcoef[5] = 0.9;
    stresspat[8].expcoef[6] = 1.1;
    stresspat[8].expcoef[7] = 0.9;

    stresspat[9].name = "Reel";
    stresspat[9].meter = "2/4";
    stresspat[9].nseg = 8;
    stresspat[9].nval = 2;
    stresspat[9].vel[0] = 120;
    stresspat[9].vel[1] = 60;
    stresspat[9].expcoef[0] = 1.1;
    stresspat[9].expcoef[1] = 0.9;

    stresspat[10].name = "Slip Jig";
    stresspat[10].meter = "9/8";
    stresspat[10].nseg = 9;
    stresspat[10].nval = 3;
    stresspat[10].vel[0] = 110;
    stresspat[10].vel[1] = 70;
    stresspat[10].vel[2] = 80;
    stresspat[10].expcoef[0] = 1.4;
    stresspat[10].expcoef[1] = 0.5;
    stresspat[10].expcoef[2] = 1.1;

    stresspat[11].name = "Double Jig";
    stresspat[11].meter = "6/8";
    stresspat[11].nseg = 6;
    stresspat[11].nval = 3;
    stresspat[11].vel[0] = 110;
    stresspat[11].vel[1] = 70;
    stresspat[11].vel[2] = 80;
    stresspat[11].expcoef[0] = 1.2;
    stresspat[11].expcoef[1] = 0.7;
    stresspat[11].expcoef[2] = 1.1;

    stresspat[12].name = "Single Jig";
    stresspat[12].meter = "6/8";
    stresspat[12].nseg = 6;
    stresspat[12].nval = 3;
    stresspat[12].vel[0] = 110;
    stresspat[12].vel[1] = 80;
    stresspat[12].vel[2] = 90;
    stresspat[12].expcoef[0] = 1.2;
    stresspat[12].expcoef[1] = 0.7;
    stresspat[12].expcoef[2] = 1.1;

    stresspat[13].name = "Slide";
    stresspat[13].meter = "6/8";
    stresspat[13].nseg = 6;
    stresspat[13].nval = 3;
    stresspat[13].vel[0] = 110;
    stresspat[13].vel[1] = 80;
    stresspat[13].vel[2] = 90;
    stresspat[13].expcoef[0] = 1.3;
    stresspat[13].expcoef[1] = 0.8;
    stresspat[13].expcoef[2] = 0.9;

    stresspat[14].name = "Jig";
    stresspat[14].meter = "6/8";
    stresspat[14].nseg = 6;
    stresspat[14].nval = 3;
    stresspat[14].vel[0] = 110;
    stresspat[14].vel[1] = 80;
    stresspat[14].vel[2] = 90;
    stresspat[14].expcoef[0] = 1.2;
    stresspat[14].expcoef[1] = 0.7;
    stresspat[14].expcoef[2] = 1.1;

    stresspat[15].name = "Ragtime";
    stresspat[15].meter = "12/8";
    stresspat[15].nseg = 12;
    stresspat[15].nval = 3;
    stresspat[15].vel[0] = 110;
    stresspat[15].vel[1] = 70;
    stresspat[15].vel[2] = 90;
    stresspat[15].expcoef[0] = 1.4;
    stresspat[15].expcoef[1] = 0.6;
    stresspat[15].expcoef[2] = 1.0;

    stresspat[16].name = "Strathspey";
    stresspat[16].meter = "C";
    stresspat[16].nseg = 8;
    stresspat[16].nval = 2;
    stresspat[16].vel[0] = 120;
    stresspat[16].vel[1] = 80;
    stresspat[16].expcoef[0] = 1.0;
    stresspat[16].expcoef[1] = 1.0;

    stresspat[17].name = "Fling";
    stresspat[17].meter = "C";
    stresspat[17].nseg = 8;
    stresspat[17].nval = 2;
    stresspat[17].vel[0] = 110;
    stresspat[17].vel[1] = 90;
    stresspat[17].expcoef[0] = 1.4;
    stresspat[17].expcoef[1] = 0.6;

    stresspat[18].name = "Set Dance";
    stresspat[18].meter = "4/4";
    stresspat[18].nseg = 8;
    stresspat[18].nval = 2;
    stresspat[18].vel[0] = 110;
    stresspat[18].vel[1] = 90;
    stresspat[18].expcoef[0] = 1.4;
    stresspat[18].expcoef[1] = 0.6;

    stresspat[19].name = "Set Dance";
    stresspat[19].meter = "C|";
    stresspat[19].nseg = 8;
    stresspat[19].nval = 2;
    stresspat[19].vel[0] = 110;
    stresspat[19].vel[1] = 90;
    stresspat[19].expcoef[0] = 1.4;
    stresspat[19].expcoef[1] = 0.6;

    stresspat[20].name = "Waltz";
    stresspat[20].meter = "3/4";
    stresspat[20].nseg = 3;
    stresspat[20].nval = 3;
    stresspat[20].vel[0] = 110;
    stresspat[20].vel[1] = 70;
    stresspat[20].vel[2] = 70;
    stresspat[20].expcoef[0] = 1.04;
    stresspat[20].expcoef[1] = 0.98;
    stresspat[20].expcoef[2] = 0.98;

    stresspat[21].name = "Slow March";
    stresspat[21].meter = "C|";
    stresspat[21].nseg = 8;
    stresspat[21].nval = 3;
    stresspat[21].vel[0] = 115;
    stresspat[21].vel[1] = 85;
    stresspat[21].vel[2] = 100;
    stresspat[21].vel[3] = 85;
    stresspat[21].expcoef[0] = 1.1;
    stresspat[21].expcoef[1] = 0.9;
    stresspat[21].expcoef[2] = 1.1;
    stresspat[21].expcoef[3] = 0.9;

    stresspat[22].name = "Slow March";
    stresspat[22].meter = "C";
    stresspat[22].nseg = 8;
    stresspat[22].nval = 3;
    stresspat[22].vel[0] = 115;
    stresspat[22].vel[1] = 85;
    stresspat[22].vel[2] = 100;
    stresspat[22].vel[3] = 85;
    stresspat[22].expcoef[0] = 1.1;
    stresspat[22].expcoef[1] = 0.9;
    stresspat[22].expcoef[2] = 1.1;
    stresspat[22].expcoef[3] = 0.9;

    stresspat[23].name = "March";
    stresspat[23].meter = "C|";
    stresspat[23].nseg = 8;
    stresspat[23].nval = 2;
    stresspat[23].vel[0] = 115;
    stresspat[23].vel[1] = 85;
    stresspat[23].expcoef[0] = 1.1;
    stresspat[23].expcoef[1] = 0.9;

    stresspat[24].name = "March";
    stresspat[24].meter = "C";
    stresspat[24].nseg = 8;
    stresspat[24].nval = 4;
    stresspat[24].vel[0] = 115;
    stresspat[24].vel[1] = 85;
    stresspat[24].vel[2] = 100;
    stresspat[24].vel[3] = 85;
    stresspat[24].expcoef[0] = 1.1;
    stresspat[24].expcoef[1] = 0.9;
    stresspat[24].expcoef[2] = 1.1;
    stresspat[24].expcoef[3] = 0.9;

    stresspat[25].name = "March";
    stresspat[25].meter = "6/8";
    stresspat[25].nseg = 8;
    stresspat[25].nval = 3;
    stresspat[25].vel[0] = 110;
    stresspat[25].vel[1] = 70;
    stresspat[25].vel[2] = 80;
    stresspat[25].expcoef[0] = 1.1;
    stresspat[25].expcoef[1] = 0.95;
    stresspat[25].expcoef[2] = 0.95;

    stresspat[26].name = "March";
    stresspat[26].meter = "2/4";
    stresspat[26].nseg = 8;
    stresspat[26].nval = 2;
    stresspat[26].vel[0] = 115;
    stresspat[26].vel[1] = 85;
    stresspat[26].expcoef[0] = 1.1;
    stresspat[26].expcoef[1] = 0.9;

    stresspat[27].name = "Polka k1";
    stresspat[27].meter = "3/4";
    stresspat[27].nseg = 3;
    stresspat[27].nval = 3;
    stresspat[27].vel[0] = 90;
    stresspat[27].vel[1] = 110;
    stresspat[27].vel[2] = 100;
    stresspat[27].expcoef[0] = 0.75;
    stresspat[27].expcoef[1] = 1.25;
    stresspat[27].expcoef[2] = 1.00;

    stresspat[28].name = "Polka";
    stresspat[28].meter = "4/4";
    stresspat[28].nseg = 8;
    stresspat[28].nval = 2;
    stresspat[28].vel[0] = 110;
    stresspat[28].vel[1] = 90;
    stresspat[28].expcoef[0] = 1.4;
    stresspat[28].expcoef[1] = 0.6;

    stresspat[29].name = "saucy";
    stresspat[29].meter = "3/4";
    stresspat[29].nseg = 6;
    stresspat[29].nval = 6;
    stresspat[29].vel[0] = 115;
    stresspat[29].vel[1] = 85;
    stresspat[29].vel[2] = 120;
    stresspat[29].vel[3] = 85;
    stresspat[29].vel[4] = 115;
    stresspat[29].vel[5] = 85;
    stresspat[29].expcoef[0] = 1.2;
    stresspat[29].expcoef[1] = 0.8;
    stresspat[29].expcoef[2] = 1.3;
    stresspat[29].expcoef[3] = 0.7;
    stresspat[29].expcoef[4] = 1.1;
    stresspat[29].expcoef[5] = 0.9;

    stresspat[30].name = "Slip jig";
    stresspat[30].meter = "3/4";
    stresspat[30].nseg = 9;
    stresspat[30].nval = 3;
    stresspat[30].vel[0] = 110;
    stresspat[30].vel[1] = 80;
    stresspat[30].vel[2] = 100;
    stresspat[30].expcoef[0] = 1.3;
    stresspat[30].expcoef[1] = 0.7;
    stresspat[30].expcoef[2] = 1.0;

    stresspat[31].name = "Tango";
    stresspat[31].meter = "2/4";
    stresspat[31].nseg = 8;
    stresspat[31].nval = 4;
    stresspat[31].vel[0] = 110;
    stresspat[31].vel[1] = 90;
    stresspat[31].vel[2] = 90;
    stresspat[31].vel[3] = 100;
    stresspat[31].expcoef[0] = 1.6;
    stresspat[31].expcoef[1] = 0.8;
    stresspat[31].expcoef[2] = 0.8;
    stresspat[31].expcoef[3] = 0.8;
    this->nmodels = 32;
}

void 
AbcStore::apply_bf_stress_factors() 
{
    char msg[64];
    if(this->verbose) 
    {
        snprintf(msg, 64, "rhythmdesignator = %s\n",
            this->rhythmdesignator);
        this->log(msg);
    }
    int j = 0;
    if(this->genMidi.stress_pattern_loaded == 0) 
    {
        if(this->rhythmdesignator[0] == '\0') 
        {
            this->error("No R: in header, cannot apply Barfly model without %%MIDI ptstress");
            return;
        }
        j = this->load_stress_parameters(this->rhythmdesignator);
        if (j < 0 && this->genMidi.beatmodel == 0) 
        {
            this->error("invalid R: designator");
            return;
        }
    }
    if(this->verbose)
    {
        snprintf(msg, 64, "beat_modifer using segment size %d/%d",
            this->genMidi.segnum, this->genMidi.segden);
        this->log(msg);
    }
    char inputfile[64];
    char command[40];
    char const *p;
    int barnumber = 0;
    while(j < this->nextFeature) 
    {
        Abc::FeatureDesc &fd = this->featurelist[j];
        switch(fd.feature) 
        {
        case Abc::SINGLE_BAR:           /*  |  */
        case Abc::DOUBLE_BAR:           /*  || */
        case Abc::BAR_REP:              /*  |: */
        case Abc::REP_BAR:              /*  :| */
        case Abc::DOUBLE_REP:           /*  :: */
            barnumber++;
            if(this->verbose>1) 
            {
                snprintf(msg, 64, "bar %d", barnumber);
                this->log(msg);
            }
            if(this->genMidi.beatmodel == 2) 
                this->beat_modifier(j);
            break;
        case Abc::DYNAMIC:
            p = this->atext[fd.pitch].c_str();
            this->parser->skipspace(&p);
            this->parser->readstr(command, &p, 40);
            if(strcmp(command, "stressmodel") == 0) 
            {
                this->parser->skipspace(&p);
                this->genMidi.beatmodel = this->parser->readnump(&p);
                this->genMidi.stressmodel = this->genMidi.beatmodel;
                if(this->verbose) 
                {
                    snprintf(msg, 64, "%s %d", command, this->genMidi.beatmodel);
                    this->log(msg);
                }
                if (this->genMidi.beatmodel == 1)
                    return; /* do not apply beat_modifier */
            }
            else
            if(strcmp(command, "ptstress") == 0) 
            {
                this->parser->skipspace(&p);
                strncpy(inputfile, p ,60);
                if(this->verbose) 
                {
                    snprintf(msg, 64, "ptstress file = %s", inputfile);
                    this->log(msg);
                }
                if(this->parse_stress_params(inputfile) == -1) 
                    this->readstressfile(inputfile);
                this->calculate_stress_parameters();
                this->genMidi.beatmodel = 2;
                if(this->genMidi.stressmodel !=0 && 
                   this->genMidi.beatmodel != this->genMidi.stressmodel) 
                {
                    this->genMidi.beatmodel = this->genMidi.stressmodel;
                }
            }
            break;
        default:
            break;
        }
        j = j+1;
    }
}

int
AbcStore::stress_locator(char const *rhythmdesignator, 
                         char const *timesigstring)
{
    for(int i=0; i < this->nmodels; i++)
    {
        if(stricmp(rhythmdesignator, this->stresspat[i].name.c_str()) == 0 &&
           stricmp(timesigstring, this->stresspat[i].meter.c_str()) == 0)
        {
            return i;
        }
    }
    return -1;
}

/* load_stress_parameters  returns 0 or -1 */
int
AbcStore::load_stress_parameters(char const *rhythmdesignator)
{
    this->genMidi.maxdur = 0;
    for(int n = 0; n < 32; n++)
    {
        this->genMidi.fdur[n] = 0.0;
        this->genMidi.ngain[n] = 0;
    }
    this->genMidi.fdursum[0] = this->genMidi.fdur[0];
    if(strlen(rhythmdesignator) < 2)
    {
        this->genMidi.beatmodel = 0;
        return -1;
    }
    int index = this->stress_locator(rhythmdesignator, 
                        this->parser->timesigstring);
    if(index == -1)
    {
        char msg[100];
        snprintf(msg, 100, 
            "rhythm designator %s %s is not one of",
            rhythmdesignator, this->parser->timesigstring);
        this->warning(msg);
        for(int i = 0; i < this->nmodels; i++)
        {
            snprintf(msg, 100, "  %s %s", 
                this->stresspat[i].name.c_str(), 
                this->stresspat[i].meter.c_str());
            this->warning(msg);
        }
        this->genMidi.beatmodel = 0;
        return -1;
    }

    if (this->genMidi.stressmodel == 0)
        this->genMidi.beatmodel = 2;
    else
        this->genMidi.beatmodel = this->genMidi.stressmodel;

    this->genMidi.nseg = this->stresspat[index].nseg;
    int nval = this->stresspat[index].nval;

    this->genMidi.segnum = time_num;
    this->genMidi.segden = time_denom * this->genMidi.nseg;
    AbcMusic::reduceFraction(&this->genMidi.segnum, &this->genMidi.segden);

    /* compute number of segments in quarter note */
    int qnotenum = this->genMidi.segden;
    int qnoteden = this->genMidi.segnum * 4;
    AbcMusic::reduceFraction(&qnotenum, &qnoteden);

    for (int n = 0; n < this->genMidi.nseg + 1; n++)
    {
        int i = n % nval;
        this->genMidi.ngain[n] = this->stresspat[index].vel[i];
        this->genMidi.fdur[n] = this->stresspat[index].expcoef[i];
        if(this->verbose)
        {
            char msg[100];
            snprintf(msg, 100, "%d %f", 
                this->genMidi.ngain[n],  
                this->genMidi.fdur[n]);
        }
        this->genMidi.maxdur = std::max(this->genMidi.maxdur, this->genMidi.fdur[n]);
        if (n > 0)
        {
            this->genMidi.fdursum[n] = this->genMidi.fdursum[n - 1] + 
                this->genMidi.fdur[n - 1] * qnoteden / (float) qnotenum;
        }
        /*  num[],denom[] use quarter note units = 1/1, segment units are usually
         *  half that size, so we divide by 2.0
         */
    }
    /*printf("maxdur = %f\n",maxdur);*/
    return 0;
}

/* [SS] 2013-04-10 */
void
AbcStore::read_custom_stress_file (char *filename)
{
    char msg[100];
    this->init_stresspat();
    FILE *inhandle = fopen(filename, "r");
    if(inhandle == NULL)
    {
        snprintf(msg, 100, "Failed to open file %s", filename);
        this->error(msg);
        return;
    }
    if(this->verbose > 0) 
    {
        snprintf(msg, 100, "reading %s", filename);
        this->log(msg);
    }
    char name[32];
    char meter[6];
    char str[4];
    int nseg, nval;
    int gain;
    float expand;
    while(!feof(inhandle))
    {
        if(feof(inhandle)) // XXX?
            break;
        int j = fscanf(inhandle, "%31s", name);
        if(j == -1)
            break;
        j = fscanf(inhandle, "%5s", meter);
        int index = this->stress_locator(&name[0], &meter[0]);
        if(this->verbose > 1) 
        {
            snprintf(msg, 100, "%s %s index = %d\n", name, meter, index);
            this->log(msg);
        }
        j = fscanf(inhandle, "%d %d", &nseg, &nval);
        if(this->verbose > 2) 
        {
            snprintf(msg, 100, "j = %d nseg = %d nval = %d\n", 
                    j, nseg, nval);
            this->log(msg);
        }
        if(j != 2) 
        {
            this->error("Malformed custom stress file.");
            fclose(inhandle); /* [JA] 2020-09-30 */
            return;
        }
        
        if(nval > 16) 
        {
            snprintf(msg, 100, "stresspat: nval = %d is too large for structure %s",
                    nval,name);
            this->error(msg);
            fclose(inhandle); /* [JA] 2020-09-30 */
            return;
        }
        
        /* copy model to stresspat[] */
        if(index < 0)
        {
            /*creating new model, must include name and meter */
            index = this->nmodels;
            if(index > 47)
            {
                this->error("used up all available space for stress models");
                fclose(inhandle); /* [JA] 2020-09-30 */
                return;
            }
            this->nmodels++;
            this->stresspat[index].name = name;
            this->stresspat[index].meter = meter;
        }
        this->stresspat[index].nseg = nseg;
        this->stresspat[index].nval = nval;
        for(int i = 0; i < nval; i++)
        {
            j = fscanf(inhandle, "%d %f", &gain, &expand);
            if(this->verbose > 2) 
            {
                snprintf(msg, 100, "%d %f", gain, expand);
                this->log(msg);
            }
            if(j != 2) 
            {
                this->error("Malformed custom stress file. (2)");
                fclose(inhandle); /* [JA] 2020-09-30 */
                return;
            }
            if(feof(inhandle))
                break;
            this->stresspat[index].vel[i] = gain;
            this->stresspat[index].expcoef[i] = expand;
        }
        if(fgets(str, 3, inhandle) == NULL) 
            break; /* [SDG] 2020-06-03 */
    }
    fclose(inhandle); /* [JA] 2020-09-30 */
}

int 
AbcStore::parse_stress_params(char const *input) 
{
    if(this->verbose > 1) 
        this->log("parsing stress parameters");
    char msg[100];
    char *next;
    float f = (float) strtod(input, &next);
    input = next;
    if (*input == '\0') {return -1;} /* no data, probably file name */
    if (f == 0.0) {return -1;} /* no data, probably file name */
    this->genMidi.nseg = (int) (f +0.0001);

    int n;
    for(n=0;n<32;n++) 
    {
        this->genMidi.fdur[n] = 0.0f; 
        this->genMidi.ngain[n] = 0;
    }
    if(this->genMidi.nseg > 31) return -1;

    n = 0;
    while(*input != '\0' && n < this->genMidi.nseg) 
    {
        f = (float) strtod(input,&next);
        this->genMidi.ngain[n] = (int) (f + 0.0001);
        if(this->genMidi.ngain[n] > 127 || this->genMidi.ngain[n] < 0) 
        {
            snprintf(msg, 100, "bad velocity value ngain[%d] = %d in ptstress command\n",
                n, this->genMidi.ngain[n]);
            this->error(msg);
        }
        input = next;
        f = (float) strtod(input, &next);
        this->genMidi.fdur[n] = f;
        if(this->genMidi.fdur[n] > this->genMidi.nseg || 
           this->genMidi.fdur[n] < 0.0f) 
        {
            snprintf(msg, 100, "bad expansion factor fdur[%d] = %f in ptstress command\n",
                n, this->genMidi.fdur[n]);
            this->error(msg);
        }
        input = next;
        n++;
    }
    if(n != this->genMidi.nseg) 
        return -1;
    else 
    {
        this->genMidi.beatmodel = 2;
        this->genMidi.barflymode = 1;
        return 0;
    }
}

void 
AbcStore::readstressfile(char const * filename)
{
    char msg[100];
    this->genMidi.maxdur = 0;
    FILE * inputhandle = fopen(filename,"r");
    if (inputhandle == NULL) 
    {
        snprintf(msg, 100, "Failed to open file %s", filename);
        this->error(msg);
        return;
    }
    for(int n=0;n<32;n++) 
    {
        this->genMidi.fdur[n]= 0.0f; 
        this->genMidi.ngain[n] = 0;
    }
    this->genMidi.fdursum[0] = this->genMidi.fdur[0];
    this->genMidi.beatmodel = 2; /* for Phil Taylor's stress model */
    int idummy = fscanf(inputhandle,"%d", &this->genMidi.nseg);
    /*printf("%d\n",nseg);*/
    if (this->genMidi.nseg > 31) 
        this->genMidi.nseg = 31;

    for(int n=0;n < this->genMidi.nseg+1; n++) 
    {
        idummy = fscanf(inputhandle,"%d %f",
                    &this->genMidi.ngain[n],
                    &this->genMidi.fdur[n]);
        if(verbose) 
        {
            snprintf(msg, 100, "%d %f",
                this->genMidi.ngain[n],
                this->genMidi.fdur[n]);
            this->log(msg);
        }
    }
    fclose(inputhandle);
}

void 
AbcStore::calculate_stress_parameters()
{
    char msg[100];
    this->genMidi.segnum = this->time_num;
    this->genMidi.segden = this->time_denom * this->genMidi.nseg;
    AbcMusic::reduceFraction(&this->genMidi.segnum, &this->genMidi.segden);

    /* compute number of segments in quarter note */
    int qnotenum = this->genMidi.segden;
    int qnoteden = this->genMidi.segnum*4;
    AbcMusic::reduceFraction(&qnotenum, &qnoteden);
    if(this->verbose > 1) 
    {
        snprintf(msg, 100, "segment size set to %d/%d",
            this->genMidi.segnum,
            this->genMidi.segden);
        this->log(msg);
    }
    for(int n=0; n<this->genMidi.nseg+1; n++) 
    {
        this->genMidi.maxdur = std::max(this->genMidi.maxdur,
                                        this->genMidi.fdur[n]);
        if(n > 0) 
        {
            this->genMidi.fdursum[n] = this->genMidi.fdursum[n-1] +
                this->genMidi.fdur[n-1]*qnoteden/(float) qnotenum;
        }
        if(this->genMidi.fdursum[n] > (this->genMidi.nseg + 0.05f)) 
        {
            this->error("bad stress file: sum of the expansion factors exceeds "
                        "number of segments\nAborting stress model");
            this->error(msg);
            this->genMidi.beatmodel = 0;
            return;
        }
        if(this->genMidi.ngain[n] > 127 || this->genMidi.ngain[n] < 0) 
        {
            this->error("bad stress file: note velocity not between 0 and 127\n Aborting the stress model");
            this->genMidi.beatmodel = 0;
            return;
        }
        /* num[],denom[] use quarter note units = 1/1, segment units are usually
            half that size, so we divide by 2.0
        */
        if(this->verbose > 1) 
        {
            snprintf(msg, 100, " %f", this->genMidi.fdursum[n]);
            this->log(msg);
        }
    }
    if(this->verbose > 1) 
    {
        snprintf(msg, 100, " == fdursum");
        this->log(msg);
    }
    /*printf("maxdur = %f\n",maxdur);*/
    /*if (fdursum[nseg] != (float) nseg) fdursum[nseg] = (float) nseg; [SS] 2011-09-06 */
    float lastsegvalue = this->genMidi.nseg * qnoteden / (float) qnotenum;
    /* ensure fdursum[nseg] = lastsegvalue [SS] 2011-09-06 */
    if (this->genMidi.fdursum[this->genMidi.nseg] != lastsegvalue)
    {
        snprintf(msg, 100, 
            "The sum of the expansion factors is not %d\n"
            " some adjustments are made.", this->genMidi.nseg);
        this->warning(msg);
        this->genMidi.fdursum[this->genMidi.nseg] = lastsegvalue;
    }
}

/* modifies the note durations inside a beat using Phil Taylor's model */
void 
AbcStore::beat_modifier(int i)
{
    /* start_num/start_denom is the original position of the note onset
    * end_num/end_denom is the original position of the end of the note
    * mstart_num/mstart_denom is the modified position of the note onset
    * mend_num/mend_denom is the modified position of the note end
    * startseg_num/startseg_denom is the position of the note onset in
    * segment units. eg. if a note starts in the middle of segment 1,
    * its value would be 3/2 (assuming we are counting from 0).
    */
    int notecount = 0;
    int inchord = 0;
    int start_num = 0;
    int start_denom = 1;
    int end_num = start_num;
    int end_denom = start_denom;
    int startseg_num, startseg_denom;
    int endseg_num, endseg_denom;
    int delta_num, delta_denom;
    int mstart_num, mstart_denom;
    int mend_num, mend_denom;

    i++;

    Abc::FeatureType ft;
    do
    {
        Abc::FeatureDesc &fd = this->featurelist[i];
        ft = fd.feature;
        if(ft == Abc::DOUBLE_BAR ||
           ft == Abc::BAR_REP ||
           ft == Abc::DOUBLE_REP ||
           ft == Abc::REP_BAR) 
           break;
        if(ft == Abc::CHORDON) 
        {
            inchord = 1;
            notecount = 0;
            i++;
            continue;
        }
        if(ft == Abc::CHORDOFF ||
           ft == Abc::CHORDOFFEX) 
        {
            inchord = 0;
            notecount = 0;
            start_num = end_num;
            start_denom = end_denom;
            i++;
            continue;
        }
        if(ft == Abc::NOTE || ft == Abc::TNOTE ||
           (ft == Abc::REST && fd.pitch == 0)) 
        {
            /* Care is needed for tied notes; they appear as TNOTE followed by*/
            /* two REST. We want to ignore those two rests.*/
            /* if REST and pitch[i] != 0 it is a tied note converted to a rest */
            /* Hopefully we do not encounter tied rests. */
            if(notecount == 0) 
            {
                AbcMusic::addFraction(&end_num, &end_denom, fd.num, fd.denom);
                AbcMusic::reduceFraction(&end_num, &end_denom);

                /* Convert note positions to where they would map to after applying
                   the duration modifiers. We map the positions to segment units and then
                   transform them back to note units using the modified space fdursum.
                   Since all units of time are represented as fractions, the operations
                   are less transparent.
                */
                /* divide start_num/start_denom by segnum/segden */
                /* (divide note duration by 4 because they are 4 times larger in num/denom*/
                startseg_num = start_num * this->genMidi.segden;
                startseg_denom = start_denom * this->genMidi.segnum*4;
                AbcMusic::reduceFraction(&startseg_num, &startseg_denom);

                /* repeat for end_num and end_denom; */
                endseg_num = end_num * this->genMidi.segden;
                endseg_denom = end_denom * this->genMidi.segnum*4;
                AbcMusic::reduceFraction(&endseg_num, &endseg_denom);

                fdursum_at_segment(startseg_num, startseg_denom, 
                            &mstart_num, &mstart_denom);
                fdursum_at_segment(endseg_num, endseg_denom, 
                            &mend_num, &mend_denom);

                /*  now compute the new note length */
                delta_num = mend_num;
                delta_denom = mend_denom;
                AbcMusic::addFraction(&delta_num, &delta_denom,
                                -mstart_num, mstart_denom);
                AbcMusic::reduceFraction(&delta_num, &delta_denom);
                if(inchord) 
                    notecount++;
            }
            if(this->verbose > 1) 
            {
                char msg[100];
                snprintf(msg, 100,
                    "pitch %d from = %d/%d (%d/%d) to %d/%d (%d/%d) becomes %d/%d %d/%d",
                    fd.pitch, start_num, start_denom, startseg_num, startseg_denom,
                    end_num, end_denom, endseg_num, endseg_denom,
                    mstart_num, mstart_denom, mend_num, mend_denom);
                this->log(msg);
                snprintf(msg, 100, " - %d/%d", delta_num, delta_denom);
                this->log(msg);
            }
            fd.num = delta_num; 
            fd.denom = delta_denom;

            int segnumber = startseg_num/startseg_denom; /* [SS] 2011-08-17 */
            fd.stressvelocity = this->genMidi.ngain[segnumber];
            
            if(notecount == 0) 
            {
                start_num = end_num;
                start_denom = end_denom;
            }
            if(fd.feature == Abc::TNOTE) 
                i++; /*skip following rest */
        }
        i++;
    } while(ft != Abc::SINGLE_BAR);
}

void 
AbcStore::fdursum_at_segment(int segposnum, int segposden, 
    int *val_num, int *val_den)
{
    int inx0, inx1, remainder;
    int nseg = this->genMidi.nseg;
    float val, a0, a1;
    *val_num = 0;
    inx0 = segposnum/segposden;
    if(inx0 > nseg) 
    {
        *val_num = *val_num + (int) ((float) 1000.0*this->genMidi.fdursum[nseg]);
        /*inx0 = inx0 - nseg;  [SS] 2013-06-07*/
        inx0 = inx0 % nseg;  /* [SS] 2013-06-07 */
    }
    inx1 = inx0 + 1;
    remainder = segposnum % segposden;
    if(remainder == 0) 
    {
        val = this->genMidi.fdursum[inx0];
    } 
    else 
    {
        if(inx1 > nseg) 
        {
            printf("***fdursum_at_segment: inx1 = %d too large\n",inx1);
        }
        
        a0 = remainder / (float) segposden;
        a1 = 1.f - a0; 
        val = a1 * this->genMidi.fdursum[inx0] + 
              a0 * this->genMidi.fdursum[inx1];
    }
    *val_num  += (int) (1000.0 * val +0.5);
    *val_den = 1000; 
    AbcMusic::reduceFraction(val_num, val_den);
}