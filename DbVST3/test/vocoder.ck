

int done;

fun void doit()
{
    DbVST3 vocoder;
    // vocoder.setVerbosity(1);

    vocoder.loadPlugin("TAL-Vocoder-2.vst3");
    <<<"Vocoder loaded,", vocoder.getNumParameters(), "parameters.">>>;
    vocoder.setParameter("Bypass", 0);
    vocoder.setParameter("inputmode", 1);  // L+R
    vocoder.setParameter("sidechain", 0);

    Mix2 mix => vocoder => dac;
    .8 => mix.gain;

    SndBuf buf;
    buf.read("../../PitchTrack/data/obama.wav");
    buf.loop(1);
    buf.rate(1);

    buf => mix.right; // modulator
    SawOsc o => mix.left; // carrier
    100 => o.freq;
    .5 => o.gain;

    for(int i;i<2;i++)
    {
        buf.rate(1 + i * .2);
        buf.phase(0);
        for(int j;j<10;j++)
            1::second =>now;
    }
    1 => done;
}

fun void doitMidi()
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
    vocoder.setVerbosity(1);

    // inputRouting must be set prior to plugin-load
    // left is modulator, right is sidechain/carrier (?)
    // vocoder.setInputRouting("11"); 

    // 1 input event busses
    // Event bus 0: MIDI Input
    // AudioIn bus:Main.0 nchan:2
    // AudioIn bus:Main.1 nchan:0
    // Configuring audio _buses_: in 2 out:1
    // Configure audio _channels_: in 2 out 2

    vocoder.loadPlugin("TAL-Vocoder-2.vst3");
    <<<"Vocoder loaded,", vocoder.getNumParameters(), "parameters.">>>;
    vocoder.setParameter("Bypass", 0);

    Mix2 mix => vocoder => dac;
    SndBuf buf;
    buf.read("../../PitchTrack/data/obama.wav");
    buf.loop(1);
    buf.rate(1);

    // 0 means use its own carrier, then midievents should matter?
    //   without which there should be no output. (no success to date).
    1 => float inputMode; 
    vocoder.setParameter("inputmode", inputMode); 
    vocoder.setParameter("sidechain", 0);
    if(inputMode == 0 && 0)
    {
        buf => mix.right; // modulator, internal carrier
        buf => mix.left;
    }
    else
    {
        buf => mix.right; // modulator
        SawOsc o => mix.left; // carrier
        100 => o.freq;
        .5 => o.gain;
    }

    for(int i;i<1;i++)
    {
        //<<<"Program", i>>>;
        //vocoder.setParameter("Program", i/10.); // doesn't work
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
