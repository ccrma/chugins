#include "AbcMidiFile.h"
#include <cstdio>
#include <cstdlib>

/*
 * AbcMidiFile.cpp - loosely based on the the original
 *  abc: midifile.c.
 * 
 *      Standard MIDI Files, and is part of the Musical
 *      instrument Digital Interface specification.
 *      The spec is avaiable from:
 *
 *           International MIDI Association
 *           5316 West 57th Street
 *           Los Angeles, CA 90056
 *
 *      An in-depth description of the spec can also be found
 *      in the article "Introducing Standard MIDI Files", published
 *      in Electronic Musician magazine, April, 1989.
 * 
 */

const int AbcMidiFile::magicHD = 0x4d546864;
const int AbcMidiFile::magicRK = 0x4d54726b;

AbcMidiFile::AbcMidiFile()
{
    this->writer = nullptr;
}

AbcMidiFile::~AbcMidiFile()
{
}

int
AbcMidiFile::write(IMidiWriter *w, int format, int ntracks, int division)
{
    int err = 0;
    this->writer = w;
    return err;
}

int
AbcMidiFile::writeMetaEvent(long dt, int type, char const *data, int nbytes)
{
    int err = 0;
    return err;
}

int
AbcMidiFile::writeTempo(long t)
{
    int err = 0;
    return err;
}

int
AbcMidiFile::writeMidiEvent(long dt, int type, int chan, char const *data, int nbytes)
{
    int err = 0;
    return err;
}

void
AbcMidiFile::singleNoteTuningChange(int key, float midipitch)
{
}

#if 0

int Mf_nomerge = 0;    /* 1 => continue'ed system exclusives are */
                    /* not collapsed. */
long Mf_currtime = 0L;    /* current time in delta-time units */

/* private stuff */
long Mf_toberead = 0L;
long Mf_bytesread = 0L;

long Mf_numbyteswritten = 0L; /* linking with store.c */

static long readvarinum();
static long read32bit();
static long to32bit();
static int read16bit();
static int to16bit();
static char *msg();
int skiptrack ();

/* this block was previously in midifile.h [SS] 2010-01-23*/
static int readtrack();
static void readheader();
static void badbyte();
static void metaevent();
static void sysex();
static void chanmessage();
static void msginit();
static void msgadd();
static void biggermsg();
static void mf_write_track_chunk();
static void mf_write_header_chunk();
static void WriteVarLen();
static void write32bit();
static void write16bit();
static int msgleng();
static int eputc();
/* end of block */


/* following declaration added 27/8/96 JRA 
static readheader(), readtrack(), badbyte(), metaevent(), sysex(),
       chanmessage(), msginit(), msgleng(), msgadd(), biggermsg();
*/
int ntrks;

void mfread()     /* The only non-static function in this file. */
{
  int track;
  if ( Mf_getc == NULLFUNC )
    mferror("mfread() called without setting Mf_getc"); 

  readheader();
  track =1;
  while (readtrack()) 
    {track++;
     if(track>ntrks) break;
    }
}


void mfreadtrk(itrack)     /* The only non-static function in this file. */
int itrack;
{
  int track,ok;
  if ( Mf_getc == NULLFUNC )
    mferror("mfprocess() called without setting Mf_getc");

  readheader();
  track =1;
  ok = 1;
  for (track=1;track<=ntrks && ok == 1;track++)
   {if (track == itrack)
     ok = readtrack();
    else
     ok = skiptrack();
   }
}



/* for backward compatibility with the original lib */
void midifile()
{
    mfread();
}

static int
readmt(s)    /* read through the "MThd" or "MTrk" header string */
char *s;
{
  int n = 0;
  char *p = s;
  int c;

  while ( n++<4 && (c=(*Mf_getc)()) != EOF ) {
    if ( c != *p++ ) {
      char buff[32];
      (void) strcpy(buff,"expecting ");
      (void) strcat(buff,s);
      mferror(buff);
    }
  }
  return(c);
}

