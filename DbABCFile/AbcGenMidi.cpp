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

/* init all vars associated with meter --- */
void
AbcGenMidi::set_meter(int n, int m)
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

/* add a/b to the count of units in the bar */
void
AbcGenMidi::addunits(int a, int b)
{
    this->bar_num = this->bar_num*(b*this->b_denom) + (a*this->b_num)*this->bar_denom;
    this->bar_denom = this->bar_denom * (b*this->b_denom);
    this->reduce(&this->bar_num, &this->bar_denom);
  /*printf("position = %d/%d\n",bar_num,bar_denom);*/
}

/* set up a string which indicates how to generate accompaniment from */
/* guitar chords (i.e. "A", "G" in abc). */
/* called from dodeferred(), startfile() and setbeat() */
void
AbcGenMidi::set_gchords(char const *s)
{
    char const *p = s;
    int j = 0;
    int seq_len = 0;
#if 0
    while ((strchr("zcfbghijGHIJx", *p) != NULL) && (j <39)) 
    {
        if (*p == 0) break;
        gchord_seq[j] = *p;
        p = p + 1;
    if ((*p >= '0') && (*p <= '9')) {
      gchord_len[j] = readnump(&p);
    } else {
      gchord_len[j] = 1;
    };
    seq_len = seq_len + gchord_len[j];
    j = j + 1;
  };
  if (seq_len == 0) {
    event_error("Bad gchord");
    gchord_seq[0] = 'z';
    gchord_len[0] = 1;
    seq_len = 1;
  };
  gchord_seq[j] = '\0';
  if (j == 39) {
    event_error("Sequence string too long");
  };
  /* work out unit delay in 1/4 notes*/
  g_num = mtime_num * 4*gchordbars;
  g_denom = mtime_denom * seq_len;
  reduce(&g_num, &g_denom);
/*  printf("%s  %d %d\n",s,g_num,g_denom); */
#endif
}

void set_drums(s)
char* s;
/* set up a string which indicates drum pattern */
/* called from dodeferred() */
{
  int seq_len, count, drum_hits;
  char* p;
  int i, j, place;

  p = s;
  count = 0;
  drum_hits = 0;
  seq_len = 0;
  while (((*p == 'z') || (*p == 'd')) && (count<39)) {
    if (*p == 'd') {
      drum_hits = drum_hits + 1;
    };
    drum_seq[count] = *p;
    p = p + 1;
    if ((*p >= '0') && (*p <= '9')) {
      drum_len[count] = readnump(&p);
    } else {
      drum_len[count] = 1;
    };
    seq_len = seq_len + drum_len[count];
    count = count + 1;
  };
  drum_seq[count] = '\0';
  if (seq_len == 0) {
    event_error("Bad drum sequence");
    drum_seq[0] = 'z';
    drum_len[0] = 1;
    seq_len = 1;
  };
  if (count == 39) {
    event_error("Drum sequence string too long");
  };
  /* look for program and velocity specifiers */
  for (i = 0; i<count; i++) {
    drum_program[i] = 35;
    drum_velocity[i] = 80;
  };
  skipspace(&p);
  i = 0;
  place = 0;
  while (isdigit(*p)) {
    j = readnump(&p);
    if (i < drum_hits) {
      while (drum_seq[place] != 'd') {
        place = place + 1;
      };
      if (j > 127) {
        event_error("Drum program must be in the range 0-127");
      } else {
        drum_program[place] = j;
      };
      place = place + 1;
    } else {
      if (i < 2*count) {
        if (i == drum_hits) {
          place = 0;
        };
        while (drum_seq[place] != 'd') {
          place = place + 1;
        };
        if ((j < 1) || (j > 127)) {
          event_error("Drum velocity must be in the range 1-127");
        } else {
          drum_velocity[place] = j;
        };
        place = place + 1;
      };
    };
    i = i + 1;
    skipspace(&p);
  };
  if (i > 2*drum_hits) {
    event_error("Too many data items for drum sequence");
  };
  /* work out unit delay in 1/4 notes*/
  drum_num = mtime_num * 4*drumbars;
  drum_denom = mtime_denom * seq_len;
  reduce(&drum_num, &drum_denom);
}