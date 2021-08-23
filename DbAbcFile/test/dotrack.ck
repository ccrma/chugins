public class DoTrack
{
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

    fun void go(DbAbcFile dbf, int track, float speed, int running[])
    {
        int v;
        MidiMsg msg;
        while(dbf.read(msg, track))
        {
            if(msg.when > 0::second)
            {
                // <<<"Track", track, "waiting", msg.when>>>;
                msg.when * speed => now; // speed of 1 is nominal
            }
                
            if((msg.data1 & 0xF0) == 0x90)
            {
                // NOTEON
                // get the pitch and convert to frequencey; set
                if(track == 2)
                    <<<"Track", track, "note-on, pitch", msg.data2>>>;
                msg.data2 => Std.mtof => w[v].freq;
                // velocity data; note on
                msg.data3/127.0 => w[v].noteOn;
                // cycle the voices
                (v+1)%w.size() => v;
            }
            else
            if((msg.data1 & 0xF0) == 0x80)
            {
                // NOTEOFF - need to track which voice is associated with which
                // note.  Many instruments force note-off when new note-on arrives.
                if(track == 2)
                    <<<"Track", track, "note-off, pitch", msg.data2>>>;
            }
            else
            {
                // MetaEvent..
                // log
                if(msg.data1 == 1) // text annotation (string doesn't fit in MidiMsg)
                    continue;
                else
                if(msg.data1 == 3) // sequence name
                    continue;
                else
                if(msg.data1 == 88)
                    continue; // <<<"Time Signature", track, msg.data2, msg.data3>>>;
                else
                if(msg.data1 == 89)
                    continue; // <<<"Key Signature", track, msg.data2, msg.data3>>>;
                else
                if(msg.data1 == 192)
                {
                    // used to select an instrument program (eg: for drone, test h).
                    <<<"Program change", track, msg.data2, msg.data3>>>;
                    continue; 
                }
                else
                if(msg.data1 == 224)
                {
                    <<<"Pitch wheel", track, msg.data2, msg.data3>>>;
                    continue; 
                }
                else
                    <<<"Track", track, "unhandled", msg.data1>>>;
                
            }
        }
        1 -=> running[0];
    }
}
