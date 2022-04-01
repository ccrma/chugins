int done;
SndBuf buf;
Mix2 mix;
SqrOsc osc;
DbVST3 vocoder;

// vocoder.setVerbosity(1);
vocoder.loadPlugin("TAL-Vocoder-2.vst3");
while(!vocoder.ready())
    1::ms => now;
vocoder.selectModule(0);
<<<"Vocoder loaded, has", vocoder.getNumParameters(), "parameters.">>>;
vocoder.setParameter("Bypass", 0);
vocoder.setParameter("inputmode", 1);  // 0 means use its own carrier, 1 means L+R
vocoder.setParameter("sidechain", 0);
vocoder.setParameter("Program", 0.4);

// WIP:
// inputRouting must be set prior to plugin-load
// left is modulator, right is sidechain/carrier (?)
// vocoder.setInputRouting("11"); 
// 1 input event busses
// Event bus 0: MIDI Input
// AudioIn bus:Main.0 nchan:2
// AudioIn bus:Main.1 nchan:0
// Configuring audio _buses_: in 2 out:1
// Configure audio _channels_: in 2 out 2

mix => vocoder => dac;
.8 => mix.gain;

buf.read("../../PitchTrack/data/obama.wav");
buf.loop(1);
buf.rate(1);
buf => mix.right; // modulator

osc => mix.left; // carrier
100 => osc.freq;
.5 => osc.gain;
.5 => buf.gain;

fun void doit()
{
    for(int i;i<10;i++)
    {
        Math.mtof(40+i) => osc.freq;
        10::second => now;
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

spork ~ doit();
// spork ~ doitMidi();

int seconds;
while(!done)
{
    <<<"tick", seconds++>>>;
    1::second => now;
}
