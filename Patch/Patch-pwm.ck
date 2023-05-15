// Use Patch and Range to smoothly modulate the width of
// the pulse oscillator.
SinOsc lfo => Range r => Patch p => blackhole;
PulseOsc tone => dac;

// Help reveals all secrets
Patch.help();

0.1 => lfo.freq;
// Modulate pulse width
(-1, 1, 0.07, 0.93) => r.range;

// Connect to the "width" member function of tone.
p.connect(tone, "width");

1::week => now;
