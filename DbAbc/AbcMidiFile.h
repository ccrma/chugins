#ifndef AbcMidiFile_h
#define AbcMidiFile_h

#include <cstdio>
#include <vector>

struct MidiEvent
{
    MidiEvent()
    {
    }

    MidiEvent(const MidiEvent &rhs)
    {
        this->evt = rhs.evt;
        this->metaType = rhs.metaType;
        this->dur = rhs.dur;
        this->data = rhs.data;
    }

    MidiEvent(long dt, int type, unsigned char const *data, int size)
    {
        this->dur = dt;
        this->evt = type;
        this->metaType = -1;
        this->data.assign(data, data+size);
    }

    MidiEvent(long dt, int meta, int type, unsigned char const *data, int size)
    {
        this->dur = dt;
        this->evt = meta;
        this->metaType = type;
        this->data.assign(data, data+size);
    }

    int evt, metaType;
    long dur;
    std::vector<unsigned char> data; // 0 means no event - ie: a feature didn't map to an event

    enum Codes
    {
        note_off =         	0x80,
        note_on =          	0x90,
        poly_aftertouch =  	0xa0,
        control_change =   	0xb0,
        program_chng =     	0xc0,
        channel_aftertouch =0xd0,
        pitch_wheel =     	0xe0,
        system_exclusive =  0xf0,
    };

    enum Controllers
    {
        damper_pedal =      0x40,
        portamento =	    0x41,
        sostenuto =	        0x42,
        soft_pedal =	    0x43,
        general_4 =         0x44,
        hold_2 =		    0x45,
        general_5 =	        0x50,
        general_6 =	        0x51,
        general_7 =	        0x52,
        general_8 =	        0x53,
        tremolo_depth =     0x5c,
        chorus_depth =	    0x5d,
        detune =		    0x5e,
        phaser_depth =      0x5f,
    };

    enum Params
    {
        /* parameter values */
        data_inc =	        0x60,
        data_dec =	        0x61,

        /* parameter selection */
        non_reg_lsb =	    0x62,
        non_reg_msb =       0x63,
        reg_lsb =           0x64,
        reg_msb =		    0x65,
    };

    /* Standard MIDI Files meta event definitions */
    enum Event
    {
        meta_event =		0xFF,
        sequence_number = 	0x00,
        text_event =		0x01,
        copyright_notice = 	0x02,
        sequence_name =    	0x03,
        instrument_name = 	0x04,
        lyric =	        	0x05,
        marker =			0x06,
        cue_point =		    0x07,
        channel_prefix =	0x20,
        end_of_track =		0x2f,
        set_tempo =		    0x51,
        smpte_offset =		0x54,
        time_signature =	0x58,
        key_signature =		0x59,
        sequencer_specific = 0x74,
    };
};

class IMidiWriter
{
public:
    virtual ~IMidiWriter() {}

    virtual int writeMetaEvent(long delta_time, int type, unsigned char const *data, int size) = 0;
    virtual int writeMidiEvent(long delta_time, int type, int chan, unsigned char const *data, int size) = 0;
    virtual int writeTempo(long tempo) = 0;
    virtual void singleNoteTuningChange(int key, float midipitch) {}
    /* Write tempo */
    /* all tempos are written as 120 beats/minute, */
    /* expressed in microseconds/quarter note     */

};

class AbcMidiFile : public IMidiWriter
{
public:
    AbcMidiFile();
    ~AbcMidiFile();

    class IFileHelper
    {
    public:
        virtual ~IFileHelper() {};

        /* invoked by AbcMidiFile via write --- */
        virtual void midierror(char const *) = 0; 
        virtual long writetrack(int xtrack) = 0;
        virtual void writetempotrack() {}; // optional - for format: 1
    };

    // write is a synchronous, atomic operation (open+write+close)
    //    IMidiWriter->writetrack is called to emit the components
    //    of a single track via our write methods.
    //
    // format
    //      0 : single multi-channel track
    //      1 : multiple simultaneous track
    //      2 : one ore more sequentially independent single track patterns
    // division -  can represent two things, depending on whether it is 
    //      positive or negative         (bit 15 set or not).  If bit 15 
    //      of division is zero, bits 14 through 0 represent the number of 
    //      delta-time "ticks" which make up a quarter note.  If bit  15 of
    //      division  is  a one, delta-times in a file correspond to
    //      subdivisions of a second similiar to  SMPTE  and  MIDI
    //      time code.  In  this format bits 14 through 8 contain
    //      one of four values - 24, -25, -29, or -30,
    //      corresponding  to  the  four standard  SMPTE and MIDI
    //      time code frame per second formats, where  -29
    //      represents  30  drop  frame.   The  second  byte
    //      consisting  of  bits 7 through 0 corresponds the the
    //      resolution within a frame.  Refer the Standard MIDI
    //      Files 1.0 spec for more details.
    int write(IFileHelper *, FILE *fp, int format, int ntracks, int division);

    // methods available for calling during writetrack callback.
    int writeTempo(long tempo) override;
    int writeMetaEvent(long delta_time, int type, unsigned char const *data, int size) override;
    int writeMidiEvent(long delta_time, int type, int chan, unsigned char const *data, int size) override;
    void singleNoteTuningChange(int key, float midipitch) override;


private:
    IFileHelper *helper;
    FILE *fp;

    unsigned char lowerbyte(int x) 
    {
        return static_cast<unsigned char>(x & 0xff);
    }

    unsigned char upperbyte(int x) 
    {
        return static_cast<unsigned char>((x & 0xff00)>>8);
    }

    /*
     * write8bit()
     * write32bit()
     * write16bit()
     *
     * These routines are used to make sure that the byte order of
     * the various data types remains constant between machines. This
     * helps make sure that the code will be portable from one system
     * to the next. 
     *
     */
    int writeByte(int c)
    {
        if(this->fp)
        {
            int r = fputc(c, this->fp);
            if(r != -1)
            {
                this->numBytesWritten++;
                this->trackBytesWritten++;
            }
            return r;
        }
        else
            return -1; // EOF
    }

    void write16bit(int data)
    {
        if(this->fp)
        {
            this->writeByte((char)((data >> 8 ) & 0xff));
            this->writeByte((char)(data & 0xff));
        }
    }

    void write32bit(int data)
    {
        if(this->fp)
        {
            this->writeByte((char)((data >> 24) & 0xff));
            this->writeByte((char)((data >> 16) & 0xff));
            this->writeByte((char)((data >> 8 ) & 0xff));
            this->writeByte((char)(data & 0xff));
        }
    }

    void writeVarLen(long value) // 1, 2, 3, 4 bytes
    {
        if(this->fp)
        {
            long buffer = value & 0x7f;
            while((value >>= 7) > 0)
            {
                buffer <<= 8;
                buffer |= 0x80;
                buffer += (value & 0x7f);
            }
            while(1)
            {
                this->writeByte((char)(buffer & 0xff));
                if(buffer & 0x80)
                    buffer >>= 8;
                else
                    break;
            }
        }
    }

private:
    void writeHeaderChunk(int format, int ntracks, int division);
    void writeTrackChunk(int track);

    int msgleng();

private:
    int ntrks;
    int nomerge;
    long currtime;
    long numBytesWritten;
    long trackBytesWritten;
};

#endif