static int
egetc()      /* read a single character and abort on EOF */
{
  int c = (*Mf_getc)();

  if ( c == EOF )
    mferror("premature EOF");
  Mf_toberead--;
  Mf_bytesread++;
  return(c);
}

static void
readheader()    /* read a header chunk */
{
  int format, division;

  if ( readmt("MThd") == EOF )
    return;

  Mf_toberead = read32bit();
  Mf_bytesread = 0;
  format = read16bit();
  ntrks = read16bit();
  division = read16bit();

  if ( Mf_header )
    (*Mf_header)(format,ntrks,division);

  /* flush any extra stuff, in case the length of header is not 6 */
  while ( Mf_toberead > 0 )
    (void) egetc();
}

int skiptrack ()
{
int byte;
if ( readmt("MTrk") == EOF )
    return(0);
Mf_toberead = read32bit();
byte = 0;
while (Mf_toberead && byte != EOF) byte = egetc();
if (byte == EOF) {mferror("premature EOF\n"); return(0);}
return(1);
}


int
readtrack()     /* read a track chunk */
{
  /* This array is indexed by the high half of a status byte.  It's */
  /* value is either the number of bytes needed (1 or 2) for a channel */
  /* message, or 0 (meaning it's not  a channel message). */
  static int chantype[] = {
    0, 0, 0, 0, 0, 0, 0, 0,    /* 0x00 through 0x70 */
    2, 2, 2, 2, 1, 1, 2, 0    /* 0x80 through 0xf0 */
  };
  long lookfor;
  int c, c1, type;
  int sysexcontinue = 0;  /* 1 if last message was an unfinished sysex */
  int running = 0;  /* 1 when running status used */
  int status = 0;    /* status value (e.g. 0x90==note-on) */
  int laststatus;   /* for running status */
  int needed;
  int time_increment;
  long varinum;

  laststatus = 0;
  if ( readmt("MTrk") == EOF )
    return(0);

  Mf_toberead = read32bit();
  Mf_currtime = 0;
  Mf_bytesread =0;

  if ( Mf_trackstart )
    (*Mf_trackstart)();

  while ( Mf_toberead > 0 ) {

    time_increment =  readvarinum();  /* delta time */
    if (time_increment < 0) {printf("bad time increment = %d\n",time_increment);
                             mferror("bad time increment");
                            }
    Mf_currtime += time_increment; /* [SS] 2018-06-13 */

    c = egetc();

    if ( sysexcontinue && c != 0xf7 )
      mferror("didn't find expected continuation of a sysex");

/* if bit 7 not set, there is no status byte following the
*  delta time, so it must a running status and we assume the
*  last status occuring in the preceding channel message.   */
    if ( (c & 0x80) == 0 ) {   /* running status? */
      if ( status == 0 )
        mferror("unexpected running status");
      running = 1;
    }
    else { /* [SS] 2013-09-10 */
      if (c>>4 != 0x0f) { /* if it is not a meta event save the status*/
       laststatus = c;
        }
     running = 0;
     status = c;
    }

    /* [SS] 2013-09-10 */
    if (running) needed = chantype[ (laststatus>>4) & 0xf];
    else needed = chantype[ (status>>4) & 0xf ];

    if ( needed ) {    /* ie. is it a channel message? */

      if ( running ) {
        c1 = c;
        chanmessage( laststatus, c1, (needed>1) ? egetc() : 0 );
        }
      else {
        c1 = egetc();
        chanmessage( status, c1, (needed>1) ? egetc() : 0 );
       }
      continue;;
    }

    switch ( c ) {

    case 0xff:      /* meta event */

      type = egetc();
      varinum = readvarinum();
      lookfor = Mf_toberead - varinum;
      msginit();

      while ( Mf_toberead > lookfor )
        msgadd(egetc());

      metaevent(type);
      break;

    case 0xf0:    /* start of system exclusive */

      varinum = readvarinum();
      lookfor = Mf_toberead - varinum;
      msginit();
      msgadd(0xf0);

      while ( Mf_toberead > lookfor )
        msgadd(c=egetc());

      if ( c==0xf7 || Mf_nomerge==0 )
        sysex();
      else
        sysexcontinue = 1;  /* merge into next msg */
      break;

    case 0xf7:  /* sysex continuation or arbitrary stuff */

      varinum = readvarinum();
      lookfor = Mf_toberead - varinum;

      if ( ! sysexcontinue )
        msginit();

      while ( Mf_toberead > lookfor )
        msgadd(c=egetc());

      if ( ! sysexcontinue ) {
        if ( Mf_arbitrary )
          (*Mf_arbitrary)(msgleng(),msg());
      }
      else if ( c == 0xf7 ) {
        sysex();
        sysexcontinue = 0;
      }
      break;
    default:
      badbyte(c);
      break;
    }
  }
  if ( Mf_trackend )
    (*Mf_trackend)();
  return(1);
}

