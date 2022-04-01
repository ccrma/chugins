## LiCK Distortion effects as ChuCK chugin

DbLiCKDist is a chugin that implements [LiCK's](https://github.com/heuermh/lick/tree/master/lick/dist)
distortion chugen's natively.

The idea is that we'd like to distort an input signal in a variety
of subtle or less-subtle ways.  Converted to chugin form for improved
performance.

### API

`distortion(string x)` sets the distortion effect. Valid values
include:

* WaveShaper  `x/(1.f + fabsf(x))`
* Atan  `atan(x)/2PI`
* BucketBrigade  (purpose unclear, produces two shreds)
* Chew(lfo0Step, lfo0Depth, lfo1Step, lfo2Depth) has two triangular LFOs running 
   by default at 4400 hz and depth of .1. These are used to define a top and 
   bottom envelope against which to clip the signal in the clip region we 
   replace the signal with the triwave.
* Clip(range) clips the incoming signal to +/- range
* Duff(shape) `x/((1.1f - shape) + fabs(x))`
* Frostburn `(x * fabsf(x) + x)/(x*x + fabsf(x) + 1.f)`
* FullRectifier(threshold, bias) values below threshold are inverted and biased.
  Classic full-rectification has threshold of 0 and bias of 0.
* Offset(offset)  `x + offset`
* Phase(inOrOut) when inOrOut is 1: the signal is 'inverted' (phase-shifted by 180)
* Invert inverts the signal
* KijjazDist `x/(1.f + x*x)`.
* KijjazDist2 `x^3/(1.0 + abs(x^3))`
* KijjazDist3 `x(1.0 + x^2)/(1.0 + abs(x(1.0 + x^2)))`
* KijjazDist4 `x(1.0 + x^4)/(1.0 + abs(x(1.0 + x^4)))`
* Ribbon `x/(0.25 * x^2 + 1.0)`
* Tanh hyperbolic tangent

`setparam(int index, float value)` sets the indexth parameter of the
 current distortion. Only those effects that accept arguments specified
 above attend to this request.

 ### Example

```ck
DbLiCKDistort distort;
SndBuf o => distort => dac;
o.read("../../PitchTrack/data/obama.wav");
o.loop(1);

<<< "Defaults are rather subtle.",
"Effects are best experimented with within Fiddle and a guitar.">>>;

["WaveShaper", "Atan", "Chew", "Clip", "Duff", 
 "Frostburn", "FullRectifier", "Offset", "Phase", "Invert",
 "KijjazDist", "KijjazDist2", "KijjazDist3", "KijjazDist4",
 "Ribbon", "Tanh"
 ] @=> string effects[];

for(int i; i<effects.size();i++)
{
    <<<i, effects[i]>>>;
    distort.distortion(effects[i]);
    2::second => now;
}
```