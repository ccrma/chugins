// Use Patch and Range to smoothly modulate the width of
// the pulse oscillator.
PulseOsc tone(110) => dac;
SinOsc lfo(0.1) => Range r => Patch p(tone, "width") => blackhole;

// Help reveals all secrets
Patch.help();

// 0.1 => lfo.freq;
// Modulate pulse width
(-1, 1, 0.07, 0.93) => r.range;

eon => now;