static void
badbyte(c)
int c;
{
  char buff[32];

  (void) sprintf(buff,"unexpected byte: 0x%02x",c);
  mferror(buff);
}

static void
metaevent(type)
int type;
{
  int leng;
  char *m;

  leng = msgleng();
  m = msg();
  switch  ( type ) {
  case 0x00:
    if ( Mf_seqnum )
      (*Mf_seqnum)(to16bit(m[0],m[1]));
    break;
  case 0x01:  /* Text event */
  case 0x02:  /* Copyright notice */
  case 0x03:  /* Sequence/Track name */
  case 0x04:  /* Instrument name */
  case 0x05:  /* Lyric */
  case 0x06:  /* Marker */
  case 0x07:  /* Cue point */
  case 0x08:
  case 0x09:
  case 0x0a:
  case 0x0b:
  case 0x0c:
  case 0x0d:
  case 0x0e:
  case 0x0f:
    /* These are all text events */
    if ( Mf_text )
      (*Mf_text)(type,leng,m);
    break;
  case 0x2f:  /* End of Track */
    if ( Mf_eot )
      (*Mf_eot)();
    break;
  case 0x51:  /* Set tempo */
    if ( Mf_tempo )
      (*Mf_tempo)(to32bit(0,m[0],m[1],m[2]));
    break;
  case 0x54:
    if ( Mf_smpte )
      (*Mf_smpte)(m[0],m[1],m[2],m[3],m[4]);
    break;
  case 0x58:
    if ( Mf_timesig )
      (*Mf_timesig)(m[0],m[1],m[2],m[3]);
    break;
  case 0x59:
    if ( Mf_keysig )
      (*Mf_keysig)(m[0],m[1]);
    break;
  case 0x7f:
    if ( Mf_seqspecific )
      (*Mf_seqspecific)(leng,m);
    break;
  default:
    if ( Mf_metamisc )
      (*Mf_metamisc)(type,leng,m);
  }
}

static void 
sysex()
{
  if ( Mf_sysex )
    (*Mf_sysex)(msgleng(),msg());
}

static void
chanmessage(status,c1,c2)
int status;
int c1, c2;
{
  int chan = status & 0xf;

  switch ( status & 0xf0 ) {
  case 0x80:
    if ( Mf_noteoff )
      (*Mf_noteoff)(chan,c1,c2);
    break;
  case 0x90:
    if ( Mf_noteon )
      (*Mf_noteon)(chan,c1,c2);
    break;
  case 0xa0:
    if ( Mf_pressure )
      (*Mf_pressure)(chan,c1,c2);
    break;
  case 0xb0:
    if ( Mf_parameter )
      (*Mf_parameter)(chan,c1,c2);
    break;
  case 0xe0:
    if ( Mf_pitchbend )
      (*Mf_pitchbend)(chan,c1,c2);
    break;
  case 0xc0:
    if ( Mf_program )
      (*Mf_program)(chan,c1);
    break;
  case 0xd0:
    if ( Mf_chanpressure )
      (*Mf_chanpressure)(chan,c1);
    break;
  }
}

/* readvarinum - read a varying-length number, and return the */
/* number of characters it took. */

