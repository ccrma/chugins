SinOsc lfo => Scaler s => blackhole;
SinOsc tone => dac;

3 => lfo.freq;
(0, 1, 440, 10) => s.radius;

while(10::ms => now) {
	s.last() => tone.freq;
}