// run: chuck abcCC.ck
//  midi CC's can be expressed in-line with abcnotes and happen
//  at the right time.
"X:1\n" +
"M:2/4\n" +
"L:1/4\n" +
"Q:1/4=80\n"+
"K:C\n"+
"|:\n"+
"%%MIDI control 7 0\n"+
"|C//\n" +
"%%MIDI control 7 127\n"+
"C//\n" +
"%%MIDI control 7 0\n"+
"C//\n" +
"%%MIDI control 7 127\n"+
"C//\n" +
"%%MIDI control 7 0\n"+
"C//\n" +
"%%MIDI control 7 127\n"+
"C//\n" +
"%%MIDI control 7 0\n"+
"C//\n" +
"%%MIDI control 7 127\n"+
"C//\n" +
"|A A |A A :|\n" +
"Q:1/4=75\n"+
"K:C transpose=-2\n"+
"|:\n"+
"|[I:MIDI=control 7 0] C// [I:MIDI=control 7 127] C//\n" +
" [I:MIDI=control 7 0] C// [I:MIDI=control 7 127] C//\n" +
" [I:MIDI=control 7 0] C// [I:MIDI=control 7 127] C//\n" +
" [I:MIDI=control 7 0] C// [I:MIDI=control 7 127] C//\n" +
"|A A |A A :|\n"
=> string tune;

DoTrack doit;
DbAbc abc;

tune => abc.open => int success;
if(success == 0)
{
    <<<"problem with abc string">>>;
    me.exit();
}

<<< "abcstring", "nchan", abc.numTracks() >>>;

[0] @=> int running[];
1 => float timeScale;
for(0=>int t; t < abc.numTracks(); t++)
{
    1 +=> running[0];
    spork ~ doit.go(abc, t, timeScale, running); // t==1 ? s : f);
}
while(running[0] > 0)
    1::second => now;
abc.close();
<<<"ring">>>;
2::second => now;

class DoTrack
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

    fun void go(DbAbc abc, int track, float speed, int running[])
    {
        int v, lastVoice, lastPitch;
        AbcMsg msg;
        while(abc.read(msg, track))
        {
            if(msg.when > 0::second)
            {
                // <<<"Track", track, "waiting", msg.when>>>;
                msg.when * speed => now; // speed of 1 is nominal
            }
            msg.status & 0xF0 => int stat;
            if(stat == 0x90) // NOTEON
            {
                // get the pitch and convert to frequency; set
                if(verbose)
                    <<<"Note-on, pitch", msg.data1, "track", track>>>;
                msg.data1 => lastPitch => Std.mtof => w[v].freq;
                msg.data2/127.0 => w[v].noteOn; // velocity data; note on
                v => lastVoice; // used by toy pitchbend
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
                <<<"MIDI polyphonic aftertouch", msg.status, 
                    "track", track>>>;
            }
            else
            if(stat == 0xB0) // control/mode change
            {
                <<<"MIDI control/mode change", msg.status, 
                    msg.data1, msg.data2, "track", track>>>;
            }
            else
            if(stat == 0xC0) // program change
            {
                <<<"MIDI program change", msg.status, 
                    msg.data1, msg.data2, "track", track>>>;
            }
            else
            if(stat == 0xD0) // channel aftertouch
            {
                <<<"MIDI channel aftertouch", msg.status, 
                    msg.data1, msg.data2, "track", track>>>;
            }
            else
            if(stat == 0xE0) // pitch wheel
            {
                // a more sophisticated multi-voice implementation would be
                // need to support bending chords.
                msg.data1  + (msg.data2 << 7) => int wheel; // numbers between 0 and 16383
                (wheel - 8192) / 8192. => float pct;
                lastPitch + 2 * pct => Std.mtof => w[lastVoice].freq;
                if(verbose)
                    <<<"MIDI pitch wheel", wheel, pct, "track", track>>>;
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
                        <<<"MetaEvent text", 
                            "'", msg.datastr, "' track", track>>>;
                    }
                    else
                    if(msg.meta == 3)
                    {
                        <<<"MetaEvent title", 
                            "'", msg.datastr, "' track", track>>>;
                    }
                    else
                    if(msg.meta == 81)
                    {
                        // MetaEvent tempochange 9 39 192 track 2 
                        // The MIDI set tempo meta message sets the tempo of a MIDI 
                        // sequence in terms of microseconds per quarter note.
                        (msg.data1 <<16) + (msg.data2 << 8) + msg.data3 => int tempo;
                        60 * 1000000.0 / tempo => float bpm; 
                        abc.setBPM(bpm);
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
                    <<<"Abc unimplemented MIDI/misc", msg.status,
                        msg.data1, msg.data2, msg.data3, msg.data4,
                        "track", track >>>;
                
            }
            else
                <<<"Abc unexpected status", msg.status, "track", track>>>;
        }
        1 -=> running[0];
    }
}