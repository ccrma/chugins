// Test that the bpm works even when 
// no ".asd" file exists.
// You should hear 2 beats, then a tempo change, then repeat.
WarpBuf s1 => dac;

me.dir() + "assets/381353__waveplaysfx__drumloop-120-bpm-edm-drum-loop-022_no_asd.wav" => s1.read;

while(true) {
	60. + Std.rand2(0, 3)*30 => s1.bpm;
	<<<"bpm: " + s1.bpm()>>>;
	(120./s1.bpm())::second => now;
}