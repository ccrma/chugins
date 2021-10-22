

int done;
fun void doit()
{
    MidiIn min;
    MidiMsg msg;
    if(!min.open(0))
    {
        <<< "MIDI device 0 can't be opened." >>>;
        me.exit();
    }
    <<< "MIDI device:", min.num(), " -> ", min.name() >>>;

    DbVST3 vocoder;
    vocoder.setVerbosity(3);

    // 1 input event busses
    // Event bus 0: MIDI Input
    // AudioIn bus:Main.0 nchan:2
    // AudioIn bus:Main.1 nchan:0
    // Configuring audio _buses_: in 2 out:1
    // Configure audio _channels_: in 2 out 2

    vocoder.loadPlugin("TAL-Vocoder-2.vst3");
    <<<"Vocoder loaded,", vocoder.getNumParameters(), "parameters.">>>;
    vocoder.setParameter("Bypass", 0);

    0 => float inputMode; // 0 means use its own carrier, then midievents matter?
    vocoder.setParameter("inputmode", 1); 

    Mix2 mix => vocoder => dac;
    SndBuf buf 
    buf.read("../../PitchTrack/data/obama.wav");
    buf.loop(1);
    buf.rate(1);

    if(inputMode == 0)
    {
        buf => mix.right; // modulator, internal carrier
        buf => mix.left;
    }
    else
    {
        buf => mix.right; // modulator
        SqrOsc o => mix.left; // carrier
        .5 => o.gain;
    }

    for(int i;i<10;i++)
    {
        <<<"Program", i>>>;
        vocoder.setParameter("Program", i/10.);
        0 => int nevents;
        while(nevents < 8)
        {
            min => now; // <----------------
            while(min.recv(msg))
            {
                <<< "chuck midievent", msg.data1, nevents++ >>>;
                vocoder.midiEvent(msg);
            }
        }
    }
    1 => done;
    <<<"Done">>>;
}

// spork ~ timeKeeper();
spork ~ doit();

int seconds;
while(!done)
{
    <<<"tick", seconds++>>>;
    1::second => now;
}