static long
readvarinum()
{
  long value;
  int c;

  c = egetc();
  value = c;
  if ( c & 0x80 ) {
    value &= 0x7f;
    do {
      c = egetc();
      value = (value << 7) + (c & 0x7f);
    } while (c & 0x80);
  }
  return (value);
}

static long
to32bit(c1,c2,c3,c4)
int c1, c2, c3, c4;
{
  long value = 0L;

  value = (c1 & 0xff);
  value = (value<<8) + (c2 & 0xff);
  value = (value<<8) + (c3 & 0xff);
  value = (value<<8) + (c4 & 0xff);
  return (value);
}

static int
to16bit(c1,c2)
int c1, c2;
{
  return ((c1 & 0xff ) << 8) + (c2 & 0xff);
}

static long
read32bit()
{
  int c1, c2, c3, c4;

  c1 = egetc();
  c2 = egetc();
  c3 = egetc();
  c4 = egetc();
  return to32bit(c1,c2,c3,c4);
}

static int
read16bit()
{
  int c1, c2;
  c1 = egetc();
  c2 = egetc();
  return to16bit(c1,c2);
}

/* static */
void mferror(s)
	char *s;
{
  if ( Mf_error ) {
    (*Mf_error)(s);
    exit(1);
  } else {
    printf("MIDI read/write error : %s\n", s);
    exit(1);
  };
}

/* The code below allows collection of a system exclusive message of */
/* arbitrary length.  The Msgbuff is expanded as necessary.  The only */
/* visible data/routines are msginit(), msgadd(), msg(), msgleng(). */

#define MSGINCREMENT 128
static char *Msgbuff = NULL;  /* message buffer */
static int Msgsize = 0;    /* Size of currently allocated Msg */
static int Msgindex = 0;  /* index of next available location in Msg */

static void
msginit()
{
  Msgindex = 0;
}

static char *
msg()
{
  return(Msgbuff);
}

static int
msgleng()
{
  return(Msgindex);
}

static void
msgadd(c)
int c;
{
  /* If necessary, allocate larger message buffer. */
  if ( Msgindex >= Msgsize )
    biggermsg();
  Msgbuff[Msgindex++] = c;
}

static void
biggermsg()
{
/*   char *malloc(); */
  char *newmess;
  char *oldmess = Msgbuff;
  int oldleng = Msgsize;

  Msgsize += MSGINCREMENT;
/* to ensure a string is terminated with 0 [SS] 2017-08-30 */
/*  newmess = (char *) malloc( (unsigned)(sizeof(char)*Msgsize) ); */
  newmess = (char *) calloc( (unsigned)(sizeof(char)*Msgsize), sizeof(char));

  if(newmess == NULL)
    mferror("malloc error!");
    
  /* copy old message into larger new one */
  if ( oldmess != NULL ) {
    register char *p = newmess;
    register char *q = oldmess;
    register char *endq = &oldmess[oldleng];

    for ( ; q!=endq ; p++,q++ )
      *p = *q;
    free(oldmess);
  }
  Msgbuff = newmess;
}

/*
 * mfwrite() - The only fuction you'll need to call to write out
 *             a midi file.
 *
 * format      0 - Single multi-channel track
 *             1 - Multiple simultaneous tracks
 *             2 - One or more sequentially independent
 *                 single track patterns                
 * ntracks     The number of tracks in the file.
 * division    This is kind of tricky, it can represent two
 *             things, depending on whether it is positive or negative
 *             (bit 15 set or not).  If  bit  15  of division  is zero,
 *             bits 14 through 0 represent the number of delta-time
 *             "ticks" which make up a quarter note.  If bit  15 of
 *             division  is  a one, delta-times in a file correspond to
 *             subdivisions of a second similiar to  SMPTE  and  MIDI
 *             time code.  In  this format bits 14 through 8 contain
 *             one of four values - 24, -25, -29, or -30,
 *             corresponding  to  the  four standard  SMPTE and MIDI
 *             time code frame per second formats, where  -29
 *             represents  30  drop  frame.   The  second  byte
 *             consisting  of  bits 7 through 0 corresponds the the
 *             resolution within a frame.  Refer the Standard MIDI
 *             Files 1.0 spec for more details.
 * fp          This should be the open file pointer to the file you
 *             want to write.  It will have be a global in order
 *             to work with Mf_putc.  
 */ 
