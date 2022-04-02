
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

    DbMdaDX10 x => dac;
    x.selectPreset(20);
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

15::second => dur d;
spork ~ doit(1, d);

d => now;

<<< "Done" >>>;

