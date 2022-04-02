## DbMdaDX10 chugin

The DbMdaDX10 chugin is a port of Paul Keller's open-source
vst plugins here: http://mda.smartelectronix.com/ and
here: https://sourceforge.net/projects/mda-vst. It is
now GPL2 and MIT licensed.

### API

 `printPresets(int)` prints all known presets to the console.

 `selectPreset(int)` selects indexed preset.

 `printParams()` prints all know parameters to the console.

 `int getNumParams()` returns the number of parameters.

 `setParamValue(int i, float value)` sets the indexed parameter to value.

 `handleMidiEvent(int status, int data1, int data2)` sends midi events to the plugin.

 `noteOn(int mnote, float vel)` sends a note-on event to the plugin.

 `noteOff(int mnote, float vel)` sends a note-off event to the plugin.

### Example

test.ck

 ```ck
DbMdaDX10 inst => dac;

<<<"Presets", "">>>;
inst.printPresets();

for(int j;j<inst.getNumPresets();j++)
{
    <<<"Preset", j>>>;
    inst.selectPreset(j);
    for(int i;i<20;i++)
    {
        Math.random2(40, 60) => int mnote;
        inst.noteOn(mnote, 1.);
        .1::second => now;
        inst.noteOff(mnote, 1.);
    }
}
```