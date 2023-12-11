SinOsc sin => Pan2 pan => dac;

// Our LFO is fed into Patch's input...
SinOsc lfo(1) => Patch p(pan, "pan") => blackhole;

eon => now;
