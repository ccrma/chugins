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

    //DbVST3 filt;
    // filt.loadPlugin("ValhallaSupermassive.vst3");
    // filt.loadPlugin("adelay.vst3");
    //filt.loadPlugin("hostchecker.vst3");

    DbVST3 inst;
    inst.loadPlugin("mda-vst3.vst3");
    inst.selectModule(10); // DX 10 synth
    // inst.loadPlugin("Movementron/Resources/Movementron.vst3");
    inst.selectModule(0);

    // inst => filt => dac;
    inst => dac;
    now => time start;
    while(now - start < maxwait)
    {
        min => now;
        while(min.recv(msg))
        {
            // <<< "chuck midievent" , msg.data1 >>>;
            inst.midiEvent(msg);
        }
    }
    <<< "shred", id, "done" >>>;
}

spork ~ doit(1, 45::second);

45::second => now;

<<< "Done" >>>;
