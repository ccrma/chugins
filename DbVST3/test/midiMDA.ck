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
    x.loadPlugin("mda-vst3.vst3");
    while(!x.ready())
        1::ms => now;
    x.selectModule(10);
    while(!x.ready())
        1::ms => now;
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

15::second => dur d;
spork ~ doit(1, d);

d => now;

<<< "Done" >>>;
