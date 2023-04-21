SinOsc sin => Pan2 pan => dac;

// Our LFO is fed into Patch's input...
SinOsc lfo => Patch p => blackhole;
1 => lfo.freq;

// ... where it then updates the pan position at every tick
p.connect(pan, "pan");

1::hour => now;
