fun void doit(int id, dur maxwait)
{
    MidiIn min;
    MidiMsg msg;

    if(!min.open(0))
    {
        <<< "MIDI device 0 can't be opened." >>>;
        me.exit();
    }

    <<< "MIDI device:", min.num(), " -> ", min.name() >>>;
    <<< "Listening for midi events for", maxwait >>>;

    DbVST3 x => dac;
    x.loadPlugin("LABS (64 Bit).vst3");
    now => time start;
    while(now - start < maxwait)
    {
        min => now;
        while(min.recv(msg))
        {
            <<< "chuck midievent" , msg.data1 >>>;
            x.midiEvent(msg);
        }
    }
    <<< "shred", id, "done" >>>;
}

<<<"MIDI for 15 seconds">>>;
15::second => dur d;
spork ~ doit(1, d);

d => now;

<<< "Done" >>>;
