DbLiCKLFO lfo => dac;
lfo.sine();
lfo.freq(200);
.5 => lfo.gain;

1::second => now;

<<<"wobble">>>;
lfo.phasewobble(.0001);
lfo.phasewobblefreq(5);
1::second => now;

