DbAbcFile dbf;
MidiMsg msg;
NRev reverb => dac;
0.025 => reverb.mix;

"../samples/boys.abc" => dbf.open;

doTrack(1, 100);

fun void doTrack(int track, float speed)
{
    // hack polyphony
    Wurley s[4];
    // for each voice
    for( int i; i < s.size(); i++ )
    {
        .5 => s[i].gain;
        s[i] => reverb;
    }

    // voice number for quick polyphony
    int v;
    while(dbf.read(msg, 1))
    {
        <<< "msg", msg.when, msg.data1, msg.data2, msg.data3 >>>;
        if( msg.when > 0::second )
                msg.when * speed => now; // speed of 1 is nominal
            
        // catch NOTEON messages
        if( (msg.data1 & 0xF0) == 0x90 && msg.data2 > 0 && msg.data3 > 0 )
        {
            // get the pitch and convert to frequencey; set
            msg.data2 => Std.mtof => s[v].freq;
            // velocity data; note on
            msg.data3/127.0 => s[v].noteOn;
            // cycle the voices
            (v+1)%s.size() => v;

            // log
            cherr <= "NOTE ON track:" <= track <= " pitch:" <= msg.data2 <=" velocity:" <= msg.data3 <= IO.newline(); 
        }
        // other messages
        else
        {
            // log
            // cherr <= "----EVENT (unhandled) track:" <= track <= " type:" <= (msg.data1&0xF0)
            //      <= " data2:" <= msg.data2 <= " data3:" <= msg.data3 <= IO.newline(); 
        }
    }
}