void 
mfwrite(format,ntracks,division,fp) 
int format,ntracks,division; 
FILE *fp; 
{
    int i; 

    if ( Mf_putc == NULLFUNC )
      mferror("mf_write() called without setting Mf_putc");

    if ( Mf_writetrack == NULLFUNC )
      mferror("mf_write() called without setting Mf_writetrack"); 

    /* every MIDI file starts with a header */
    mf_write_header_chunk(format,ntracks,division);

    /* In format 1 files, the first track is a tempo map */
    if(format == 1 && ( Mf_writetempotrack ))
    {
  (*Mf_writetempotrack)();
    }

    /* The rest of the file is a series of tracks */
    for(i = 0; i < ntracks; i++)
        mf_write_track_chunk(i,fp);
}

#ifdef NOFTELL
int nullpass = 0;
#endif

int nullputc(c)
/* dummy putc for abc checking option */
/* also used for FTELL work-around */
char c;
{
  int t;

  t = ((int) c) & 0xFF;
  return (t);
}                         

static void 
mf_write_track_chunk(which_track,fp)
int which_track;
FILE *fp;
{
  long trkhdr,trklength;
  long offset, place_marker;
  long endspace = 0; /* [SDG] 2020-06-02 */

/* There is an alternate version of this code selected by NOFTELL
   which doesn't require use of file seek */
  
#ifdef NOFTELL
  int (*tempfunc)() = NULLFUNC;
  extern int myputc();

  trkhdr = MTrk;
  trklength = 0;
  tempfunc = Mf_putc;
  Mf_putc = nullputc;
  Mf_numbyteswritten = 0L; /* the header's length doesn't count */
  /* dummy write track */
  nullpass = 1;
  if( Mf_writetrack )
  {
    endspace = (*Mf_writetrack)(which_track);
  }
  WriteVarLen(endspace);
  /* mf_write End of track meta event */
  /* eputc(0); */
  eputc(meta_event);
  eputc(end_of_track);
  eputc(0);
  trklength = Mf_numbyteswritten;
  /* now actually write track */
  Mf_putc = tempfunc;
  nullpass = 0;
  write32bit(trkhdr);
  write32bit(trklength);
  Mf_numbyteswritten = 0L; /* the header's length doesn't count */
  /* now output the data */
  if( Mf_writetrack )
  {
    endspace = (*Mf_writetrack)(which_track);
  }
  WriteVarLen(endspace);
  eputc(meta_event);
  eputc(end_of_track);
  eputc(0);
  if (trklength != Mf_numbyteswritten) {
    mferror("NOFTELL workaround failed to predict tracklength");
  }

#else  
  /* this is the original code */
  trkhdr = MTrk;
  trklength = 0;

  /* Remember where the length was written, because we don't
     know how long it will be until we've finished writing */
  offset = ftell(fp); 

#ifdef DEBUG
        printf("offset = %d\n",(int) offset);
#endif

  /* Write the track chunk header */
  write32bit(trkhdr);
  write32bit(trklength);

  Mf_numbyteswritten = 0L; /* the header's length doesn't count */

  if( Mf_writetrack )
  {
      endspace = (*Mf_writetrack)(which_track);
  }

  /* mf_write End of track meta event */
  WriteVarLen(endspace);
  /* eputc(0); */
  eputc(meta_event);
  eputc(end_of_track);

   eputc(0);
   
  /* It's impossible to know how long the track chunk will be beforehand,
           so the position of the track length data is kept so that it can
           be written after the chunk has been generated */
  place_marker = ftell(fp);
  
  /* This method turned out not to be portable because the
           parameter returned from ftell is not guaranteed to be
           in bytes on every machine */
   /* track.length = place_marker - offset - (long) sizeof(track); */

#ifdef DEBUG
printf("length = %d\n",(int) trklength);
#endif

   if(fseek(fp,offset,0) < 0)
      mferror("error seeking during final stage of write");

  trklength = Mf_numbyteswritten;

  /* Re-mf_write the track chunk header with right length */
  write32bit(trkhdr);
  write32bit(trklength);

  fseek(fp,place_marker,0);
#endif
} /* End gen_track_chunk() */


