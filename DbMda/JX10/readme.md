## DbMdaJX10 chugin

The DbMdaJX10 chugin is a port of Paul Kellett's open-source
vst plugin here: http://mda.smartelectronix.com/ and
here: https://sourceforge.net/projects/mda-vst. It is
now GPL2 and MIT licensed.

mda-JX10 emulates a subset of the vintage Roland JX10 synthesizer.
It comes with 52 convenient presets which you can modify to suit
your tastes.

### API

#### Presets

`printPresets(int)` prints all known presets to the console.

`int getNumPresets()` returns the number of presets.
 
`selectPreset(int i)` selects the indexed preset.

| Preset Id | Name                   |
| --------: | :--------------------- |
|         0 | 5th Sweep Pad          |
|         1 | Echo Pad [SA]          |
|         2 | Space Chimes [SA]      |
|         3 | Solid Backing          |
|         4 | Velocity Backing [SA]  |
|         5 | Rubber Backing [ZF]    |
|         6 | 808 State Lead         |
|         7 | Mono Glide             |
|         8 | Detuned Techno Lead    |
|         9 | Hard Lead [SA]         |
|        10 | Bubble                 |
|        11 | Monosynth              |
|        12 | Moogcury Lite          |
|        13 | Gangsta Whine          |
|        14 | Higher Synth [ZF]      |
|        15 | 303 Saw Bass           |
|        16 | 303 Square Bass        |
|        17 | Analog Bass            |
|        18 | Analog Bass 2          |
|        19 | Low Pulses             |
|        20 | Sine Infra-Bass        |
|        21 | Wobble Bass [SA]       |
|        22 | Squelch Bass           |
|        23 | Rubber Bass [ZF]       |
|        24 | Soft Pick Bass         |
|        25 | Fretless Bass          |
|        26 | Whistler               |
|        27 | Very Soft Pad          |
|        28 | Pizzicato              |
|        29 | Synth Strings          |
|        30 | Synth Strings 2        |
|        31 | Leslie Organ           |
|        32 | Click Organ            |
|        33 | Hard Organ             |
|        34 | Bass Clarinet          |
|        35 | Trumpet                |
|        36 | Soft Horn              |
|        37 | Brass Section          |
|        38 | Synth Brass            |
|        39 | Detuned Syn Brass [ZF] |
|        40 | Power PWM              |
|        41 | Water Velocity [SA]    |
|        42 | Ghost [SA]             |
|        43 | Soft E.Piano           |
|        44 | Thumb Piano            |
|        45 | Steel Drums [ZF]       |
|        46 | Car Horn               |
|        47 | Helicopter             |
|        48 | Arctic Wind            |
|        49 | Thip                   |
|        50 | Synth Tom              |
|        51 | Squelchy Frog          |

#### Parameters

`printParams()` prints all known parameters and values to the console.

`int getNumParams()` returns the number of parameters.

`float getParamValue(int i)` returns the value of the indexed parameter.

`setParamValue(int i, float value)` sets the indexed parameter to value.

| Param Id | Name          |
| -------: | :------------ |
|        0 | OSC Mix 1     |
|        1 | OSC Tune 0.37 |
|        2 | OSC Fine 0.25 |
|        3 | Glide 0.3     |
|        4 | Gld Rate 0.32 |
|        5 | Gld Bend 0.5  |
|        6 | VCF Freq 0.9  |
|        7 | VCF Reso 0.6  |
|        8 | VCF Env 0.12  |
|        9 | VCF LFO 0     |
|       10 | VCF Vel 0.5   |
|       11 | VCF Att 0.9   |
|       12 | VCF Dec 0.89  |
|       13 | VCF Sus 0.9   |
|       14 | VCF Rel 0.73  |
|       15 | ENV Att 0     |
|       16 | ENV Dec 0.5   |
|       17 | ENV Sus 1     |
|       18 | ENV Rel 0.71  |
|       19 | LFO Rate 0.81 |
|       20 | Vibrato 0.65  |
|       21 | Noise 0       |
|       22 | Octave 0.5    |
|       23 | Tuning 0.5    |

#### Events

`handleMidiEvent(MidiMsg msg)` sends midi events to the plugin.

`noteOn(int mnote, float vel)` sends a note-on event to the plugin.

`noteOff(int mnote, float vel)` sends a note-off event to the plugin.

### Example

test.ck

```ck
DbMdaJX10 inst => dac;

<<<"Parameters", "">>>;
inst.printParams();

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
