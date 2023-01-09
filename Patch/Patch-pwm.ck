// Use scaler to make an easy vibrato
SinOsc lfo => Range r => Patch p => blackhole;
PulseOsc tone => dac;

// Help reveals all secrets
Range.help();

3 => lfo.freq;
// Modulate frequency +- 10Hz around A440
(-1, 1, 0.1, 0.9) => r.range;

p.connect(tone, "width");

1::week => now;