static void 
mf_write_header_chunk(format,ntracks,division)
int format,ntracks,division;
{
    long ident,length;
    
    ident = MThd;           /* Head chunk identifier                    */
    length = 6;             /* Chunk length                             */

    /* individual bytes of the header must be written separately
       to preserve byte order across cpu types :-( */
    write32bit(ident);
    write32bit(length);
    write16bit(format);
    write16bit(ntracks);
    write16bit(division);
} /* end gen_header_chunk() */


/*
 * mf_write_midi_event()
 * 
 * Library routine to mf_write a single MIDI track event in the standard MIDI
 * file format. The format is:
 *
 *                    <delta-time><event>
 *
 * In this case, event can be any multi-byte midi message, such as
 * "note on", "note off", etc.      
 *
 * delta_time - the time in ticks since the last event.
 * type - the type of meta event.
 * chan - The midi channel.
 * data - A pointer to a block of chars containing the META EVENT,
 *        data.
 * size - The length of the meta-event data.
 */
int 
mf_write_midi_event(delta_time, type, chan, data, size)
long delta_time;
int chan,type;
int size;
char *data;
{
    int i;
    char c;

    WriteVarLen(delta_time);

    /* all MIDI events start with the type in the first four bits,
       and the channel in the lower four bits */
    c = (char) (type | chan);

    if(chan > 15) {
      mferror("error: MIDI channel greater than 16");
    };

    eputc(c);

    /* write out the data bytes */
    for(i = 0; i < size; i++)
  eputc(data[i]);

    return(size);
} /* end mf_write MIDI event */

/*
 * mf_write_meta_event()
 *
 * Library routine to mf_write a single meta event in the standard MIDI
 * file format. The format of a meta event is:
 *
 *          <delta-time><FF><type><length><bytes>
 *
 * delta_time - the time in ticks since the last event.
 * type - the type of meta event.
 * data - A pointer to a block of chars containing the META EVENT,
 *        data.
 * size - The length of the meta-event data.
 */
int
mf_write_meta_event(delta_time, type, data, size)
long delta_time;
char *data;
int type;
int size;
{
    int i;

    WriteVarLen(delta_time);
    
    /* This marks the fact we're writing a meta-event */
    eputc(meta_event);

    /* The type of meta event */
    eputc(type);

    /* The length of the data bytes to follow */
    WriteVarLen((long)size); 

    for(i = 0; i < size; i++)
    {
  if(eputc((data[i] & 0xff)) != (data[i] & 0xff))
      return(-1); 
    }
    return(size);
} /* end mf_write_meta_event */

void 
mf_write_tempo(tempo)
long tempo;
{
    /* Write tempo */
    /* all tempos are written as 120 beats/minute, */
    /* expressed in microseconds/quarter note     */
    eputc(0);
    eputc(meta_event);
    eputc(set_tempo);

    eputc(3);
    eputc((char)(0xff & (tempo >> 16)));
    eputc((char)(0xff & (tempo >> 8)));
    eputc((char)(0xff & tempo));
}



/* 
 * This routine converts delta times in seconds into ticks. The
 * else statement is needed because the formula is different for tracks
 * based on notes and tracks based on SMPTE times.
 *
 */
long 
mf_sec2ticks(secs,division,tempo)
float secs;
int division;
long tempo;
{
  long ticks;
  float smpte_format, smpte_resolution;

  if(division > 0) {
    ticks = (long) ((secs * ((float)(division))* 1000000.0) / 
                                          ((float)(tempo))+0.5);
  } else {
    smpte_format = upperbyte(division);
    smpte_resolution = lowerbyte(division);
    ticks = (long) (secs * smpte_format * smpte_resolution * 1000000.0 + 0.5);
  };
  return (ticks);
} /* end of sec2ticks() */

