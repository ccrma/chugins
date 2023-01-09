// Use Range to make an easy vibrato
SinOsc lfo => Range r => blackhole;
SinOsc tone => dac;

// Help reveals all secrets
Range.help();

3 => lfo.freq;
// Modulate frequency +- 10Hz around A440
(0, 1, 440, 10) => r.radius;

// This is equivalent to the above line, but sets the input/output range specifically
// (-1, 1, 430, 450) => r.range 

while(10::ms => now) {
	r.last() => tone.freq;
}
