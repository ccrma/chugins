fun void doit(int id, dur maxwait)
{
    MidiIn min;
    MidiMsg msg;

    if(!min.open(0))
    {
        <<< "MIDI device 0 can't be opened." >>>;
        me.exit();
    }

    string instname;
    -1 => int mod; // default

    "mda-vst3.vst3" => instname;
    14 => mod; // 10:DX, 14:JX

    // "Movementron/Resources/Movementron.vst3" >> instname;

    <<< "MIDI device:", min.num(), " -> ", min.name() >>>;
    <<< "Listening for midi events for", maxwait >>>;
    <<< "Instrument", instname, "module", mod >>>;

    //DbVST3 filt;
    // filt.loadPlugin("ValhallaSupermassive.vst3");
    // filt.loadPlugin("adelay.vst3");
    //filt.loadPlugin("hostchecker.vst3");

    DbVST3 inst;
    inst.setVerbosity(2);
    inst.loadPlugin(instname);
    if(mod != -1)
        inst.selectModule(mod);

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
