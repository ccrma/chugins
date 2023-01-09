// Use scaler to make an easy vibrato
SinOsc lfo => Scaler s => blackhole;
SinOsc tone => dac;

// Help reveals all secrets
Scaler.help();

3 => lfo.freq;
// Modulate frequency +- 10Hz around A440
(0, 1, 440, 10) => s.radius;

// This is equivalent to the above line, but sets the input/output range specifically
// (-1, 1, 430, 450) => s.range 

while(10::ms => now) {
	s.last() => tone.freq;
}
