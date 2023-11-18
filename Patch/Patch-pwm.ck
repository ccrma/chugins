// Use Patch and Range to smoothly modulate the width of
// the pulse oscillator.


// Let's make our pulse oscillator!
PulseOsc tone(110) => dac;

// Module the pulse width
TriOsc lfo(0.1) => Range r(0.07, 0.93) => Patch p(tone, "width") => blackhole;

// Help reveals all secrets
Patch.help();

eon => now;
