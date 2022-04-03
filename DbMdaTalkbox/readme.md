## DbMdaTalkbox chugin

The DbMdaTalkbox chugin is a port of Paul Keller's open-source
vst plugin here: http://mda.smartelectronix.com/ and
here: https://sourceforge.net/projects/mda-vst. It is
GPL2 and MIT licensed.

mda-Talkbox implements an LPC (linear-predictive-coding) vocoder to
produce wonderfully robotic sounds.  

Vocoders combine two inputs, the carrier and the modulator, to produce
an output signal.  Typically, the carrier is a musical instrument output
and the modulator is a speech-like-signal.  In this implementation,
the DbMdaTalkbox access a single stereo input signal with the modulator
in the left channel and the carrier in the right.  ChucK's `Pan2` ugen
can be used to produce a stereo channel from two mono inputs.

### API

#### Presets

`printPresets(int)` prints all known presets to the console.

`int getNumPresets()` returns the number of presets.
 
`selectPreset(int i)` selects the indexed preset.

#### Parameters

`printParams()` prints all known parameters and values to the console.

`int getNumParams()` returns the number of parameters.

`float getParamValue(int i)` returns the value of the indexed parameter.

`setParamValue(int i, float value)` sets the indexed parameter to value.

### Example

test.ck

```ck
Pan2 pan => DbMdaTalkbox talk => dac;
talk.printParams();

SqrOsc sqr;
sqr => pan.right; // carrier
sqr.gain(.5);

SndBuf buf;
buf => pan.left;  // modulator
buf.gain(.5);
buf.loop(1);
buf.read("../../PitchTrack/data/obama.wav");
buf.pos(0);

0.1 => float dry;
talk.setParam(1, dry);
<<<"dry", dry>>>;
for(1=>int j;j<=10;j++)
{
    j/10. => float wet; 
    talk.setParam(0, wet);
    <<<"wet", wet>>>;
    for(int i;i<10;i++)
    {
        Math.random2f(50, 200) => sqr.freq;
        .60::second => now;
    }
}
```
