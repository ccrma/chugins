DbAbcFile dbf;
NRev reverb => dac;
0.025 => reverb.mix;


"../samples/demo1.abc" => string file;
file => dbf.open;
<<< file, "nchan", dbf.numTracks() >>>;

Wurley w[4];
.5 => w[0].gain;
w[0] => reverb;
.5 => w[1].gain;
w[1] => reverb;
.5 => w[2].gain;
w[2] => reverb;
.5 => w[3].gain;
w[3] => reverb;

0 => int running;

1 => float timeScale;
for(0=>int t; t < dbf.numTracks(); t++)
{
    running++;
    spork ~ doTrack(t, timeScale); // t==1 ? s : f);
}

while(running > 0)
    1::second => now;

dbf.close();

<<<"ring">>>;
2::second => now;

fun void doTrack(int track, float speed)
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
            // <<<"Track", track, "note-on, pitch", msg.data2>>>;
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
        }
        else
        {
            // MetaEvent..
            // log
            if(msg.data1 == 1) // text annotation (string doesn't fit in MidiMsg)
                continue;
            else
            if(msg.data1 == 88)
                continue; // <<<"Time Signature", track, msg.data2, msg.data3>>>;
            else
            if(msg.data1 == 89)
                continue; // <<<"Key Signature", track, msg.data2, msg.data3>>>;
            else
                <<<"Track", track, "unhandled", msg.data1>>>;
            
        }
    }
    running--;
}
