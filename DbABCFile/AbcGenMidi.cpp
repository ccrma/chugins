#include "AbcGenMidi.h"
#include "AbcMusic.h"
#include <cstdlib>
#include <cstdio>
#include <algorithm>

AbcGenMidi::AbcGenMidi()
{
}

void
AbcGenMidi::Init()
{
    this->barflymode = 1;
    this->stressmodel = 0;
    this->stress_pattern_loaded = 0;
    this->parts = 0;
    this->partspec.clear(); // string
    for(int j=0; j<26; j++)
        this->part_start[j] = -1;
    for(int j=0; j<16; j++)
        this->channel_in_use[j] = 0; // xxx: it's actually len:19
    for(int j=0; j<256; j++)
        this->drum_map[j] = j;
}

int 
AbcGenMidi::parse_stress_params(char *input) 
{
    char *next;
    int success = 0;
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
            printf("**error** bad stress file: sum of the expansion factors exceeds number of segments\nAborting stress model\n");
            this->beatmodel = 0;
            return;
        }
        if (ngain[n] > 127 || ngain[n] < 0) 
        {  
            printf("**error** bad stress file: note velocity not between 0 and 127\n Aborting the stress model\n");
            this->beatmodel = 0;
            return;
        }
        lastsegvalue = (float) this->nseg * qfrac;
        /* ensure fdursum[nseg] = lastsegvalue [SS] 2011-09-06 */
        if(this->fdursum[this->nseg] != lastsegvalue)
        {
            printf("**warning** the sum of the expansion factors is not %d\n some adjustments are made.\n",
                    nseg);
            this->fdursum[nseg] = lastsegvalue;
        }
    }
}