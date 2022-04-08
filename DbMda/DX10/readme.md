## DbMdaDX10 chugin

The DbMdaDX10 chugin is a port of Paul Kellett's open-source
vst plugin here: http://mda.smartelectronix.com/ and
here: https://sourceforge.net/projects/mda-vst. It is
now GPL2 and MIT licensed.

mda-DX10 emulates a subset of the estimable Yamaha DX10 synthesizer.
It comes with 32 convenient presets which you can select to quickly
explore its possibilities.

### API

#### Presets

`printPresets(int)` prints all known presets to the console.

`int getNumPresets()` returns the number of presets.
 
`selectPreset(int i)` selects the indexed preset.

| Index | Name           |
| ----: | :------------- |
|     0 | Bright E.Piano |
|     1 | Jazz E.Piano   |
|     2 | E.Piano Pad    |
|     3 | Fuzzy E.Piano  |
|     4 | Soft Chimes    |
|     5 | Harpsichord    |
|     6 | Funk Clav      |
|     7 | Sitar          |
|     8 | Chiff Organ    |
|     9 | Tinkle         |
|    10 | Space Pad      |
|    11 | Koto           |
|    12 | Harp           |
|    13 | Jazz Guitar    |
|    14 | Steel Drum     |
|    15 | Log Drum       |
|    16 | Trumpet        |
|    17 | Horn           |
|    18 | Reed 1         |
|    19 | Reed 2         |
|    20 | Violin         |
|    21 | Chunky Bass    |
|    22 | E.Bass         |
|    23 | Clunk Bass     |
|    24 | Thick Bass     |
|    25 | Sine Bass      |
|    26 | Square Bass    |
|    27 | Upright Bass 1 |
|    28 | Upright Bass 2 |
|    29 | Harmonics      |
|    30 | Scratch        |
|    31 | Syn Tom        |

#### Parameters

`printParams()` prints all known parameters and values to the console.

`float getParam(int i)` returns the value of the indexed parameter.

`setParam(int i, float value)` sets the indexed parameter to value.

| Index | Parameter | Default |
| ----: | :-------- | :------ |
|     0 | Attack    | 0.000   |
|     1 | Decay     | 0.650   |
|     2 | Release   | 0.441   |
|     3 | Coarse    | 0.842   |
|     4 | Fine      | 0.329   |
|     5 | ModInit   | 0.230   |
|     6 | ModDec    | 0.800   |
|     7 | ModSus    | 0.050   |
|     8 | ModRel    | 0.800   |
|     9 | ModVel    | 0.900   |
|    10 | Vibrato   | 0.000   |
|    11 | Octave    | 0.500   |
|    12 | FineTune  | 0.500   |
|    13 | WaveForm  | 0.447   |
|    14 | ModThru   | 0.000   |
|    15 | LFORate   | 0.414   |

#### Events

`midiEvent(MidiMsg msg)` sends midi events to the plugin.

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
