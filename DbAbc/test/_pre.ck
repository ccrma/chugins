public class DoTrack
{
    0 => int verbose;
    NRev reverb => dac;
    0.025 => reverb.mix;
    Wurley w[4];
    .5 => w[0].gain;
    w[0] => reverb;
    .5 => w[1].gain;
    w[1] => reverb;
    .5 => w[2].gain;
    w[2] => reverb;
    .5 => w[3].gain;
    w[3] => reverb;

    fun void go(DbAbc dbf, int track, float speed, int running[])
    {
        int v;
        AbcMsg msg;
        while(dbf.read(msg, track))
        {
            if(msg.when > 0::second)
            {
                // <<<"Track", track, "waiting", msg.when>>>;
                msg.when * speed => now; // speed of 1 is nominal
            }
            msg.status & 0xF0 => int stat;
            if(stat == 0x90) // NOTEON
            {
                // get the pitch and convert to frequencey; set
                if(verbose)
                    <<<"Note-on, pitch", msg.data1, "track", track>>>;
                msg.data1 => Std.mtof => w[v].freq;
                msg.data2/127.0 => w[v].noteOn; // velocity data; note on
                (v+1)%w.size() => v; // cycle the voices
            }
            else
            if(stat == 0x80) // NOTEOFF
            {
                // need to track which voice is associated with which note.  
                // Many instruments force note-off when new note-on arrives.
                if(verbose)
                    <<<"Track", track, "note-off, pitch", msg.data1>>>;
            }
            else
            if(stat == 0xA0) // polyphonic aftertouch
            {
                <<<"Unimplemented polyphonic aftertouch", msg.status, 
                    "track", track>>>;
            }
            else
            if(stat == 0xB0) // control/mode change
            {
                <<<"Unimplemented control/mode polyphonic aftertouch", msg.status, 
                    msg.data1, msg.data2, "track", track>>>;
            }
            else
            if(stat == 0xC0) // program change
            {
                <<<"Unimplemented program change", msg.status, 
                    msg.data1, msg.data2, "track", track>>>;
            }
            else
            if(stat == 0xD0) // channel aftertouch
            {
                <<<"Unimplemented channel aftertouch", msg.status, 
                    msg.data1, msg.data2, "track", track>>>;
            }
            else
            if(stat == 0xE0) // pitch wheel
            {
                <<<"Unimplemented pitch wheel", msg.status, 
                    msg.data1, msg.data2, msg.data3, msg.data4,
                    "track", track>>>;
            }
            else
            if(stat == 0xF0) // misc
            {
                if(msg.meta != -1)
                {
                    if(msg.meta == 0)
                        <<<"MetaEvent sequence number", 
                            msg.data1, msg.data2,
                            "track", track>>>;
                    else
                    if(msg.meta == 1)
                    {
                        <<<"MetaEvent text event", 
                            "'", msg.datastr, "' track", track>>>;
                    }
                    else
                    if(msg.meta == 88)
                    {
                        // 4 values numerator, denominator, XXX, 8
                        <<<"MetaEvent time signature", 
                            msg.data1, msg.data2, msg.data3, msg.data4,
                            "track", track>>>;
                    }
                    else
                    if(msg.meta == 89)
                    {
                        // data3 is #sharps, data4 is minor
                        <<<"MetaEvent key signature", 
                            msg.data1, msg.data2,
                            "track", track>>>;
                    }
                    else
                        <<<"MetaEvent", 
                            msg.data1, msg.data2, msg.data3, msg.data4,
                            "track", track>>>;
                }
                else
                    <<<"AbcEvent unexpected misc", msg.status,
                        msg.data1, msg.data2, msg.data3, msg.data4,
                        "track", track >>>;
                
            }
            else
                <<<"Unexpected abc status", msg.status, "track", track>>>;
        }
        1 -=> running[0];
    }
}
