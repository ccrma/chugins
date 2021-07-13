# DbGraphBuf

## Intro
DbGrainBuf is a sndbuf-based granular synthesis chugin for Chuck.
A rewrite of chuck's SndBuf UGen that combines functionality of
SuperCollider's GrainBuf ugen as motivated by Eli Fieldsteel's 
highly recommended [youtube tutorial](https://www.youtube.com/watch?v=WBqAM_94TW4).


## Use Cases

### Bypass

Useful to verify behavior/timing of your sound file.

```chuck
DbGrainBuf db => NRev rev => dac;
1 => db.bypass;
1.1 => db.rate;
"../../PitchTrack/data/obama.wav" => db.read;
.5 => db.phase; // start 1/4 through the file
3::second => now;
```

### Granular Synthesis

1. specify trigger frequency + range - this is how often new grains are created.
   Higher frequence means more grains.  Usually higher frequencies require/imply
   smaller grain-durations. A non-zero range causes triggers to occur at
   random time within the range (measured a pct of the trigger period).
   To specify an external trigger, just connect your trigger UGEN to the 
   input.
2. pick a grain duration, variance and rate. 
   Duration specified in time units, variance a pct of the grain duration.
   Rate controls grain "pitch" and direction. midiratio can be used to
   pitch-shift in harmonic intervals.
3. pick a grain position/phase (sampled when a new grain is triggered)
    * start, stop, rate, wobble
        * start==stop: constant position
        * start=0, stop=1, rate=1, play-through via grains
        * start=0, stop=1, rate=midiratio(1), at a slightly faster rate

## Reference

```supercollider
# from Eli Fieldsteel's Granular Synthesis Tutorial
SynthDef.new(\gs, 
{
    arg sync=1, dens=20, dur=0.1, durRand=1, buf=0,
    rate=1, pos=0, posSpeed=1, posRand=0,
    grainEnv=(-1), pan=0, panHz=0.1, panRand=0,
    atk=1, sus=2, rel=1, c0=1, c1=(-1), amp=1, out=0;

    var sig, env, densCtrl, durCtrl, posCtrl, panCtrl;
    env = EnvGen(ar(Env.new([0,1,1,0], [atk,sus,rel],[c0,0,c1]), doneAction:2));
    densCtrl = Select.ar(sync, [Dust.ar(dens), Impulse.ar(dens)]);
    durCtrl = dur * LFNoise1.kr(100).exprange(1/durRand, durRand);
    posCtrl = Phasor.ar(0, posSpeed*BufRateScale.ir(buf), 0, BufSamples.ir(buf) - 1);
    posCtrl = posCtrl + LFNoise1.kr(100).bipolar(posRand*SampleRate.ir);
    posCtrl = posCtrl / BufSamples.ir(buf);
    posCtrl = posCtrl  + pos;

    panCtrl = pan + LFNoise1.kr(panHz).bipoloar(panRand);
    sig = GrainBuf.ar(
        2, // numchan
        densCtrl, // trigger
        durCtrl,  // dur
        buf,      // sndbuf
        rate,     // rate
        posCtrl,  // pos (0-1)
        2,        // interp (2 is linear)
        panCtrl,  // pan
        grainEnv  // envbuf (windowing function)
    );
    sig = sig * env * amp;
    Out.ar(out, sig);
}).add;

```

