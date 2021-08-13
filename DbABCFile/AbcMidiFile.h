#ifndef AbcMidiFile_h
#define AbcMidiFile_h

class AbcMidiFile
{
public:
    static const int magicHD;
    static const int magicRK;

    AbcMidiFile();
    ~AbcMidiFile();

    class IMidiWriter
    {
    public:
        virtual ~IMidiWriter() {};
        virtual int writetrack(int xtrack) = 0;
    };

    // atomic operation (begin+end)
    int write(IMidiWriter *, int format, int ntracks, int division);

    // methods available for calling during writetrack callback.
    int writeMetaEvent(long delta_time, int type, char const *data, int size);
    int writeTempo(long tempo);
    int writeMidiEvent(long delta_time, int type, int chan, char const *data, int size);
    void singleNoteTuningChange(int key, float midipitch);

    enum MidiCodes
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

    enum MidiControllers
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

    enum MidiParams
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
    enum MidiEvent
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

private:
    IMidiWriter *writer;

    unsigned char lowerbyte(int x) 
    {
        return static_cast<unsigned char>(x & 0xff);
    }

    unsigned char upperbyte(int x) 
    {
        return static_cast<unsigned char>((x & 0xff00)>>8);
    }

private:
    long readvarinum();
    long read32bit();
    long to32bit();
    int read16bit();
    int to16bit();
    char *msg();
    int skiptrack();

    int readtrack();
    void readheader();
    void badbyte();
    void metaevent();
    void sysex();
    void chanmessage();
    void msginit();
    void msgadd();
    void biggermsg();
    void mf_write_track_chunk();
    void mf_write_header_chunk();
    void WriteVarLen();
    void write32bit();
    void write16bit();
    int msgleng();
    int eputc();

private:
    int ntrks;
    int nomerge;
    long currtime;
    long toberead;
    long bytesread;
    long byteswritten;
};

#endif