TriOsc foo => dac;
adc => PitchTrack hh => blackhole;
0.5 => foo.gain;

while (true)
{
	samp => now;
	hh.get() => foo.freq;
}
