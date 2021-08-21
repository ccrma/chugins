DbAbcFile dbf;
NRev reverb => dac;
0.025 => reverb.mix;


// boys produces 3 channels: 
// 0 is tempo-channel (no notes)
// 1 is tune
// 2 is drums (two pitches)
//
"../samples/boys.abc" => dbf.open;

FluidSynth f[2];
"../../FluidSynth/HS_African_Percussion.sf2"  => f[0].open;
"../../FluidSynth/HS_African_Percussion.sf2"  => f[1].open;
f[0] => reverb;
f[1] => reverb;
1.5 => f[0].gain;
1.5 => f[1].gain;

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

for(0=>int t; t < dbf.numTracks(); t++)
{
    running++;
    spork ~ doTrack(t, 30); // t==1 ? s : f);
}
// doTrack(1, 70);

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
            if(track == 1)
            {
                msg.data2 => Std.mtof => w[v].freq;
                // velocity data; note on
                msg.data3/127.0 => w[v].noteOn;
                // cycle the voices
                (v+1)%w.size() => v;
            }
            else
            if(track == 2)
            {
                f[v].noteOn(msg.data2, msg.data3, msg.data1 & 0x0F);
                (v+1)%f.size() => v;
            }
        }
        else
        if((msg.data1 & 0xF0) == 0x80)
        {
            // NOTOFF - need to track which voice is associated with which
            // note
        }
        else
        {
            // log
            <<<"Track", track, "unhandled", msg.data1>>>;
        }
    }
    running--;
}


// writeTempo: 200000
// ----EVENT (unhandled) track:0 type:0 data2:110 data3:111
// ----EVENT (unhandled) track:0 type:80 data2:0 data3:1
// ----EVENT (unhandled) track:0 type:80 data2:6 data3:3
// MidiEvent needs more data 10
// ----EVENT (unhandled) track:1 type:0 data2:110 data3:111
// warning: Track 1 Bar -858993460 has -858993460/-858993460 units instead of 6
// MidiEvent needs more data 10
// ----EVENT (unhandled) track:2 type:0 data2:110 data3:111
// ----EVENT (unhandled) track:2 type:80 data2:6 data3:3