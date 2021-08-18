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

static const int sMagicHDR = 0x4d546864;
static const int sMagicTRK = 0x4d54726b;

AbcMidiFile::AbcMidiFile()
{
    this->helper = nullptr;
}

AbcMidiFile::~AbcMidiFile()
{
}

int
AbcMidiFile::write(IFileHelper *w, FILE *fp, int format, int ntracks, int division)
{
    int err = 0;
    this->helper = w;
    this->fp = fp;
    this->numBytesWritten = 0;

    /* every MIDI file starts with a header */
    this->writeHeaderChunk(format, ntracks, division);

    /* In format 1 files, the first track is a tempo map */
    if(format == 1)
        this->helper->writetempotrack();

    /* The rest of the file is a series of tracks */
    for(int i = 0; i < ntracks; i++)
        this->writeTrackChunk(i);
    
    this->fp = nullptr;
    return err;
}

/*
 * writeMidiEvent()
 * 
 * Routine to write a single MIDI track event in the standard MIDI
 * file format. The format is:
 *
 *    <delta-time><event>
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
AbcMidiFile::writeMidiEvent(long delta_time, int type, int chan, 
    char const *data, int size)
{
    if(chan > 15) 
        this->helper->midierror("error: MIDI channel greater than 16");

    this->writeVarLen(delta_time);

    /* all MIDI events start with the type in the first four bits,
       and the channel in the lower four bits */
    char c = (char) (type | chan);
    this->writeByte(c);

    /* write out the data bytes */
    for(int i = 0; i < size; i++)
        this->writeByte(data[i]);

    return size;
}

/*
 * Routine to write a single meta event in the standard MIDI
 * file format. The format of a meta event is:
 *
 *   <delta-time><FF><type><length><bytes>
 *
 * delta_time - the time in ticks since the last event.
 * type - the type of meta event.
 * data - A pointer to a block of chars containing the META EVENT data.
 * size - The length of the meta-event data.
 * 
 * returns number of bytes written or -1
 */
int
AbcMidiFile::writeMetaEvent(long delta_time, int type, char const *data, int size)
{
    this->writeVarLen(delta_time);
    
    /* This marks the fact we're writing a meta-event */
    this->writeByte(MidiEvent::meta_event);

    /* The type of meta event */
    this->writeByte(type);

    /* The length of the data bytes to follow */
    this->writeVarLen((long) size); 

    for(int i = 0; i < size; i++)
    {
        if(this->writeByte((data[i] & 0xff)) != (data[i] & 0xff))
            return -1; 
    }
    return size;
}


/* Write tempo */
/* all tempos are written as 120 beats/minute, */
/* expressed in microseconds/quarter note     */
int
AbcMidiFile::writeTempo(long tempo)
{
    this->writeByte(0);
    this->writeByte(MidiEvent::meta_event);
    this->writeByte(MidiEvent::set_tempo);
    this->writeByte(3);
    this->writeByte((char)(0xff & (tempo >> 16)));
    this->writeByte((char)(0xff & (tempo >> 8)));
    this->writeByte((char)(0xff & tempo));
    return 0;
}

void
AbcMidiFile::singleNoteTuningChange(int key, float midipitch)
{
    this->writeByte(0);    /* varinum delta_t (time to next event) */
    this->writeByte(0xf0); /* sysex initiation */
    this->writeByte(11);   /* 11 bytes included in sysex */
    this->writeByte(127);  /* universal sysex command (0x7f) */
    this->writeByte(0);    /* device id */
    this->writeByte(8);    /* midi tuning */
    this->writeByte(2);    /* note change */
    this->writeByte(0);    /* program number 0 - 127 */
    this->writeByte(1);    /* only one change */

    unsigned char kk = (unsigned char) 127 & key;
    this->writeByte(kk);  /* MIDI key  0 - 127 */

    int number = (int) midipitch;
    float fraction = midipitch - (float) number;
    if(fraction < 0.f) 
        fraction = -fraction;
    int intfraction = (int) fraction*16384;
    unsigned char xx = 0x7f & number;
    unsigned char yy = intfraction/128;
    unsigned char zz = intfraction % 128;
    yy = 0x7f & yy;
    zz = 0x7f & zz;
    this->writeByte(xx);
    this->writeByte(yy);
    this->writeByte(zz);
    this->writeByte(247); /* 0xf7 terminates sysex command */
}

/*--------------------------------------------------------------------------*/
void
AbcMidiFile::writeHeaderChunk(int format, int ntracks, int division)
{
    int ident = sMagicHDR;    /* Head chunk identifier */
    int length = 6;         /* Chunk length */

    /* individual bytes of the header must be written separately
       to preserve byte order across cpu types :-( */
    this->write32bit(ident);
    this->write32bit(length);
    this->write16bit(format);
    this->write16bit(ntracks);
    this->write16bit(division);
}

void
AbcMidiFile::writeTrackChunk(int track)
{
    int trkhdr = sMagicTRK;

    /* Remember where the length was written, because we don't
        know how long it will be until we've finished writing */
    long offset = ftell(fp); 

    /* Write the track chunk header */
    this->write32bit(trkhdr);
    this->write32bit(0); // <--- filled in later

    this->trackBytesWritten = 0L; /* the header's length doesn't count */

    long endspace = this->helper->writetrack(track);

    /* mf_write End of track meta event */
    this->writeVarLen(endspace);

    this->writeByte(MidiEvent::meta_event);
    this->writeByte(MidiEvent::end_of_track);
    this->writeByte(0);
    
    /* It's impossible to know how long the track chunk will be beforehand,
       so the position of the track length data is kept so that it can
       be written after the chunk has been generated */
    long place_marker = ftell(fp);
    
    /* This method turned out not to be portable because the
       parameter returned from ftell is not guaranteed to be
       in bytes on every machine */
    /* track.length = place_marker - offset - (long) sizeof(track); */

    if(fseek(this->fp, offset, 0) < 0)
        this->helper->midierror("error seeking during final stage of write");

    /* Re-mf_write the track chunk header with right length */
    long trackBytes = this->trackBytesWritten;
    this->write32bit(trkhdr); // <--- not actually necessary
    this->write32bit(trackBytes);

    fseek(this->fp, place_marker, 0);
}
