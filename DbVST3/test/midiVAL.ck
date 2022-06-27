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

    DbVST3 y;
    DbVST3 x => y => dac;
    // DbVST3 x => dac;

    x.loadPlugin("mda-vst3.vst3");
    // x.selectModule(14); // JX synth
    x.selectModule(12); // EPiano
    y.loadPlugin("ValhallaSupermassive.vst3");
    y.setParameter("Bypass", 0);

    now => time start;
    while(now - start < maxwait)
    {
        min => now;
        while(min.recv(msg))
        {
            // <<< "chuck midievent" , msg.data1 >>>;
            x.midiEvent(msg);
        }
    }
    <<< "shred", id, "done" >>>;
}

spork ~ doit(1, 45::second);

45::second => now;

<<< "Done" >>>;