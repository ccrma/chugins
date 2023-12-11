// Use Range to make an easy vibrato


// Range expects an input value from [-1,1] by default.
// This maps the values from [-1,1] to the frequecies [430,450],
// creating a vibrato of +- 10Hz.
SinOsc lfo(3) => Range r(430,450) => blackhole;

// This declaration is equivalent.
// SinOsc lfo(3) => Range r(-1, 1, 430,450) => blackhole;
SinOsc tone => dac;

// Help reveals all secrets
Range.help();

// Alternative way to set range: modulate frequency +- 10Hz around A440
// (0, 1, 440, 10) => r.radius;

// This is equivalent to the above line, but sets the input/output range specifically
// (-1, 1, 430, 450) => r.range 

// update the oscillator's frequency
while(10::ms => now) {
	r.last() => tone.freq;
}
