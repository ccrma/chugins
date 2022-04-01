## LiCK LFO effects as ChuCK chugin

DbLiCKLFO is a chugin that implements [LiCK's](https://github.com/heuermh/lick/tree/master/lick/lfo)
LFO chugen's natively.

LiCK's LFO is a linear combination of eight oscillators. In this implementation
only those oscillators with non-zero weights are actually evaluated. Convenience
methods are provided in the common case where only one oscillator is required.
The chugin also supports

* saw
* sine
* square
* triangle
* hyper
* sample and hold
* smoothed sample and hold
* filtered noise

 ### API

`freq(float f)` sets the oscillator frequency.

`setrange(float min, float max)` sets the oscillator output range.

`modulate(int onoff)` causes the optional input signal to be amplitude-modulated
(multiplied) by the LFO.

`setmix(float saw, float sine, float sqr, float tri, float hyper, float sh, float ssh, float fnoise)` sets the linear 
combination of oscillators.  Weights are auto-normalized to sum to 1.

`saw()` sets the oscillator mix to pure saw wave.

`phasor()` inverts saw to produce a phasor (with positive slope).

`sine()` sets the oscillator mix to pure sine wave.

`sqr()` sets the oscillator mix to pure square wave.

`tri()` sets the oscillator mix to pure tri wave.

`hyper()` sets the oscillator mix to pure exponential phasor.

`sampleHold()` sets the oscillator mix to pure sample and hold.

`smoothSampleHold()` sets the oscillator mix to pure smoothed sample and hold.

`fnoise()` sets the osciallator mix to pure filtered noise.

`sethold(dur h)` sets the hold duration for sample and hold.

`setsmoothhold(dur h, dur smoothhold)` sets the hold durations for smoothed 
sample and hold.

`setnoisehold(dur h)` sets filtered noise hold duration.

`float eval(float phase)` evaluates the LFO at the specified phase [0-1.].

`float phase(float phase)` sets the current phase offset for free-running mode.

`float phasewobble(float phase)` introduces a wobble to into the phase 
calculation in free-running mode.

`float phasewobblefreq(float f)` specifies the frequency for the phase wobble
measured in Hz.

 ### Examples

 Normally you'll use DbLiCKLFO in the usual assortment of envelope modification
 or parameter-automatiion applications. Here we route the LFO output 
 straight to the DAC.  In the Fiddle environment you can plot the results
 to visualize your configuration.

#### testall.ck

 ```ck
 DbLiCKLFO lfo;

 lfo => dac;
 lfo.setmix(1,1,1,1,1,1,1,1); // an equal combination of all
 50 => lfo.freq;
 .5 => lfo.gain;

 1::second => now;
 ```

 #### testwobble.ck

```ck
DbLiCKLFO lfo => dac;
lfo.sine();
lfo.freq(200);
.5 => lfo.gain;

1::second => now;

<<<"wobble">>>;
lfo.phasewobble(.0001);
lfo.phasewobblefreq(5);
1::second => now;


 ```