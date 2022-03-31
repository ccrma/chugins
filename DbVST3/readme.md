# DbVST3 - a ChucK-plugin that loads VST3 plugins

## Usage

Like all chugins, all you need to do is instantiate DbVST3 in your ChucK 
programs and wire it into the audio chain.  

VST3 commonly fall into two categories:

1. instruments - which react to midi events and produce audio out.
2. effects - which process audio signals.

The type of the plugin determines how to connect it into your processing
network and shown in this example.

## Example

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
    while(!x.ready()) // loadplugin, selectModule are both asynchronous
        1::ms => now;

    y.loadPlugin("ValhallaSupermassive.vst3");
    while(!y.ready()) // loadplugin, selectModule are both asynchronous
        1::ms => now;

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

## Interface

| Methods                                       | Description                                                                                                              |
| :-------------------------------------------- | :----------------------------------------------------------------------------------------------------------------------- |
| `void loadPlugin(string nm)`                  | Triggers asynchronous loading of the named vst3 plugin.                                                                  |
| `void selectModule(int)`                      | Triggers asynchronous loading of the identified plugin module.  Most plugins have a single module, so this isn't needed. |
| `int getNumModules()`                         | Returns the number of modules in the plugin. Usually == 1.                                                               |
| `int printModules()`                          | Dumps the names of all modules in the current plugin.                                                                    |
| `int ready()`                                 | Returns 1 when asynchronous load is successful. -1 means an error was encountered and 0 means still pondering.           |
| `int getNumParameters()`                      | Returns the number of parameters in the current module.                                                                  |
| `string getParameterName(int i)`              | Returns the name associated with the indexed parameter.                                                                  |
| `void setParameter(string name, float value)` | Sets the named parameter to value.                                                                                       |
| `void setParameter(int index, float value)`   | Sets the indexed parameter to value.                                                                                     |
| `float getParameter(int index)`               | Returns the normalized parameter value for the indexed parameter.                                                        |
| `void noteOn(int midiNote, float velocity)`   | Triggers a note-on event for instruments.                                                                                |
| `void noteOff(int midiNote, float velocity)`  | Triggers a note-of event for instruments.                                                                                |
| `void midiEvent(MidiMsg m)`                   | Delivers midi events to effect or instrument.                                                                            |

## Known Issues

* Currently we don't save/stash plugin state. Plugin presets loaded
  by VST3Fiddler cause parameter updates to be stashed in the Fiddle's .chg 
  file. Plugins that support preset parameters trigger a complete update of
  parameters when its value is set.
* MIDI events may produce stuck keys.
* Some plugins, eg Spitfire LABS, don't work due to unresolved issues with
  their security features. 

## See Also

https://cannerycoders.com/docs/Fiddle/topics/vst3.html
