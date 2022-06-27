DbAbc dbf;
NRev reverb => dac;
0.025 => reverb.mix;

"../samples/baym_rebin.abc" => dbf.open;

FluidSynth f[2];
f[0] => reverb;
f[1] => reverb;
1.5 => f[0].gain;
1.5 => f[1].gain;

"../../FluidSynth/HS_African_Percussion.sf2" => string soundfont;
soundfont => f[0].open;
soundfont => f[1].open;

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

<<< "Rewind, speedup" >>>;
dbf.rewind();

0 => running;
.5 => timeScale; // < 1 is faster, > 2 is slower
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
    AbcMsg msg;
    while(dbf.read(msg, track))
    {
        if(msg.when > 0::second)
        {
            // <<<"Track", track, "waiting", msg.when>>>;
            msg.when * speed => now; // speed of 1 is nominal
        }
            
        msg.status & 0xF0 => int stat;
        if(stat == 0x90)
        {
            // NOTEON
            // get the pitch and convert to frequencey; set
            // <<<"Track", track, "note-on, pitch", msg.data2>>>;
            if(track == 1)
            {
                msg.data1 => Std.mtof => w[v].freq;
                // velocity data; note on
                msg.data2/127.0 => w[v].noteOn;
                // cycle the voices
                (v+1)%w.size() => v;
            }
            else
            if(track == 2)
            {
                f[v].noteOn(msg.data1, msg.data1, msg.status & 0x0F);
                (v+1)%f.size() => v;
            }
        }
        else
        if(stat == 0x80)
        {
            // NOTEOFF - need to track which voice is associated with which
            // note.  Many instruments force note-off when new note-on arrives.
        }
    }
    running--;
}