/*
 * Write multi-length bytes to MIDI format files
 */
static void 
WriteVarLen(value)
long value;
{
  long buffer;

  buffer = value & 0x7f;
  while((value >>= 7) > 0)
  {
  buffer <<= 8;
  buffer |= 0x80;
  buffer += (value & 0x7f);
  }
  while(1){
       eputc((char)(buffer & 0xff));
       
  if(buffer & 0x80)
    buffer >>= 8;
  else
    return;
  }
}/* end of WriteVarLen */


void single_note_tuning_change(int key, float midipitch)
{
unsigned char kk,xx,yy,zz;
int number,intfraction;
float fraction;
eputc(0); /* varinum delta_t (time to next event) */
eputc(0xf0); /* sysex initiation */
 eputc(11);  /* 11 bytes included in sysex */
eputc(127); /* universal sysex command (0x7f) */
eputc(0);    /* device id */
eputc(8);    /* midi tuning */
eputc(2);    /* note change */
eputc(0);    /* program number 0 - 127 */
eputc(1);    /* only one change */
kk = (unsigned char) 127 & key;
eputc(kk);  /* MIDI key  0 - 127 */
number = (int) midipitch;
fraction = midipitch - (float) number;
if (fraction < 0.0) fraction = -fraction;
intfraction = (int) fraction*16384;
xx = 0x7f & number;
yy = intfraction/128;
zz = intfraction % 128;
yy = 0x7f & yy;
zz = 0x7f & zz;
eputc(xx);
eputc(yy);
eputc(zz);
eputc(247); /* 0xf7 terminates sysex command */
}

/* 
 * This routine converts delta times in ticks into seconds. The
 * else statement is needed because the formula is different for tracks
 * based on notes and tracks based on SMPTE times.
 *
 */
float 
mf_ticks2sec(ticks,division,tempo)
int division;
long tempo;
long ticks;
{
    float ret;
    float smpte_format, smpte_resolution;

    if(division > 0) {
        ret = ((float) (((float)(ticks) * (float)(tempo)) / ((float)(division) * 1000000.0)));
    } else {
       smpte_format = upperbyte(division);
       smpte_resolution = lowerbyte(division);
       ret = (float) ((float) ticks / (smpte_format * smpte_resolution * 1000000.0));
    }
    return (ret);
} /* end of ticks2sec() */

/*
 * write32bit()
 * write16bit()
 *
 * These routines are used to make sure that the byte order of
 * the various data types remains constant between machines. This
 * helps make sure that the code will be portable from one system
 * to the next.  It is slightly dangerous that it assumes that longs
 * have at least 32 bits and ints have at least 16 bits, but this
 * has been true at least on PCs, UNIX machines, and Macintosh's.
 *
 */
static void 
write32bit(data)
long data;
{
    eputc((char)((data >> 24) & 0xff));
    eputc((char)((data >> 16) & 0xff));
    eputc((char)((data >> 8 ) & 0xff));
    eputc((char)(data & 0xff));
}

static void 
write16bit(data)
int data;
{
    eputc((char)((data & 0xff00) >> 8));
    eputc((char)(data & 0xff));
}

/* write a single character and abort on error */
static int eputc(c)      
char c;
{
  int return_val;
  
  if((Mf_putc) == NULLFUNC)
  {
    mferror("Mf_putc undefined");
    return(-1);
  }
  
  return_val = (*Mf_putc)(c);

  if ( return_val == EOF )
    mferror("error writing");
#ifdef PCCFIX
/* This seems to be needed for the FTELL workaround */
  if (return_val != -1) {
    return_val = (int) c;
  };
#endif                        
  Mf_numbyteswritten++;
  if(Mf_numbyteswritten > 500000) {
     printf("eputc: aborting because of file runaway (infinite loop)\n");
     exit(1);
     }
  return(return_val);
}

#endif