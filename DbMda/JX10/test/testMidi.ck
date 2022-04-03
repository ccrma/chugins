
fun void doit(int id, dur maxwait)
{
    MidiIn min;
    MidiMsg msg;

    if(!min.open(id))
    {
        <<< "MIDI device", id, "can't be opened." >>>;
        me.exit();
    }

    <<< "MIDI device:", min.num(), " -> ", min.name() >>>;
    <<< "Listening for midi events for", maxwait >>>;

    DbMdaJX10 x => dac;
    // x.selectPreset(20);
    now => time start;
    while(now - start < maxwait)
    {
        min => now;
        while(min.recv(msg))
        {
            x.midiEvent(msg);
        }
    }
    <<< "shred", id, "done" >>>;
}

45::second => dur d;
spork ~ doit(2, d);

d => now;

<<< "Done" >>>;

