# DbVST3 - a chuck-plugin that loads VST3 plugins

## Usage

Like all chugins, all you need to do is instantiate DbVST3
in your ChucK programs and wire it into the audio chain.
VST3 commonly fall into two categories:

1. instruments - which react to midi events and produce audio out.
2. effects - which process audio singals.

````ck
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
````


## Known Issues

* Currently we don't save/stash plugin state. Plugin presets loaded
  by VST3Fiddler cause parameter updates to be stashed in the .chg file.
  Plugins that support preset parameters trigger a complete update of
  parameters when its value is set.
* MIDI events may produce stuck keys.

## See Also

https://cannerycoders.com/docs/Fiddle/topics/vst3.html
