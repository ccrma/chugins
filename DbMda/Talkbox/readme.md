## DbMdaTalkbox chugin

The DbMdaTalkbox chugin is a port of Paul Kellett's open-source
vst plugin here: http://mda.smartelectronix.com/ and
here: https://sourceforge.net/projects/mda-vst. It is
GPL2 and MIT licensed.

mda-Talkbox implements an LPC (linear-predictive-coding) vocoder to
produce wonderfully robotic sounds.  Vocoders combine two inputs, the 
carrier and the modulator, to produce an output signal.  

> Typically the carrier is a speech-like signal and the modulator
> is a musical tone/instrument.  
 
In this implementation, the DbMdaTalkbox accepts a single stereo input 
signal with the carrier in the left channel and the modulator in the right
Since it is a UGen_Stereo subclass, you can connect independent mono signals 
to its left and right inlets, as demonstrated in the [example below](#Example).

### API

#### Parameters

`printParams()` prints all known parameters and values to the console.

`float getParam(int i)` returns the value of the indexed parameter.

`setParam(int i, float value)` sets the indexed parameter to value.

| Parameter | Name    | Default | Description                                            |
| --------: | :------ | :------ | :----------------------------------------------------- |
|         0 | Wet     | .5      | the amount of the processed signal in the output [0-1] |
|         1 | Dry     | 0.      | the amount of the dry signal in the output [0-1]       |
|         2 | Quality | 1.      | the quality of the result.  [0-1]                      |

### Example

test.ck

```ck
DbMdaTalkbox talk => dac;
talk.printParams();

SndBuf buf;
buf => talk.left;  // carrier
buf.gain(.5);
buf.loop(1);
buf.read("../../../PitchTrack/data/obama.wav");
buf.pos(0);

SqrOsc sqr;
sqr => talk.right; // modulator
sqr.gain(.5);

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
